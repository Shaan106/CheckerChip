from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.cc_cache_hierarchy import CheckerCacheHierarchy
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.cc_processor import CC_Processor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA

# No cache hierarchy
cache_hierarchy = CheckerCacheHierarchy(
    l1d_size="32kB", 
    l1i_size="32kB", 
    l2_size="16MB",
    l3_size="2MB",
)

# Set up the memory type and size
memory = SingleChannelDDR3_1600("16GiB")

# Processor setup
processor = CC_Processor(cpu_type=CPUTypes.O3, num_cores=8, isa=ISA.X86)

# Setup the board with the components
board = SimpleBoard(clk_freq="3GHz", 
                    processor=processor, 
                    memory=memory,
                    cache_hierarchy=cache_hierarchy)

# Set the path to the lbm binary (use the path where lbm is located)
binary = CustomResource("/home/ay140/gem5_disk/SPECCPU2006.clean/benchspec/CPU2006/470.lbm/run/build_base_x86_64.0000/lbm")

# Define the arguments to be passed to lbm (these match the format from lbm.in)
args = ["64", "reference.dat", "0", "0", 
        "/home/ay140/gem5_disk/SPECCPU2006.clean/benchspec/CPU2006/470.lbm/run/run_base_ref_x86_64.0000/100_100_130_ldc.of"]

# Set the workload for SE (System Emulation) mode
board.set_se_binary_workload(binary, arguments=args)

# Initialize the simulator with the configured board
simulator = Simulator(board=board)

# simulator.simulation.set_max_insts(1000)

# Run the simulation
simulator.run()
