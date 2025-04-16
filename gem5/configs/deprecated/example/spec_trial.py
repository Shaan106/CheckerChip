import argparse
# import os
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
# from gem5.components.boards.se_binary_workload import SEBinaryWorkload

# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("--cmd", type=str, required=True)
parser.add_argument("--options", type=str, default="")
parser.add_argument("--mem-size", type=str, default="64GiB")
parser.add_argument("--maxinsts", type=int, default=None)  # New argument
args = parser.parse_args()

# System configuration
cache_hierarchy = CheckerCacheHierarchy(
    l1d_size="32kB",
    l1i_size="32kB",
    l2_size="16MB",
    l3_size="16MB"
)

memory = SingleChannelDDR3_1600(args.mem_size)
processor = CC_Processor(cpu_type=CPUTypes.O3, num_cores=8, isa=ISA.X86)
board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# Workload setup
binary = CustomResource(args.cmd)
# input_dir = os.path.dirname(args.cmd)
command = args.options.split() if args.options else []

board.set_se_binary_workload(
    binary=binary,
    arguments=command
    # directory=input_dir,
    # os_workload=SEBinaryWorkload.X86_LINUX
)

# Simulation control with max instructions
simulator = Simulator(board=board)

if args.maxinsts:
    simulator.schedule_max_insts(args.maxinsts)  # Schedule instruction limit
simulator.run()