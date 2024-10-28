// cc_tlb.cc

#include "checker_chip/cc_tlb.hh"
#include <algorithm>

namespace gem5
{

CheckerTLB::CheckerTLB(unsigned int num_entries, unsigned int associativity,
                       unsigned int hit_latency, unsigned int miss_latency)
    : num_entries(num_entries),
      associativity(associativity),
      hit_latency(hit_latency),
      miss_latency(miss_latency)
{
    // Calculate the number of sets
    num_sets = num_entries / associativity;

    // Initialize the sets
    sets.resize(num_sets);
}

unsigned int CheckerTLB::translate(Addr vaddr)
{
    // Compute set index (e.g., using bits from the virtual address)
    unsigned int page_offset_bits = 12; // Assuming 4KB pages, standard x86
    unsigned int set_index = (vaddr >> page_offset_bits) % num_sets;

    auto &current_set = sets[set_index];

    // Check for a hit
    auto it = std::find(current_set.begin(), current_set.end(), vaddr);
    if (it != current_set.end()) {
        // TLB hit: move to front to model LRU
        current_set.erase(it);
        current_set.push_front(vaddr);
        return hit_latency;
    } else {
        // TLB miss
        // Add to TLB and handle replacement if necessary
        if (current_set.size() >= associativity) {
            // Evict the least recently used (back of the list)
            current_set.pop_back();
        }
        // Add the new entry to the front
        current_set.push_front(vaddr);
        return miss_latency;
    }
}

} // namespace gem5
