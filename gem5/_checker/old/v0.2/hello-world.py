from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import Resource
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA
from m5.objects import Root
import m5

# Enable ExecAll debug flag
# m5.trace.add_flag('ExecAll')
m5.debug.flags["ExecAll"].enable()

# Setup the components as you did before
cache_hierarchy = NoCache()
memory = SingleChannelDDR3_1600("1GiB")
processor = SimpleProcessor(cpu_type=CPUTypes.TIMING, num_cores=1, isa=ISA.X86)

board = SimpleBoard(clk_freq="3GHz", 
                    processor=processor, 
                    memory=memory,
                    cache_hierarchy=cache_hierarchy)

binary = Resource("x86-hello64-static")
board.set_se_binary_workload(binary)

simulator = Simulator(board=board)

# To output the trace in real-time
m5.trace.output("trace.txt")

# Run the simulation
simulator.run()

# Ensure to remove the flag after running if needed
# m5.trace.remove_flag('ExecAll')
m5.debug.flags["ExecAll"].disable()