from m5.params import *
from m5.SimObject import SimObject

class CC_Buffer(SimObject):
    type = "CC_Buffer"
    cxx_header = "checker_chip/cc_buffer.hh"
    cxx_class = "gem5::CC_Buffer"

    buffer_size = Param.Int(20, "Max Credits. How many items the buffer can hold")

    # buffer_size = Param.MemorySize(
    #     "1kB", "Size of buffer to fill with goodbye"
    # )
    # write_bandwidth = Param.MemoryBandwidth(
    #     "100MB/s", "Bandwidth to fill the buffer"
    # )

    # also a change in BaseO3CPU.py
    
    '''
    in BaseO3CPU.py
    from m5.objects.CC_Buffer import CC_Buffer
    cc_buffer = CC_Buffer(CC_Buffer(), "A CC_Buffer object, default initialized")

    in commit.hh
    #include "checker_chip/cc_buffer.hh"
    and
    CC_Buffer* cc_buffer;

    in commit.cc
    cc_buffer->pushCommit(name());
    goodbye->sayGoodbye(myName);
    '''