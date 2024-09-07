from m5.params import *
from m5.SimObject import SimObject

class CustomSimObject(SimObject):
    type = "CustomSimObject"

    # these are files for binding the C++ class to the Python class
    # header files contain info about the class files (.cc). Mainly used to show other classes the structure of classes without passing lots of data
    cxx_header = "v1.1/custom_sim_object.hh"# corresponding header that defines class in C++
    cxx_class = "gem5::CustomSimObject" # Full name of the C++ class

    # Parameters
    # These are the parameters that can be set in the Python script, and can be used in the C++ class
    time_to_wait = Param.Latency("Time before firing the event")
    number_of_fires = Param.Int(1, "Number of times to fire the event before "
                                   "goodbye") # default value is 1, set by the first argument
    
    # param of type RecieverSimObject
    goodbye_object = Param.GoodbyeObject("A goodbye object")

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
