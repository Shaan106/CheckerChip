
from typing import Type

from m5.objects import (
    CC_BankedCache,
    BasePrefetcher,
    Clusivity,
    StridePrefetcher,
)

from .....utils.override import *


class CC_CacheInstance(CC_BankedCache):
    """
    A simple CC Cache with default values.
    """

    def __init__(
        self,
        size: str,
        assoc: int = 16,
        tag_latency: int = 10,
        data_latency: int = 10,
        response_latency: int = 1,
        mshrs: int = 20,
        tgts_per_mshr: int = 12, 
        writeback_clean: bool = False,
        clusivity: Clusivity = "mostly_incl",
        PrefetcherCls: Type[BasePrefetcher] = StridePrefetcher,

        num_banks: int = 128, #custom added
    ):
        super().__init__()
        self.size = size
        self.assoc = assoc
        self.tag_latency = tag_latency
        self.data_latency = data_latency
        self.response_latency = response_latency
        self.mshrs = mshrs
        self.tgts_per_mshr = tgts_per_mshr
        self.writeback_clean = writeback_clean
        self.clusivity = clusivity
        self.prefetcher = PrefetcherCls()

        self.num_banks = num_banks
