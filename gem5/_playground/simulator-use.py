from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator

from gem5.isas import ISA

# Obtain the components.
cache_hierarchy = NoCache()

memory = SingleChannelDDR3_1600("1GiB")
processor = SimpleProcessor(cpu_type=CPUTypes.ATOMIC, num_cores=1, isa=ISA.X86)

# Add them to the board.
board = SimpleBoard(
    clk_freq="3GHz", processor=processor, memory=memory, cache_hierarchy=cache_hierarchy
)

# Set the workload.
binary = CustomResource(
    "playground/m5-exit-example/m5-exit-example"
)
board.set_se_binary_workload(binary)

# Setup the Simulator and run the simulation.
simulator = Simulator(board=board)
simulator.run()

print("Stuff after na m5 exit event. Before continuing rest of sim.")

simulator.run()

print("again?")

simulator.run()

'''
ay140@clipper01:~/CheckerChip/gem5$ ./build/X86/gem5.opt ./playground/simulator-use.py 
gem5 Simulator System.  https://www.gem5.org
gem5 is copyrighted software; use the --copyright option for details.

gem5 version 23.1.0.0
gem5 compiled Mar 21 2024 19:58:08
gem5 started Jun 18 2024 13:26:46
gem5 executing on clipper01, pid 2954921
command line: ./build/X86/gem5.opt ./playground/simulator-use.py

warn: The `CustomResource` class is deprecated. Please use an `AbstractResource` subclass instead.
Global frequency set at 1000000000000 ticks per second
warn: No dot file generated. Please install pydot to generate the dot file and pdf.
src/mem/dram_interface.cc:690: warn: DRAM device capacity (8192 Mbytes) does not match the address range assigned (1024 Mbytes)
src/base/statistics.hh:279: warn: One of the stats is a legacy stat. Legacy stat is a stat that does not belong to any statistics::Group. Legacy stat is deprecated.
board.remote_gdb: Listening for connections on port 7000
src/sim/simulate.cc:199: info: Entering event queue @ 0.  Starting simulation...
src/sim/mem_state.cc:448: info: Increasing stack size by one page.
src/sim/syscall_emul.cc:74: warn: ignoring syscall mprotect(...)
src/sim/syscall_emul.cc:74: warn: ignoring syscall mprotect(...)
src/sim/syscall_emul.cc:74: warn: ignoring syscall mprotect(...)
The program has started!
About to exit the simulation for the 1 st/nd/rd/st time
Stuff after na m5 exit event. Before continuing rest of sim.
src/sim/simulate.cc:199: info: Entering event queue @ 117198351.  Starting simulation...
About to exit the simulation for the 2 st/nd/rd/st time
again?
src/sim/simulate.cc:199: info: Entering event queue @ 118009872.  Starting simulation...
About to exit the simulation for the 3 st/nd/rd/st time
'''