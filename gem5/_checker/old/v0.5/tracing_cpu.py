from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.isas import ISA
from m5.objects import TimingSimpleCPU

class InstructionTracer:
    def __init__(self, buffer_size=1000):
        self.buffer_size = buffer_size
        self.instruction_buffer = []

    def trace_instruction(self, when, cpu, pc, instruction, result):
        if len(self.instruction_buffer) >= self.buffer_size:
            self.instruction_buffer.pop(0)
        self.instruction_buffer.append((pc, instruction, result))

    def get_instruction_stream(self):
        return list(self.instruction_buffer)

    def clear_instruction_stream(self):
        self.instruction_buffer.clear()

class InstructionTracingCPU(TimingSimpleCPU):
    def __init__(self, tracer, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.tracer = tracer

    def trace_instruction(self, when, pc, instruction, result):
        self.tracer.trace_instruction(when, self, pc, instruction, result)

class InstructionTracingProcessor(SimpleProcessor):
    def __init__(self, cpu_type=CPUTypes.TIMING, num_cores=1, isa=ISA.X86, buffer_size=1000):
        super().__init__(cpu_type=cpu_type, num_cores=num_cores, isa=isa)
        self.tracer = InstructionTracer(buffer_size)

    def _create_cpu(self):
        return InstructionTracingCPU(tracer=self.tracer)

    def get_instruction_stream(self):
        return self.tracer.get_instruction_stream()

    def clear_instruction_stream(self):
        self.tracer.clear_instruction_stream()