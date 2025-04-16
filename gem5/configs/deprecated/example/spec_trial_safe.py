import argparse
from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.cc_cache_hierarchy import (
    CheckerCacheHierarchy,
)

from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.cc_processor import CC_Processor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA

# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("--cmd", type=str, required=True)
parser.add_argument("--options", type=str, default="")
parser.add_argument("--mem-size", type=str, default="8GiB")
args = parser.parse_args()

# System configuration
cache_hierarchy = CheckerCacheHierarchy(
    l1d_size="32kB",
    l1i_size="32kB", 
    l2_size="16MB",
    l3_size="16MB",
)

memory = SingleChannelDDR3_1600(args.mem_size)
processor = CC_Processor(cpu_type=CPUTypes.O3, num_cores=8, isa=ISA.X86)
board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# # Workload setup
# process = CustomResource(args.cmd)
# command = [args.cmd.split("/")[-1]] + args.options.split()
# board.set_se_binary_workload(
#     binary=process,
#     arguments=command
# )

# Workload setup
binary = CustomResource(args.cmd)
# input_dir = os.path.dirname(args.cmd)  # Get binary directory

# Split arguments WITHOUT including executable name
command = args.options.split() if args.options else []

board.set_se_binary_workload(
    binary=binary,
    arguments=command
    # directory=input_dir,  # Set working directory
    # os_workload=SEBinaryWorkload.X86_LINUX
)


# Simulation control
simulator = Simulator(board=board)
simulator.run()


'''
/home/ay140/spring25/CheckerChip/gem5/build/X86/gem5.opt /home/ay140/spring25/Checkerip/gem5/configs/deprecated/example/spec_trial.py --cmd=../../build/build_base_mytest-m64.0000/lbm_s  --options="2000 reference.dat 0 0 200_200_260_ldc.of" --mem-size=8GB
'''