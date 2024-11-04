
from typing import Type

from m5.objects import (
    BasePrefetcher,
    CC_SimpleCache,
    Clusivity,
    StridePrefetcher,
)

from .....utils.override import *


# from m5.objects.ClockedObject import ClockedObject
# from m5.params import *
# from m5.proxy import *


# class CC_SimpleCache(ClockedObject):
#     type = "CC_SimpleCache"
#     cxx_header = "checker_chip/checker_cache/cc_simple_cache.hh"
#     cxx_class = "gem5::CC_SimpleCache"

#     # Vector port example. Both the instruction and data ports connect to this
#     # port which is automatically split out into two ports.
#     cpu_side = VectorResponsePort("CPU side port, receives requests")
#     mem_side = RequestPort("Memory side port, sends requests")

#     latency = Param.Cycles(1, "Cycles taken on a hit or to resolve a miss")

#     size = Param.MemorySize("16kB", "The size of the cache")

#     system = Param.System(Parent.any, "The system this cache is part of")

class CC_CacheInstance(CC_SimpleCache):
    """
    A simple CC Cache with default values.
    """

    def __init__(
        self,
        size: str,
        # assoc: int = 16,
        # tag_latency: int = 10,
        # data_latency: int = 10,
        # response_latency: int = 1,
        # mshrs: int = 20,
        # tgts_per_mshr: int = 12, 
        # writeback_clean: bool = False,
        # clusivity: Clusivity = "mostly_incl",
        # PrefetcherCls: Type[BasePrefetcher] = StridePrefetcher,
    ):
        super().__init__()
        self.size = size
        # self.assoc = assoc
        # self.tag_latency = tag_latency
        # self.data_latency = data_latency
        # self.response_latency = response_latency
        # self.mshrs = mshrs
        # self.tgts_per_mshr = tgts_per_mshr
        # self.writeback_clean = writeback_clean
        # self.clusivity = clusivity
        # self.prefetcher = PrefetcherCls()
