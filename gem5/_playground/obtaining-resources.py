from gem5.resources.resource import Resource

resource = Resource("riscv-disk-img")

print(f"The resource is available at {resource.get_local_path()}")

'''
ay140@clipper01:~/CheckerChip/gem5$ ./build/X86/gem5.opt ./playground/obtaining-resources.py 
gem5 Simulator System.  https://www.gem5.org
gem5 is copyrighted software; use the --copyright option for details.

gem5 version 23.1.0.0
gem5 compiled Mar 21 2024 19:58:08
gem5 started Jun 16 2024 14:51:01
gem5 executing on clipper01, pid 2518539
command line: ./build/X86/gem5.opt ./playground/obtaining-resources.py

warn: `Resource` has been deprecated. Please use the `obtain_resource` function instead.
info: Using default config
Resource 'riscv-disk-img' was not found locally. Downloading to '/home/ay140/.cache/gem5/riscv-disk-img.gz'...
Finished downloading resource 'riscv-disk-img'.
Decompressing resource 'riscv-disk-img' ('/home/ay140/.cache/gem5/riscv-disk-img.gz')...
Finished decompressing resource 'riscv-disk-img'.
The resource is available at /home/ay140/.cache/gem5/riscv-disk-img
'''