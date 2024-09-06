from m5.params import *
from m5.SimObject import SimObject

class CustomSimObject(SimObject):
    type = "CustomSimObject"

    # these are files for binding the C++ class to the Python class
    # header files contain info about the class files (.cc). Mainly used to show other classes the structure of classes without passing lots of data
    cxx_header = "v1.1/custom_sim_object.hh"# corresponding header that defines class in C++
    cxx_class = "gem5::CustomSimObject" # Full name of the C++ class
    
    