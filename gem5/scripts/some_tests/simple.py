
# import all the modules from the gem5 library
import m5
from m5.objects import *


# Sim object - basically what we are going to simulate
system = System()

# setting up the clock and the voltage domain
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain() #don't care about power right now


# memory mode
system.mem_mode = 'timing' #almost always timing mode
system.mem_ranges = [AddrRange('512MB')]


#creating the cpu
system.cpu = X86TimingSimpleCPU()

# system wide memory bus
system.membus = SystemXBar()

# caches - here there is no cache therfore connecting the cpu directly to the memory bus
system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports


# special x86 requirement to connect the interrupt port to the memory bus
system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.mem_side_ports
system.cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports
system.cpu.interrupts[0].int_responder = system.membus.mem_side_ports

system.system_port = system.membus.cpu_side_ports

#memory controller
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports


# basic program to run on x86 cpu
# another sim object
binary = 'tests/test-progs/hello/bin/x86/linux/hello'

system.workload = SEWorkload.init_compatible(binary)
process = Process()
process.cmd = [binary]
system.cpu.workload = process
system.cpu.createThreads()


# running sim!!
print("Beginning simulation!")
exit_event = m5.simulate()

print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))
