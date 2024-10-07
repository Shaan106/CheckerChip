from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import Resource
from gem5.simulate.simulator import Simulator

from gem5.isas import ISA

#essentially you get a "virtual motherboard"
# and add all your "virtual" components to said board

# no cache
cache_hierarchy = NoCache()
# type of RAM, what size
memory = SingleChannelDDR3_1600("8GiB")

#processor
#single core processor, 1 timing CPU processor
#timing is what type of gem5 simulation (also atomic etc)
processor = SimpleProcessor(cpu_type=CPUTypes.O3, num_cores=8, isa=ISA.X86)
# processor = SimpleProcessor(cpu_type=CPUTypes.TIMING, num_cores=1, isa=ISA.X86)
# ./build/X86/gem5.opt --debug-flags=Commit _checker/v0.1/hello-world.py
# setting ISA not in tutorial, checked actual files to fix.

# setup the board with all the components
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
ay140@clipper01:~/CheckerChip/gem5$ ./build/X86/gem5.opt ./playground/hello-world.py 
gem5 Simulator System.  https://www.gem5.org
gem5 is copyrighted software; use the --copyright option for details.

gem5 version 23.1.0.0
gem5 compiled Mar 21 2024 19:58:08
gem5 started Jun 16 2024 15:10:43
gem5 executing on clipper01, pid 2522187
command line: ./build/X86/gem5.opt ./playground/hello-world.py

warn: `Resource` has been deprecated. Please use the `obtain_resource` function instead.
info: Using default config
Resource 'x86-hello64-static' was not found locally. Downloading to '/home/ay140/.cache/gem5/x86-hello64-static'...
Finished downloading resource 'x86-hello64-static'.
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