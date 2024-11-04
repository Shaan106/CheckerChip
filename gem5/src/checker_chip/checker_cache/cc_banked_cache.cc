// cc_banked_cache.cc

#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "debug/CC_BankedCache.hh"

namespace gem5
{

CC_BankedCache::CC_BankedCache(const CC_BankedCacheParams &p)
    : Cache(p),
    numBanks(p.num_banks)
{
    // No additional initialization needed for now
}

unsigned
CC_BankedCache::calculateBankId(Addr addr) const
{
    // Simple modulo-based bank mapping
    return (addr / blkSize) % numBanks; // !!!!! can probably do this with bitmasks, would be faster !!!!!!
}

bool
CC_BankedCache::access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,
                    PacketList &writebacks)
{
    // Determine the bank ID based on the address
    unsigned bankId = calculateBankId(pkt->getAddr());

    // print bank ID
    DPRINTF(CC_BankedCache, "Bank ID: %u\n", bankId);

    return Cache::access(pkt, blk, lat, writebacks);
}

} // namespace gem5
