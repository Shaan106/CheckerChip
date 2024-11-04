import m5
from m5.objects import *
import os

# Add the common scripts to our path
m5.util.addToPath("../")

# Import the caches which we made
from caches import *

# Default to running 'hello', use the compiled ISA to find the binary
thispath = os.path.dirname(os.path.realpath(__file__))
default_binary = os.path.join(
    thispath,
    "../../",
    "tests/test-progs/hello/bin/x86/linux/hello",
)

# Create the system we are going to simulate
system = System()

# Set the clock frequency of the system (and all of its children)
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()

# Set up the system
system.mem_mode = "timing"  # Use timing accesses
system.mem_ranges = [AddrRange("512MB")]  # Create an address range

# Create a list of 8 O3 CPUs
system.cpu = [X86O3CPU(cpu_id=i) for i in range(8)]

# Create per-core L1 instruction and data caches
for cpu in system.cpu:
    cpu.icache = L1ICache()
    cpu.dcache = L1DCache()
    # Connect the instruction and data caches to the CPU
    cpu.icache.connectCPU(cpu)
    cpu.dcache.connectCPU(cpu)

# Create a coherent crossbar (L2 bus)
system.l2bus = L2XBar()

# Hook the CPU caches up to the L2 bus
for cpu in system.cpu:
    cpu.icache.connectBus(system.l2bus)
    cpu.dcache.connectBus(system.l2bus)

# Create an L2 cache and connect it to the L2 bus
system.l2cache = L2Cache()
system.l2cache.connectCPUSideBus(system.l2bus)

# Create a memory bus
system.membus = SystemXBar()

# Connect the L2 cache to the memory bus
system.l2cache.connectMemSideBus(system.membus)

# Create the interrupt controllers for the CPUs
for cpu in system.cpu:
    cpu.createInterruptController()
    # For x86, interrupts need to be connected to the memory bus
    cpu.interrupts[0].pio = system.membus.mem_side_ports
    cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports
    cpu.interrupts[0].int_responder = system.membus.mem_side_ports

# Connect the system port to the memory bus
system.system_port = system.membus.cpu_side_ports

# Create a DDR3 memory controller and connect it to the memory bus
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

system.workload = SEWorkload.init_compatible(default_binary)

# Create a process for each CPU with a unique PID
processes = [Process(pid=100 + i, cmd=[default_binary]) for i in range(8)]

# Assign the processes to the CPUs
for i in range(8):
    system.cpu[i].workload = processes[i]
    system.cpu[i].createThreads()

# Set up the root SimObject and start the simulation
root = Root(full_system=False, system=system)
m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
