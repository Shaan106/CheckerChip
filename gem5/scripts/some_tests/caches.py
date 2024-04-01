from m5.objects import Cache

#extending the base cache to create whatever we want
class L1Cache(Cache):
    #setting some values that do not have default values
    assoc = 2
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    #figure out the implementation a bit more?
    mshrs = 4
    tgts_per_mshr = 20

#instruction cache
class L1ICache(L1Cache):
    size = '16kB'
    # this defines how the cache connects itself to the cpu
    def connectCPU(self, cpu):
        #the cache's CPU side connects to the CPU's instruction cache port
        self.cpu_side = cpu.icache_port

#data cache
class L1DCache(L1Cache):
    size = '64kB'
    def connectCPU(self, cpu):
        self.cpu_side = cpu.dcache_port

# l2 cache - again going to build off of base cache
class L2Cache(Cache):
    size = '256kB'
    assoc = 8
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12

    #connecting the buses from/to CPU for cache
    def connectCPUSideBus(self, bus):
      self.cpu_side = bus.mem_side_ports

    def connectMemSideBus(self, bus):
      self.mem_side = bus.cpu_side_ports


# connecting the defined caches to bigger system
# def connectCPU(self, cpu):
#     # need to define this in a base class!
#     raise NotImplementedError

# def connectBus(self, bus):
#     self.mem_side = bus.cpu_side_ports