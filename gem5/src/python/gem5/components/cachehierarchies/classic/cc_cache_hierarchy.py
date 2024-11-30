
# Copyright (c) 2022 The Regents of the Yonsei University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from m5.objects import (
    BadAddr,
    BaseXBar,
    Cache,
    CC_BankedCache,
    L2XBar,
    Port,
    SystemXBar,
)

from ....isas import ISA
from ....utils.override import *
from ...boards.abstract_board import AbstractBoard
from ..abstract_cache_hierarchy import AbstractCacheHierarchy
from ..abstract_three_level_cache_hierarchy import AbstractThreeLevelCacheHierarchy
from .abstract_classic_cache_hierarchy import AbstractClassicCacheHierarchy
from .caches.l1dcache import L1DCache
from .caches.l1icache import L1ICache
from .caches.l2cache import L2Cache
from .caches.mmu_cache import MMUCache

from .caches.cc_banked_cache import CC_CacheInstance


class CheckerCacheHierarchy(
    AbstractClassicCacheHierarchy, AbstractThreeLevelCacheHierarchy
):
    """
    A cache setup where each core has a private L1 Data and Instruction Cache,
    and a L2 cache is shared with all cores. The shared L2 cache is mostly
    inclusive with respect to the split I/D L1 and MMU caches.
    """

    @staticmethod
    def _get_default_membus() -> SystemXBar:
        #getting the memside of the l2 cache, used as a default when not specified
        """
        A method used to obtain the default memory bus of 64 bit in width for
        the PrivateL1SharedL2 CacheHierarchy.

        :returns: The default memory bus for the PrivateL1SharedL2
                  CacheHierarchy.

        :rtype: SystemXBar
        """
        membus = SystemXBar(width=64)
        membus.badaddr_responder = BadAddr()
        membus.default = membus.badaddr_responder.pio
        return membus

    def __init__(
        self,
        l1d_size: str,
        l1i_size: str,
        l2_size: str,
        l3_size: str, # new

        l1d_assoc: int = 8,
        l1i_assoc: int = 8,
        l2_assoc: int = 32,
        l3_assoc: int = 32, # new
        membus: BaseXBar = _get_default_membus.__func__(),
    ) -> None:
        """
        :param l1d_size: The size of the L1 Data Cache (e.g., "32kB").
        :param  l1i_size: The size of the L1 Instruction Cache (e.g., "32kB").
        :param l2_size: The size of the L2 Cache (e.g., "256kB").
        :param l1d_assoc: The associativity of the L1 Data Cache.
        :param l1i_assoc: The associativity of the L1 Instruction Cache.
        :param l2_assoc: The associativity of the L2 Cache.
        :param membus: The memory bus. This parameter is optional parameter and
                       will default to a 64 bit width SystemXBar is not specified.
        """

        AbstractClassicCacheHierarchy.__init__(self=self)
        AbstractThreeLevelCacheHierarchy.__init__(
            self,
            l1i_size=l1i_size,
            l1i_assoc=l1i_assoc,
            l1d_size=l1d_size,
            l1d_assoc=l1d_assoc,
            l2_size=l2_size,
            l2_assoc=l2_assoc,
            l3_size=l3_size, # new
            l3_assoc=l3_assoc # new
        )

        self.membus = membus
        # self.membus.snoop_filter = SnoopFilter(entries=262144)

    #getting the overall cache hierarchy's memside and cpu side ports
    @overrides(AbstractClassicCacheHierarchy)
    def get_mem_side_port(self) -> Port:
        return self.membus.mem_side_ports

    @overrides(AbstractClassicCacheHierarchy)
    def get_cpu_side_port(self) -> Port:
        return self.membus.cpu_side_ports


    @overrides(AbstractCacheHierarchy)
    def incorporate_cache(self, board: AbstractBoard) -> None:
        # Set up the system port for functional access from the simulator.
        
        #funky gem5 thing allowing functional operations without disrupting the cache hierarchy
        board.connect_system_port(self.membus.cpu_side_ports)

        #memside of cache to memory (ie DDR3 etc)
        for _, port in board.get_memory().get_mem_ports():
            self.membus.mem_side_ports = port

        # setting up per core L1 and L2 caches.
        # per core lists of caches
        self.l1icaches = [
            L1ICache(
                size=self._l1i_size,
                assoc=self._l1i_assoc,
                writeback_clean=False,
            )
            for i in range(board.get_processor().get_num_cores())
        ]
        self.l1dcaches = [
            L1DCache(size=self._l1d_size, assoc=self._l1d_assoc)
            for i in range(board.get_processor().get_num_cores())
        ]

        # designated L1-L2 cache bus
        #look at src/mem/XBar.py to see different types of buses and how they are used.
        #may need to make another for L2-L3 etc latencies important for checker cache travel time.
        self.l2bus = L2XBar()
        self.l3bus = L2XBar()

        # defining shared l2
        self.l2cache = L2Cache(size=self._l2_size, assoc=self._l2_assoc)

        self.cc_l3cache = CC_CacheInstance(size=self._l3_size)
        # Ensure L2 cache has the correct address range from the board
        # self.cc_l2cache.addr_ranges = board.mem_ranges

        # Create a new bus for cc_buffer connections
        # self.cc_buffer_bus = SystemXBar()
        # self.cc_buffer_bus.default = self.cc_l2cache.cpu_side
        # self.cc_buffer_bus.addr_ranges = self.cc_l2cache.addr_ranges


        # ITLB Page walk caches
        self.iptw_caches = [
            MMUCache(size="8KiB", writeback_clean=False)
            for _ in range(board.get_processor().get_num_cores())
        ]
        # DTLB Page walk caches
        self.dptw_caches = [
            MMUCache(size="8KiB", writeback_clean=False)
            for _ in range(board.get_processor().get_num_cores())
        ]

        # to deal with coherence of io objects sharing memory
        # I dont think this is relevant per se.
        if board.has_coherent_io():
            self._setup_io_cache(board)


        for i, cpu in enumerate(board.get_processor().get_cores()):
            #connect every $i and $d to the cpu
            cpu.connect_icache(self.l1icaches[i].cpu_side)
            cpu.connect_dcache(self.l1dcaches[i].cpu_side)

            #connect every $i, $d, $iTLB, $dTLB to L1-L2 bus
            # tlbs share l2 with i caches and d caches.
            self.l1icaches[i].mem_side = self.l2bus.cpu_side_ports
            self.l1dcaches[i].mem_side = self.l2bus.cpu_side_ports
            self.iptw_caches[i].mem_side = self.l2bus.cpu_side_ports
            self.dptw_caches[i].mem_side = self.l2bus.cpu_side_ports

            # connect tlb ports
            cpu.connect_walker_ports(
                self.iptw_caches[i].cpu_side, self.dptw_caches[i].cpu_side
            )

            #x86 specific interrupts
            if board.get_processor().get_isa() == ISA.X86:
                int_req_port = self.membus.mem_side_ports
                int_resp_port = self.membus.cpu_side_ports
                cpu.connect_interrupt(int_req_port, int_resp_port)
            else:
                cpu.connect_interrupt()

            # checker chip specifics:
            # Connect each cc_buffer's memory-side port to the cc_buffer_bus
            # cpu.core.cc_buffer.cc_mem_side_port = self.cc_buffer_bus.cpu_side_ports
            # if (i==0):
            cpu.core.cc_buffer.cc_mem_side_port = self.cc_l3cache.cc_cpu_port
            # cpu - CC_proc, cpu.core - cc_core, cpu.core.core - base03cpu
            # cpu.core.core.cc_buffer.cc_mem_side_port = self.cc_l2cache.cc_cpu_port

        # # Connect the cc_buffer_bus memory-side port to cc_l2cache's CPU port
        # self.cc_buffer_bus.mem_side_ports = self.cc_l2cache.cc_cpu_port

        #L2Bus-L2
        # self.l2bus.mem_side_ports = self.cc_l2cache.cpu_side
        self.l2bus.mem_side_ports = self.l2cache.cpu_side

        #L2-L3Bus
        self.l3bus.cpu_side_ports = self.l2cache.mem_side

        #L3Bus-L3
        self.l3bus.mem_side_ports = self.cc_l3cache.cpu_side

        #L3-MemBus
        # self.membus.cpu_side_ports = self.l2cache.mem_side
        self.membus.cpu_side_ports = self.cc_l3cache.mem_side

        # about to do something completely crazy:

    def _setup_io_cache(self, board: AbstractBoard) -> None:
        """Create a cache for coherent I/O connections"""
        self.iocache = Cache(
            assoc=8,
            tag_latency=50,
            data_latency=50,
            response_latency=50,
            mshrs=20,
            size="1kB",
            tgts_per_mshr=12,
            addr_ranges=board.mem_ranges,
        )
        self.iocache.mem_side = self.membus.cpu_side_ports
        self.iocache.cpu_side = board.get_mem_side_coherent_io_port()
