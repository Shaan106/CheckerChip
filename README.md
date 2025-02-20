### Checker Chip Documentation

Live notes/documentation can be found [at this link](https://cypress-eye-9ef.notion.site/Checker-Chip-cb9033b81cb44369bb4a39c12b8e3f7c)

19 Feb Stable build on clipper

`scons build/X86/gem5.opt -j40; ./build/X86/gem5.opt _checker/v0.1/raxpy.py; cd _checker/v0.1; python plots.py; cd ../..`

## Running the model

*todo, scons -> gem5.opt*

## List of files changed in this project

Within `/src/`:

`checker_chip/*`

`o3/`
`