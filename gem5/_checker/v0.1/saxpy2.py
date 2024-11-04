from gem5.components.boards.simple_board import SimpleBoard


from gem5.components.cachehierarchies.classic.no_cache import NoCache

# from gem5.components.cachehierarchies.classic.private_l1_private_l2_cache_hierarchy import PrivateL1PrivateL2CacheHierarchy
from gem5.components.cachehierarchies.classic.cc_cache_hierarchy import CheckerCacheHierarchy
# from gem5.components.cachehierarchies.classic.private_l1_shared_l2_cache_hierarchy import PrivateL1SharedL2CacheHierarchy

from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import Resource
from gem5.resources.resource import obtain_resource
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator

from gem5.isas import ISA

# no cache
# cache_hierarchy = PrivateL1SharedL2CacheHierarchy(
cache_hierarchy = CheckerCacheHierarchy(
    l1d_size="32kB", 
    l1i_size="32kB", 
    l2_size="256kB"
)

# type of RAM, what size
memory = SingleChannelDDR3_1600("8GiB")

processor = SimpleProcessor(cpu_type=CPUTypes.O3, num_cores=8, isa=ISA.X86)

# setup the board with all the components
board = SimpleBoard(clk_freq="3GHz", 
                    processor=processor, 
                    memory=memory,
                    cache_hierarchy=cache_hierarchy)

# loading the binary for running hello-world

# binary = CustomResource("/home/ay140/CheckerChip/gem5/_checker/v0.1/SAXPY/saxpy_static")
binary = Resource("x86-hello64-static")

#setting workload to current board (we are going to run in SE mode)
board.set_se_binary_workload(binary)

# want to run a simulation with the board we have specified (the one just created)
simulator = Simulator(board=board)
simulator.run()