// cc_bank_unit.cc

#include "checker_chip/checker_cache/cc_bank_unit.hh"
#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "debug/CC_BankedCache.hh"

namespace gem5 {

CC_BankUnit::CC_BankUnit()
{
    // Constructor
}

CC_BankUnit::~CC_BankUnit()
{
    // Destructor
}

void CC_BankUnit::clock_update()
{
    DPRINTF(CC_BankedCache, "Clock update called\n");

    // update core queues
    updateCoreQueues();

    // If we have a parent cache, call cc_dispatchEvent
    if (parentCache) {
        parentCache->cc_dispatchEvent(bankId);
    } else {
        DPRINTF(CC_BankedCache, "ERROR: No parent cache found\n");
    }

    // remove should be done here too in future.
}

bool CC_BankUnit::addPacket(PacketPtr pkt, int senderCoreID)
{

    if (mainQueue.size() >= maxMainQueueSize) {
        // Queue is full; cannot add the packet
        return false;
    } else {
        DPRINTF(CC_BankedCache, "Adding packet to main queue. senderCoreID: %d\n", senderCoreID);
        mainQueue.push_back(std::make_tuple(pkt, senderCoreID));
        return true;
    }
}

void CC_BankUnit::updateCoreQueues()
{

    //  TODO: need to update, this needs to be called every cycle


    // if main queue is not empty, take first packet and add to corresponding core queue
    // also, need to check if core queues are full

    // TODO: loads can skip the per core queues

    // update core queues first - move previous packets to per core queues
    if (!mainQueue.empty()) {
        
        PacketPtr pkt = std::get<0>(mainQueue.front());
        int senderCoreID = std::get<1>(mainQueue.front());

        if (coreQueues[senderCoreID].size() < maxCoreQueueSize) {
            totalCoreQueueSize++;
            DPRINTF(CC_BankedCache, "Adding packet to core queue %d\n", senderCoreID);
            coreQueues[senderCoreID].push_back(mainQueue.front());
            mainQueue.pop_front();
        }
    }
}

PacketPtr CC_BankUnit::removePacket()
{

    // TODO:  this needs to be called every cycle, and send things to cc_banked_cache, not just called from cc_banked_cache

    
    // TODO: make this algorithm smarter/fairer
    // TODO: deal with loads seperately - they can skip the per core queues
    
    // Print size of each core queue
    for (int i = 0; i < numCores; i++) {
        DPRINTF(CC_BankedCache, "Core %d queue size: %lu\n", i, coreQueues[i].size());
    }

    // loop over all core queues, and remove the first ready packet

    // randomly choose a core between 0 and numCores - 1 
    // int coreToCheck = rand() % numCores;
    int coreToCheck = 0;

    // loop over all numCores core queues, starting from coreToCheck
    for (int i = 0; i < numCores; i++) {
        int currentCore = (coreToCheck + i) % numCores;
        
        if (!coreQueues[currentCore].empty()) {
            PacketPtr pkt = std::get<0>(coreQueues[currentCore].front());
            coreQueues[currentCore].pop_front();
            totalCoreQueueSize--;
            return pkt;
        }
    }

    DPRINTF(CC_BankedCache, "No packet found in any core queue\n");

    return nullptr;
}

bool CC_BankUnit::isEmpty() const
{
    // Check main queue first
    // if (!mainQueue.empty()) {
    //     return false;
    // }
    
    // Check all core queues
    // for (int i = 0; i < numCores; i++) {
    //     if (!coreQueues[i].empty()) {
    //         return false;
    //     }
    // }
    
    if (totalCoreQueueSize == 0) {
        return true;
    } else {
        return false;
    }
}

size_t CC_BankUnit::getQueueSize() const
{
    return mainQueue.size();
}

size_t CC_BankUnit::getMaxQueueSize() const
{
    return maxMainQueueSize;
}

} // namespace gem5
