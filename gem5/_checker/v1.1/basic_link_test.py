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
processor = SimpleProcessor(cpu_type=CPUTypes.O3, num_cores=4, isa=ISA.X86)

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

# ./build/X86/gem5.opt --debug-flags=CC_Buffer_Flag _checker/v1.1/basic_link_test.py
# additonal: --debug-flags=Commit