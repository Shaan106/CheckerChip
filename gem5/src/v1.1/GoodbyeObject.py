from m5.params import *
from m5.SimObject import SimObject

class GoodbyeObject(SimObject):
    type = "GoodbyeObject"
    cxx_header = "v1.1/goodbye_object.hh"
    cxx_class = "gem5::GoodbyeObject"

    buffer_size = Param.MemorySize(
        "1kB", "Size of buffer to fill with goodbye"
    )
    write_bandwidth = Param.MemoryBandwidth(
        "100MB/s", "Bandwidth to fill the buffer"
    )
