# ruby_config.py

from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA
from gem5.utils.requires import requires

# Import the MESITwoLevelCacheHierarchy class for Ruby
from gem5.components.cachehierarchies.ruby.mesi_two_level_cache_hierarchy import MESITwoLevelCacheHierarchy

# Ensure the required ISA is available
requires(isa_required=ISA.X86)

# Define the number of cores
num_cores = 8

# Define your Ruby cache hierarchy with desired properties
cache_hierarchy = MESITwoLevelCacheHierarchy(
    l1i_size="32KiB",
    l1i_assoc=2,  # Adjust associativity as needed
    l1d_size="32KiB",
    l1d_assoc=2,  # Adjust associativity as needed
    l2_size="256KiB",
    l2_assoc=8,   # Adjust associativity as needed
    num_l2_banks=4  # Set the number of L2 banks; increase if you want a banked L2 cache
)

# Define RAM and Processor
memory = SingleChannelDDR3_1600(size="8GiB")
processor = SimpleProcessor(cpu_type=CPUTypes.O3, num_cores=num_cores, isa=ISA.X86)

# Setup the board with all components, using the Ruby cache hierarchy
board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy
)

# Load the binary
binary = CustomResource("/home/ay140/CheckerChip/gem5/_checker/v0.1/SAXPY/saxpy_static")

# Setting the workload in SE mode
board.set_se_binary_workload(binary)

# Running the simulation
simulator = Simulator(board=board)
simulator.run()
