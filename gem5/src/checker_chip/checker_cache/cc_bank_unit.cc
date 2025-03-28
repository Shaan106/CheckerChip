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
    // this is called every checker clock cycle from cc_banked_cache
    

    updateCoreQueues(); //this adds packets from main queue to core queues

    
    if (parentCache) {
        retireFromCoreQueue(); // this checks if any packets are ready to be retired, and retires them
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

        bool isReady = true; // true for now, but will need to be changed based on LD/ST commit/ST complete

        mainQueue.push_back(std::make_tuple(pkt, senderCoreID, isReady));
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
        bool isReady = std::get<2>(mainQueue.front());
        if (coreQueues[senderCoreID].size() < maxCoreQueueSize) {
            totalCoreQueueSize++;
            DPRINTF(CC_BankedCache, "Adding packet to core queue %d\n", senderCoreID);
            coreQueues[senderCoreID].push_back(mainQueue.front());
            mainQueue.pop_front();
        }
    }
}

void CC_BankUnit::retireFromCoreQueue() {

    // randomly choose a core between 0 and numCores - 1 
    int coreToCheck = rand() % numCores;
    // int coreToCheck = 0;

    // loop over all numCores core queues, starting from coreToCheck
    for (int i = 0; i < numCores; i++) {
        int currentCore = (coreToCheck + i) % numCores;
        
        if (!coreQueues[currentCore].empty()) {
            PacketPtr pkt = std::get<0>(coreQueues[currentCore].front());\

            // check if packet is ready to be retired
            bool isReady = std::get<2>(coreQueues[currentCore].front());

            if (isReady) {
                coreQueues[currentCore].pop_front();
                totalCoreQueueSize--;
                parentCache->cc_dispatchFromCoreQueue(pkt);
                return;
            }
        }
    }

    // DPRINTF(CC_BankedCache, "No packet found in any core queue\n");s

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

size_t CC_BankUnit::getMainQueueSize() const
{
    return mainQueue.size();
}

size_t CC_BankUnit::getMaxQueueSize() const
{
    return maxMainQueueSize;
}

} // namespace gem5
