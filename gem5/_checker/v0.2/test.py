from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import MESITwoLevelCacheHierarchy

from unique_cache_hierarchy import UniqueCacheHierarchy

# Define your Ruby cache hierarchy with desired properties
num_cores = 8
cache_hierarchy = UniqueCacheHierarchy()

# Define RAM and Processor
memory = SingleChannelDDR3_1600("8GiB")
processor = SimpleProcessor(cpu_type=CPUTypes.O3, num_cores=num_cores, isa=ISA.X86)

# Setup the board with all components, using the Ruby cache hierarchy
board = SimpleBoard(clk_freq="3GHz", 
                    processor=processor, 
                    memory=memory,
                    cache_hierarchy=cache_hierarchy)

# Load the binary
binary = CustomResource("/home/ay140/CheckerChip/gem5/_checker/v0.1/SAXPY/saxpy_static")

# Setting the workload in SE mode
board.set_se_binary_workload(binary)

# Running the simulation
simulator = Simulator(board=board)
simulator.run()
