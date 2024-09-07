## The gem5 standard library

python/gem5/components/ contains all of the components.

All tutorial work is within /playground/

## initial build and stuff

`scons build/X86/gem5.opt -j<cores>`

## make buffer (simobject) notes

### part 1
[gem5 learn docs](https://www.gem5.org/documentation/learning_gem5/part2/debugging/)

`./build/X86/gem5.opt --debug-help` provides all of the debug flags. These would be useful for printing debug statements for example when changing commit source code.

ie for commit: `./build/X86/gem5.opt --debug-flags=Commit _checker/v0.1/hello-world.py`
only top few statements: `./build/X86/gem5.opt --debug-flags=Commit _checker/v0.1/hello-world.py | head -n 50`

### part 2 - custom debug flags

use new custom debug flag with `./build/X86/gem5.opt --debug-flags=CustomSimObjectFlag `
## other
[gem5 cpu docs](https://www.gem5.org/documentation/general_docs/cpu_models/O3CPU##Pipeline-stages)