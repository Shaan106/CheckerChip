from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import Resource
from gem5.simulate.simulator import Simulator

from gem5.isas import ISA

#importing stuff for caches
from gem5.components.cachehierarchies.classic.private_l1_private_l2_cache_hierarchy import PrivateL1PrivateL2CacheHierarchy

#essentially you get a "virtual motherboard"
# and add all your "virtual" components to said board

# new cache hierarchy
cache_hierarchy = PrivateL1PrivateL2CacheHierarchy(
      l1d_size="32KiB", # l1 data cache size
      l1i_size="32KiB", # inst cache size
      l2_size="64KiB", #l2 cache size
)
# for no cache:
# cache_hierarchy = NoCache()


# type of RAM, what size
memory = SingleChannelDDR3_1600("1GiB")

#processor
#single core processor, 1 timing CPU processor
#timing is what type of gem5 simulation (also atomic etc)
processor = SimpleProcessor(cpu_type=CPUTypes.TIMING, num_cores=1, isa=ISA.X86)
# setting ISA not in tutorial, checked actual files to fix.

# setup the board
board = SimpleBoard(clk_freq="3GHz", 
                    processor=processor, 
                    memory=memory,
                    cache_hierarchy=cache_hierarchy)


# loading the binary for running hello-world
binary = Resource("x86-hello64-static")
#setting workload to current board (we are going to run in SE mode)
board.set_se_binary_workload(binary)

# want to run a simulation with the board we have specified (the one just created)
simulator = Simulator(board=board)
simulator.run()

# ------------- output 1 --------------
'''
ay140@clipper01:~/CheckerChip/gem5$ ./build/X86/gem5.opt ./playground/hello-world-caches.py 
gem5 Simulator System.  https://www.gem5.org
gem5 is copyrighted software; use the --copyright option for details.

gem5 version 23.1.0.0
gem5 compiled Mar 21 2024 19:58:08
gem5 started Jun 16 2024 15:17:30
gem5 executing on clipper01, pid 2523741
command line: ./build/X86/gem5.opt ./playground/hello-world-caches.py

warn: `Resource` has been deprecated. Please use the `obtain_resource` function instead.
info: Using default config
Global frequency set at 1000000000000 ticks per second
warn: No dot file generated. Please install pydot to generate the dot file and pdf.
src/mem/dram_interface.cc:690: warn: DRAM device capacity (8192 Mbytes) does not match the address range assigned (1024 Mbytes)
src/base/statistics.hh:279: warn: One of the stats is a legacy stat. Legacy stat is a stat that does not belong to any statistics::Group. Legacy stat is deprecated.
board.remote_gdb: Listening for connections on port 7000
src/sim/simulate.cc:199: info: Entering event queue @ 0.  Starting simulation...
src/sim/syscall_emul.hh:1014: warn: readlink() called on '/proc/self/exe' may yield unexpected results in various settings.
      Returning '/home/ay140/.cache/gem5/x86-hello64-static'
src/sim/mem_state.cc:448: info: Increasing stack size by one page.
Hello world!
'''

# --------- OTHER INFO -----------
# go to m5out/stats.txt for more execution info .

'''
---------- Begin Simulation Statistics ----------
simSeconds                                   0.000031                       # Number of seconds simulated (Second)
simTicks                                     30765204                       # Number of ticks simulated (Tick)
finalTick                                    30765204                       # Number of ticks from beginning of simulation (restored from checkpoints and never reset) (Tick)
simFreq                                  1000000000000                       # The number of ticks per simulated second ((Tick/Second))
hostSeconds                                      0.06                       # Real time elapsed on the host (Second)
hostTickRate                                557623505                       # The number of ticks simulated per host second (ticks/s) ((Tick/Second))
hostMemory                                    1188208                       # Number of bytes of host memory used (Byte)
simInsts                                         6555                       # Number of instructions simulated (Count)
simOps                                          12992                       # Number of ops (including micro ops) simulated (Count)
hostInstRate                                   118311                       # Simulator instruction rate (inst/s) ((Count/Second))
hostOpRate                                     234381                       # Simulator op (including micro ops) rate (op/s) ((Count/Second))
board.cache_hierarchy.dptw_caches.blockedCycles::no_mshrs            0                       # number of cycles access was blocked (Cycle)
board.cache_hierarchy.dptw_caches.blockedCycles::no_targets            0                       # number of cycles access was blocked (Cycle)
board.cache_hierarchy.dptw_caches.blockedCauses::no_mshrs            0                       # number of times access was blocked (Count)

.. hundreds of  lines of just stats of diff things
'''