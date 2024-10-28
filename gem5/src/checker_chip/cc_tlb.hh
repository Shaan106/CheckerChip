// cc_tlb.hh

#ifndef cc_tlb_HH
#define cc_tlb_HH

#include <unordered_map>
#include "base/types.hh"
#include <vector>
#include <list>

namespace gem5
{

class CheckerTLB
{
  private:
    // TLB parameters
    unsigned int num_entries;
    unsigned int associativity;
    unsigned int num_sets;
    unsigned int hit_latency;
    unsigned int miss_latency;

    // TLB storage: vector of lists representing sets
    std::vector<std::list<Addr>> sets;

  public:
    CheckerTLB(unsigned int num_entries, unsigned int associativity,
               unsigned int hit_latency, unsigned int miss_latency);

    unsigned int translate(Addr vaddr);
};

} // namespace gem5

#endif // cc_tlb_HH
