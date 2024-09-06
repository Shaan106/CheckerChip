basic hello-world.py, no flavour

to run:

```sh
./build/X86/gem5.opt ./_checker/v0.1/hello-world.py 
```

Output:
```
ay140@clipper01:~/CheckerChip/gem5$ ./build/X86/gem5.opt ./_checker/v0.1/hello-world.py 
gem5 Simulator System.  https://www.gem5.org
gem5 is copyrighted software; use the --copyright option for details.

gem5 version 23.1.0.0
gem5 compiled Mar 21 2024 19:58:08
gem5 started Sep  2 2024 22:17:56
gem5 executing on clipper01, pid 2403834
command line: ./build/X86/gem5.opt ./_checker/v0.1/hello-world.py

warn: `Resource` has been deprecated. Please use the `obtain_resource` function instead.
info: Using default config
Global frequency set at 1000000000000 ticks per second
warn: No dot file generated. Please install pydot to generate the dot file and pdf.
src/mem/dram_interface.cc:690: warn: DRAM device capacity (8192 Mbytes) does not match the address range assigned (1024 Mbytes)
src/base/statistics.hh:279: warn: One of the stats is a legacy stat. Legacy stat is a stat that does not belong to any statistics::Group. Legacy stat is deprecated.
board.remote_gdb: Listening for connections on port 7000
src/sim/simulate.cc:199: info: Entering event queue @ 0.  Starting simulation...
src/sim/syscall_emul.hh:1014: warn: readlink() called on '/proc/self/exe' may yield unexpected results in various settings.
      Returning '/home/ay140/.cache/gem5/x86-hello64-static'
src/sim/mem_state.cc:448: info: Increasing stack size by one page.
Hello world!
```