from m5.objects.ClockedObject import ClockedObject
from m5.params import *
from m5.proxy import *
from m5.objects.Cache import Cache


class CC_BankedCache(Cache):
    type = "CC_BankedCache"
    cxx_header = "checker_chip/checker_cache/cc_banked_cache.hh"
    cxx_class = "gem5::CC_BankedCache"

    num_banks = Param.Int(128, "Number of banks in the cache") # set in python/cachierarchies defn of banked cache

    # Corrected port name and type
    cc_cpu_port = VectorResponsePort("CC CPU port") #used SlavePort in documentation always?

    # cc_cpu_port = VectorResponsePort("CC CPU side port, receives requests")
