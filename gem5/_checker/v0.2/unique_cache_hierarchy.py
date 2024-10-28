# https://www.gem5.org/documentation/gem5-stdlib/develop-own-components-tutorial

from gem5.components.cachehierarchies.classic.abstract_classic_cache_hierarchy import AbstractClassicCacheHierarchy
from gem5.components.cachehierarchies.classic.caches.l1dcache import L1DCache
from gem5.components.cachehierarchies.classic.caches.l1icache import L1ICache
from gem5.components.cachehierarchies.classic.caches.mmu_cache import MMUCache
from gem5.components.boards.abstract_board import AbstractBoard

from m5.objects import Port, SystemXBar, BadAddr, Cache
from gem5.components.cachehierarchies.classic.caches.l2cache import L2Cache
from m5.objects import L2XBar

class UniqueCacheHierarchy(AbstractClassicCacheHierarchy):
    def __init__(self) -> None:
        super().__init__()
        self.membus = SystemXBar(width=64)
        self.membus.badaddr_responder = BadAddr()
        self.membus.default = self.membus.badaddr_responder.pio
        self.l2bus = L2XBar()
        self.l2cache = L2Cache(size="256KiB", assoc=8)

    def incorporate_cache(self, board: AbstractBoard) -> None:
        board.connect_system_port(self.membus.cpu_side_ports)

        for cntr in board.get_memory().get_memory_controllers():
            cntr.port = self.membus.mem_side_ports

        self.l1icaches = [L1ICache(size="32KiB") for _ in range(board.get_processor().get_num_cores())]
        self.l1dcaches = [L1DCache(size="32KiB") for _ in range(board.get_processor().get_num_cores())]
        self.iptw_caches = [MMUCache(size="8KiB") for _ in range(board.get_processor().get_num_cores())]
        self.dptw_caches = [MMUCache(size="8KiB") for _ in range(board.get_processor().get_num_cores())]

        if board.has_coherent_io():
            self._setup_io_cache(board)

        for i, cpu in enumerate(board.get_processor().get_cores()):
            cpu.connect_icache(self.l1icaches[i].cpu_side)
            cpu.connect_dcache(self.l1dcaches[i].cpu_side)
            self.l1icaches[i].mem_side = self.l2bus.cpu_side_ports
            self.l1dcaches[i].mem_side = self.l2bus.cpu_side_ports
            self.iptw_caches[i].mem_side = self.l2bus.cpu_side_ports
            self.dptw_caches[i].mem_side = self.l2bus.cpu_side_ports
            cpu.connect_walker_ports(self.iptw_caches[i].cpu_side, self.dptw_caches[i].cpu_side)
            cpu.connect_interrupt(self.membus.mem_side_ports, self.membus.cpu_side_ports)

        self.l2bus.mem_side_ports = self.l2cache.cpu_side
        self.l2cache.mem_side = self.membus.cpu_side_ports
