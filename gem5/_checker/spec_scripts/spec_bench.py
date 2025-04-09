'''
/home/ay140/spring25/CheckerChip/gem5/build/X86/gem5.opt 
/home/ay140/spring25/CheckerChip/gem5/_checker/spec_scripts/spec_bench.py \
  --cmd=/home/ay140/spec-2017/cpu2017/benchspec/CPU/605.mcf_s/build/build_base_CLIPPER_TEST-m64.0000/mcf_s \
  --options="inp.in" \
  --input=/home/ay140/spec-2017/cpu2017/benchspec/CPU/605.mcf_s/run/run_base_refspeed_CLIPPER_TEST-m64.0000/inp.in \
  --output=/home/ay140/spring25/CheckerChip/gem5/_checker/spec_scripts/outputs/out.txt \
  --errout=/home/ay140/spring25/CheckerChip/gem5/_checker/spec_scripts/outputs/err.txt \
  --maxinsts=10000 \
  --num-cpus=8
'''

'''
/home/ay140/spring25/CheckerChip/gem5/build/X86/gem5.opt /home/ay140/spring25/CheckerChip/gem5/_checker/spec_scripts/spec_bench.py --cmd=/home/ay140/spec-2017/cpu2017/benchspec/CPU/605.mcf_s/build/build_base_CLIPPER_TEST-m64.0000/mcf_s --options="inp.in" --input=/home/ay140/spec-2017/cpu2017/benchspec/CPU/605.mcf_s/run/run_base_refspeed_CLIPPER_TEST-m64.0000/inp.in --output=/home/ay140/spring25/CheckerChip/gem5/_checker/spec_scripts/outputs/out.txt --errout=/home/ay140/spring25/CheckerChip/gem5/_checker/spec_scripts/outputs/err.txt --maxinsts=10000 --num-cpus=8
'''

#!/usr/bin/env python3
# Copyright (c) 2025 Duke University
# All rights reserved.

import argparse
import os
import sys
from pathlib import Path

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.params import NULL
from m5.util import addToPath, fatal, warn
from gem5.isas import ISA
# from gem5.runtime import get_runtime_isa

from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.cc_cache_hierarchy import CheckerCacheHierarchy
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.cc_processor import CC_Processor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import CustomResource, FileResource
from gem5.simulate.simulator import Simulator

# Import gem5's common options
# from ../../common.

# from common import Options
# from common import Simulation
import Options
import Simulation

def get_processes(args):
    """Interprets provided args and returns a list of processes"""
    multiprocesses = []
    inputs = []
    outputs = []
    errouts = []
    pargs = []

    workloads = args.cmd.split(";")
    if args.input != "":
        inputs = args.input.split(";")
    if args.output != "":
        outputs = args.output.split(";")
    if args.errout != "":
        errouts = args.errout.split(";")
    if args.options != "":
        pargs = args.options.split(";")

    idx = 0
    for wrkld in workloads:
        process = Process(pid=100 + idx)
        process.executable = wrkld
        process.cwd = os.getcwd()
        process.gid = os.getgid()

        if args.env:
            with open(args.env, "r") as f:
                process.env = [line.rstrip() for line in f]

        if len(pargs) > idx:
            process.cmd = [wrkld] + pargs[idx].split()
        else:
            process.cmd = [wrkld]

        if len(inputs) > idx:
            process.input = inputs[idx]
        if len(outputs) > idx:
            process.output = outputs[idx]
        if len(errouts) > idx:
            process.errout = errouts[idx]

        multiprocesses.append(process)
        idx += 1

    return multiprocesses, 1

def main():
    # Create the parser
    parser = argparse.ArgumentParser(description="Run SPEC benchmarks with gem5")
    
    # Add gem5's common options
    Options.addCommonOptions(parser)
    Options.addSEOptions(parser)
    
    # Add our custom options
    parser.add_argument("--cmd", type=str, 
                        help="Path to the benchmark executable")
    parser.add_argument("--options", type=str, default="",
                        help="Command line options for the benchmark")
    parser.add_argument("--input", type=str, default="",
                        help="Input file for the benchmark")
    parser.add_argument("--output", type=str, default="",
                        help="Output file for the benchmark")
    parser.add_argument("--errout", type=str, default="",
                        help="Error output file for the benchmark")
    parser.add_argument("--maxinsts", type=int, default=0,
                        help="Maximum number of instructions to simulate")
    parser.add_argument("--num-cpus", type=int, default=1,
                        help="Number of CPUs to simulate")
    parser.add_argument("--cpu-type", type=str, default="O3",
                        help="CPU type to simulate")
    parser.add_argument("--l1d-size", type=str, default="32kB",
                        help="L1 data cache size")
    parser.add_argument("--l1i-size", type=str, default="32kB",
                        help="L1 instruction cache size")
    parser.add_argument("--l2-size", type=str, default="16MB",
                        help="L2 cache size")
    parser.add_argument("--l3-size", type=str, default="16MB",
                        help="L3 cache size")
    parser.add_argument("--mem-size", type=str, default="8GiB",
                        help="Memory size")
    parser.add_argument("--clk-freq", type=str, default="3GHz",
                        help="Clock frequency")
    
    # Parse the arguments
    args = parser.parse_args()
    
    # Get the processes
    multiprocesses, numThreads = get_processes(args)
    
    # Create the cache hierarchy
    cache_hierarchy = CheckerCacheHierarchy(
        l1d_size=args.l1d_size,
        l1i_size=args.l1i_size,
        l2_size=args.l2_size,
        l3_size=args.l3_size,
    )
    
    # Create the memory
    memory = SingleChannelDDR3_1600(args.mem_size)
    
    # Create the processor
    processor = CC_Processor(
        cpu_type=CPUTypes.O3, 
        num_cores=args.num_cpus, 
        isa=ISA.X86
    )
    
    # Create the board
    board = SimpleBoard(
        clk_freq=args.clk_freq,
        processor=processor,
        memory=memory,
        cache_hierarchy=cache_hierarchy
    )
    
    # Set up the workload
    if len(multiprocesses) > 0:
        process = multiprocesses[0]
        
        # Create the binary resource
        binary = CustomResource(process.executable)
        
        # Create the input file resource if needed
        input_resource = None
        if process.input:
            input_resource = FileResource(local_path=process.input)
        
        # Set up the workload
        board.set_se_binary_workload(
            binary=binary,
            arguments=process.cmd[1:] if len(process.cmd) > 1 else [],
            stdin_file=input_resource,
            stdout_file=Path(process.output) if process.output else None,
            stderr_file=Path(process.errout) if process.errout else None
        )
    
    # Create the simulator
    simulator = Simulator(board=board)
    
    # Set maximum instructions if specified
    if args.maxinsts > 0:
        simulator.schedule_max_insts(inst=args.maxinsts)
    
    # Run the simulation
    simulator.run()

if __name__ == "__main__":
    main() 