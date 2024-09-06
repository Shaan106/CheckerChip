
from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.processors.base_cpu_processor import BaseCPUProcessor
# from gem5.components.processors.simple_core import SimpleCore
from gem5.isas import ISA

from Streaming_II_Core import Streaming_II_Core

class Streaming_II_Processor(BaseCPUProcessor):
    """
    A SimpleProcessor contains a number of cores of SimpleCore objects of the
    same CPUType.
    """

    def __init__(self, cpu_type: CPUTypes, num_cores: int, isa: ISA) -> None:
        """
        :param cpu_type: The CPU type for each type in the processor.

        :param num_cores: The number of CPU cores in the processor.

        :param isa: The ISA of the processor.
        """
        super().__init__(
            cores=[
                Streaming_II_Core(cpu_type=cpu_type, core_id=i, isa=isa)
                for i in range(num_cores)
            ]
        )
        self.opcode_log = Param.Int(1, "opcode, hopefully")

    def print_opcode_log(self):
        print(self.opcode_log)
