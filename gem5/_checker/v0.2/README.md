
The components are located in (ie memory type)
`gem5/src/python/gem5/components/memory/single_channel.py`

`CPUTypes.O3` is the OOO processing type [documentation](https://www.gem5.org/documentation/general_docs/cpu_models/O3CPU)


- cannot add buffers via extending classes (wierd for some reason)

To print
`./build/X86/gem5.opt --debug-flags=ExecAll ./_checker/v0.1/hello-world.py > ./_checker/v0.2/trace.txt`

Output in trace.txt

to run:

```sh

```

Output:
```
```

make a custom CPU with buffer, insts get automatically put into buffer.