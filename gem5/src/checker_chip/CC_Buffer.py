from m5.params import *
# from m5.SimObject import SimObject
from m5.objects.ClockedObject import ClockedObject #<--------------
# from m5.objects import SrcClockDomain, VoltageDomain

# class CC_Buffer(SimObject): # <----------- inherits from Clocked Object
class CC_Buffer(ClockedObject):
    type = "CC_Buffer"
    cxx_header = "checker_chip/cc_buffer.hh"
    cxx_class = "gem5::CC_Buffer"

    # # Define a default voltage domain
    # voltage_domain = VoltageDomain()

    # # Define a default clock domain with a specific clock frequency
    # clock_domain = SrcClockDomain(clock="2GHz", voltage_domain=voltage_domain)

    maxCredits = Param.Int(20, "Max Credits. How many items (credits) the buffer can hold")
    
    '''
    extra notes

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