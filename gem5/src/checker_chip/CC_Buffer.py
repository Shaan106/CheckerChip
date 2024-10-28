from m5.params import *
from m5.objects.FUPool import *
from m5.objects.ClockedObject import ClockedObject

class CC_Buffer(ClockedObject):
    type = "CC_Buffer"
    cxx_header = "checker_chip/cc_buffer.hh"
    cxx_class = "gem5::CC_Buffer"

    maxCredits = Param.Int(128, "Max Credits. How many items (credits) the buffer can hold")
    checkerFUPool = Param.FUPool(CheckerFUPool(), "Functional Unit pool for cc_buffer")
    tlbEntries = Param.Int(64, "Total number of entries in the TLB")
    tlbAssociativity = Param.Int(4, "Associativity of the TLB")
    tlbHitLatency = Param.Cycles(1, "Latency for TLB hits")
    tlbMissLatency = Param.Cycles(10, "Latency for TLB misses")

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