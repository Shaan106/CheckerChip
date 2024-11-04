// cc_tlb.cc

#include "checker_chip/cc_tlb.hh"
#include <algorithm>

namespace gem5
{

CheckerTLB::CheckerTLB(unsigned int num_entries, unsigned int associativity,
                       unsigned int hit_latency, unsigned int miss_latency)
    : num_entries(num_entries), //total slots
      associativity(associativity), //assoc
      hit_latency(hit_latency), //time to hit (cc_cycles)
      miss_latency(miss_latency) //time to miss (cc_cycles)
{
    //num sets
    num_sets = num_entries / associativity;

    //ensures sets size correct
    sets.resize(num_sets);
}

unsigned int CheckerTLB::translate(Addr vaddr)
{
    //returns time to miss/time to hit given some addr, updates internal structure.

    //compute set index from vaddr
    unsigned int page_offset_bits = 12; // assuming 4KB pages, standard x86
    unsigned int set_index = (vaddr >> page_offset_bits) % num_sets;

    auto &current_set = sets[set_index];

    //hit?
    auto it = std::find(current_set.begin(), current_set.end(), vaddr);
    if (it != current_set.end()) {
        //tlb hit, lru update
        current_set.erase(it);
        current_set.push_front(vaddr);
        return hit_latency;
    } else {
        //miss    
        if (current_set.size() >= associativity) {
            //evict
            current_set.pop_back();
        }
        //move into tlb
        current_set.push_front(vaddr);
        return miss_latency;
    }
}

} // namespace gem5
