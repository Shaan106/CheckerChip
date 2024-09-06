from gem5.components.processors.cpu_types import CPUTypes
# from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.isas import ISA
from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject
from m5.objects import TimingSimpleCPU

from override import overrides

class InstructionStreamingCPU(TimingSimpleCPU):
    type = 'InstructionStreamingCPU'
    cxx_header = "cpu/simple/timing.hh"
    cxx_class = 'gem5::TimingSimpleCPU'

    # buffer_size = Param.Int(1000, "Size of the instruction buffer")
    # buffer_size = 1000

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.instruction_buffer = []

    # @overrides(TimingSimpleCPU)
    def _instruction_executed(self, instruction):
        opcode = instruction.opcode
        operands = instruction.operands
        if len(self.instruction_buffer) >= 1000:
            self.instruction_buffer.pop(0)
        self.instruction_buffer.append((opcode, operands))

    # @overrides(TimingSimpleCPU)
    def clear_instruction_stream(self):
        self.instruction_buffer.clear()

class InstructionStreamingProcessor(SimpleProcessor):
    def __init__(self, cpu_type=CPUTypes.TIMING, num_cores=1, isa=ISA.X86):
        super().__init__(cpu_type=cpu_type, num_cores=num_cores, isa=isa)
        # self.instruction_buffer = []
        self.instruction_buffer = []

    # # @overrides(SimpleProcessor)
    # def _create_cpu(self):
    #     return InstructionStreamingCPU(buffer_size=self.buffer_size)
    
    # @overrides(TimingSimpleCPU)
    def get_instruction_stream(self):
        # return list(self.instruction_buffer)
        return [1, 2, 3]
    
    # def _instruction_executed(self, instruction):
    #     opcode = instruction.opcode
    #     operands = instruction.operands
    #     if len(self.instruction_buffer) >= 1000:
    #         self.instruction_buffer.pop(0)
    #     self.instruction_buffer.append((opcode, operands))