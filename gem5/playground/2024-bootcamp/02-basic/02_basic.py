from gem5.prebuilt.demo.x86_demo_board import X86DemoBoard
from gem5.resources.resource import obtain_resource
from gem5.simulate.simulator import Simulator

# using a basic pre-built board
board = X86DemoBoard()

"""
obtain_resource downloads the files needed to run workload
Boots Ubuntu without systemd then exits the simulation
Downloads disk image, kernel, and sets default parameters
"""

board.set_workload(
    obtain_resource("x86-ubuntu-24.04-boot-no-systemd")
)

# creating a simulator to actually run:
sim = Simulator(board)
sim.run(20_000_000_000) # 20 billion ticks or 20 ms

# running this model:
# gem5-mesi basic.py <-- from the bootcamp
# ./build/X86/gem5.opt playground/2024-bootcamp/02_basic.py <-- what actually works for clipper

"""
output:

gem5's output

In m5out/ you'll see:

stats.txt: The statistics from the simulation.
board.pc.com_1.device: The console output from the simulation.
citations.bib: Citations for the models and resources used.
config.ini/json: The configuration file used.
config*.pdf/svg: A visualization of the configuration for the system and the caches.
"""