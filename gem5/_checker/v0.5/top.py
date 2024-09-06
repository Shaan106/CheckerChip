from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.resources.resource import Resource
from gem5.simulate.simulator import Simulator
from gem5.isas import ISA

# Import our custom processor
from tracing_cpu import InstructionTracingProcessor

# Create components
cache_hierarchy = NoCache()
memory = SingleChannelDDR3_1600("8GiB")
processor = InstructionTracingProcessor(cpu_type=CPUTypes.TIMING, num_cores=1, isa=ISA.X86, buffer_size=1000)

# Setup the board
board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy
)

# Load the binary
binary = Resource("x86-hello64-static")
board.set_se_binary_workload(binary)

# Create and run the simulation
simulator = Simulator(board=board)

# Run the simulation
simulator.run()

# After simulation, process the instruction stream
instruction_stream = processor.get_instruction_stream()
for pc, instruction, result in instruction_stream:
    print(f"PC: {pc}, Instruction: {instruction}, Result: {result}")

# Clear the instruction stream if needed
processor.clear_instruction_stream()