from m5.objects.ClockedObject import ClockedObject
from m5.params import *
from m5.proxy import *
from m5.objects.Cache import Cache


class CC_BankedCache(Cache):
    type = "CC_BankedCache"
    cxx_header = "checker_chip/checker_cache/cc_banked_cache.hh"
    cxx_class = "gem5::CC_BankedCache"

    num_banks = Param.Int(4, "Number of banks in the cache")