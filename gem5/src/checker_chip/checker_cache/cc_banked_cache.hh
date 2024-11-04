// cc_banked_cache.hh

#ifndef __CC_MEM_CACHE_BANKED_CACHE_HH__
#define __CC_MEM_CACHE_BANKED_CACHE_HH__

#include "mem/cache/cache.hh"
#include "params/CC_BankedCache.hh"

namespace gem5
{

class CC_BankedCache : public Cache
{
  public:
    // Constructor
    CC_BankedCache(const CC_BankedCacheParams &p);

    // Destructor
    ~CC_BankedCache() = default;

  protected:
    // For now, we don't override any methods
};

} // namespace gem5

#endif // __CC_MEM_CACHE_BANKED_CACHE_HH__
