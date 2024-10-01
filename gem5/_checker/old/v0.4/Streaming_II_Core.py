
import importlib
import platform
from typing import Optional

from gem5.isas import ISA
from requires import requires
from gem5.components.processors.base_cpu_core import BaseCPUCore
from gem5.components.processors.cpu_types import CPUTypes


class Streaming_II_Core(BaseCPUCore):
    """
    A `SimpleCore` instantiates a core based on the CPUType enum pass. The
    `SimpleCore` creates a single `SimObject` of that type.
    """

    def __init__(self, cpu_type: CPUTypes, core_id: int, isa: ISA):
        requires(isa_required=isa)
        super().__init__(
            core=Streaming_II_Core.cpu_simobject_factory(
                isa=isa, cpu_type=cpu_type, core_id=core_id
            ),
            isa=isa,
        )

        self._cpu_type = cpu_type

    def get_type(self) -> CPUTypes:
        return self._cpu_type

    @classmethod
    def cpu_class_factory(cls, cpu_type: CPUTypes, isa: ISA) -> type:
        """
        A factory used to return the SimObject type  given the cpu type,
        and ISA target. An exception will be thrown if there is an
        incompatibility.

        :param cpu_type: The target CPU type.
        :param isa: The target ISA.
        """

        assert isa is not None
        requires(isa_required=isa)

        _isa_string_map = {
            ISA.X86: "X86",
            ISA.ARM: "Arm",
            ISA.RISCV: "Riscv",
            ISA.SPARC: "Sparc",
            ISA.POWER: "Power",
            ISA.MIPS: "Mips",
        }

        _cpu_types_string_map = {
            CPUTypes.ATOMIC: "AtomicSimpleCPU",
            CPUTypes.O3: "O3CPU",
            CPUTypes.TIMING: "TimingSimpleCPU",
            CPUTypes.KVM: "KvmCPU",
            CPUTypes.MINOR: "MinorCPU",
        }

        if isa not in _isa_string_map:
            raise NotImplementedError(
                f"ISA '{isa.name}' does not have an"
                "entry in `AbstractCore.cpu_simobject_factory._isa_string_map`"
            )

        if cpu_type not in _cpu_types_string_map:
            raise NotImplementedError(
                f"CPUType '{cpu_type.name}' "
                "does not have an entry in "
                "`AbstractCore.cpu_simobject_factory._cpu_types_string_map`"
            )

        if cpu_type == CPUTypes.KVM:
            # For some reason, the KVM CPU is under "m5.objects" not the
            # "m5.objects.{ISA}CPU".
            module_str = f"m5.objects"
        else:
            module_str = f"m5.objects.{_isa_string_map[isa]}CPU"

        # GEM5 compiles two versions of KVM for ARM depending upon the host CPU
        # : ArmKvmCPU and ArmV8KvmCPU for 32 bit (Armv7l) and 64 bit (Armv8)
        # respectively.

        if (
            isa.name == "ARM"
            and cpu_type == CPUTypes.KVM
            and platform.architecture()[0] == "64bit"
        ):
            cpu_class_str = (
                f"{_isa_string_map[isa]}V8"
                f"{_cpu_types_string_map[cpu_type]}"
            )
        else:
            cpu_class_str = (
                f"{_isa_string_map[isa]}" f"{_cpu_types_string_map[cpu_type]}"
            )

        try:
            to_return_cls = getattr(
                importlib.import_module(module_str), cpu_class_str
            )
        except ImportError:
            raise Exception(
                f"Cannot find CPU type '{cpu_type.name}' for '{isa.name}' "
                "ISA. Please ensure you have compiled the correct version of "
                "gem5."
            )

        return to_return_cls

    @classmethod
    def cpu_simobject_factory(
        cls, cpu_type: CPUTypes, isa: ISA, core_id: int
    ) -> BaseCPUCore:
        """
        A factory used to return the SimObject core object given the cpu type,
        and ISA target. An exception will be thrown if there is an
        incompatibility.

        :param cpu_type: The target CPU type.
        :param isa: The target ISA.
        :param core_id: The id of the core to be returned.
        """

        return cls.cpu_class_factory(cpu_type=cpu_type, isa=isa)(
            cpu_id=core_id
        )
