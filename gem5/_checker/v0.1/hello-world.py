from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.processors.cpu_types import CPUTypes
from gem5.resources.resource import Resource
from gem5.resources.resource import obtain_resource
from gem5.resources.resource import CustomResource
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
# binary = obtain_resource(resource_id="x86-floatmm")
# binary = CustomResource("/home/ay140/CheckerChip/gem5/_checker/v0.1/SAXPY/saxpy_static")
# binary = "/SAXPY/saxpy"
#setting workload to current board (we are going to run in SE mode)
board.set_se_binary_workload(binary)

# want to run a simulation with the board we have specified (the one just created)
simulator = Simulator(board=board)
simulator.run()