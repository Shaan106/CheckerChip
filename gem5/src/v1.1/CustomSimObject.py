from m5.params import *
from m5.SimObject import SimObject
from m5.objects.GoodbyeObject import GoodbyeObject

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
    # default value is a GoodbyeObject with buffer_size of 100B
    goodbye_object = Param.GoodbyeObject(GoodbyeObject(buffer_size="100B"),
                                         "A goodbye object") 
    