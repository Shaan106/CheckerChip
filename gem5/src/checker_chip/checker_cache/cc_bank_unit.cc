// cc_bank_unit.cc

#include "checker_chip/checker_cache/cc_bank_unit.hh"
#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "debug/CC_BankedCache.hh"
#include "checker_chip/checker_cache/checker_packet_state.hh"

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

        CC_PacketState *cc_packet_state = dynamic_cast<CC_PacketState *>(pkt->senderState);

        uint64_t uniqueInstSeqNum = cc_packet_state->uniqueInstSeqNum;

        bool isReady;

        // check type of packet
        if (cc_packet_state) {
            if (cc_packet_state->storeType == StoreType::commit) {
                DPRINTF(CC_BankedCache, "CC_BankUnit: Processing commit packet, uniqueInstSeqNum: %llu\n", uniqueInstSeqNum);
                // Handle commit packet
                isReady = false;
            } else if (cc_packet_state->storeType == StoreType::complete) {
                DPRINTF(CC_BankedCache, "CC_BankUnit: Processing complete packet, uniqueInstSeqNum: %llu\n", uniqueInstSeqNum);
                // Handle complete packet
                isReady = true;
            } else if (cc_packet_state->storeType == StoreType::non_store) {
                DPRINTF(CC_BankedCache, "CC_BankUnit: Processing non-store packet, uniqueInstSeqNum: %llu\n", uniqueInstSeqNum);
                // Handle non-store packet
                isReady = true;
            } else {
                DPRINTF(CC_BankedCache, "ERROR:CC_BankUnit: Unknown packet type, uniqueInstSeqNum: %llu\n", uniqueInstSeqNum);
                // print more info about the packet
                // DPRINTF(CC_BankedCache, "Packet type: %d\n", cc_packet_state->storeType);
                DPRINTF(CC_BankedCache, "Packet: %s\n", pkt->print());

                return false;
            }
        } else {
            DPRINTF(CC_BankedCache, "ERROR: CC_BankUnit: No packet state found\n");
            return false;
        }

        mainQueue.push_back(std::make_tuple(pkt, senderCoreID, uniqueInstSeqNum, isReady));
        
        return true;
    }
}

void CC_BankUnit::updateCoreQueues()
{
    // if main queue is not empty, take first packet and add to corresponding core queue
    
    //  TODO: need to update, this needs to be called every cycle

    // TODO: loads can skip the per core queues

    // update core queues first - move previous packets to per core queues
    if (!mainQueue.empty()) {
        
        PacketPtr pkt = std::get<0>(mainQueue.front());
        int senderCoreID = std::get<1>(mainQueue.front());
        // bool isReady = std::get<2>(mainQueue.front());
        uint64_t uniqueInstSeqNum = std::get<2>(mainQueue.front());

        CC_PacketState *cc_packet_state = dynamic_cast<CC_PacketState *>(pkt->senderState);

        if (coreQueues[senderCoreID].size() < maxCoreQueueSize) {

            // if store type is commit, add to core queue
            if (cc_packet_state->storeType == StoreType::commit) {
                totalCoreQueueSize++;
                DPRINTF(CC_BankedCache, "ST::commit in core queue %d\n", senderCoreID);
                coreQueues[senderCoreID].push_back(mainQueue.front());
                mainQueue.pop_front();
            
            // if store type is complete, release packet in relevant core queue (set isReady to true)
            } else if (cc_packet_state->storeType == StoreType::complete) {
                DPRINTF(CC_BankedCache, "ST::complete in core queue %d\n", senderCoreID);
                
                // get unique instruction ID from packet
                uint64_t uniqueInstSeqNum = cc_packet_state->uniqueInstSeqNum;

                // check if uniqueInstSeqNum is in the relevant core queue
                for (auto &packet : coreQueues[senderCoreID]) { // loop over all packets in the core queue
                    if (std::get<2>(packet) == uniqueInstSeqNum) { // if coreQueue[senderCoreID][i].uniqueInstSeqNum == uniqueInstSeqNum
                        std::get<3>(packet) = true; // set isReady to true
                    }
                }

                mainQueue.pop_front();
            // if store type is non-store, then continue for now (no need to add to core queue)
            } else if (cc_packet_state->storeType == StoreType::non_store) {
                // TODO: loads don't need to be added to core queues
                totalCoreQueueSize++;
                DPRINTF(CC_BankedCache, "LD::non_store in core queue %d\n", senderCoreID);
                coreQueues[senderCoreID].push_back(mainQueue.front());
                mainQueue.pop_front();        
            }

        }
    }
}

void CC_BankUnit::retireFromCoreQueue() {

    // print all core queue sizes
    // for (int i = 0; i < numCores; i++) {
    //     DPRINTF(CC_BankedCache, "Core queue %d size: %d\n", i, coreQueues[i].size());
    // }

    // randomly choose a core between 0 and numCores - 1 
    int coreToCheck = rand() % numCores;
    // int coreToCheck = 0;

    // loop over all numCores core queues, starting from coreToCheck
    for (int i = 0; i < numCores; i++) {
        int currentCore = (coreToCheck + i) % numCores;
        
        if (!coreQueues[currentCore].empty()) {
            PacketPtr pkt = std::get<0>(coreQueues[currentCore].front());

            // check if packet is ready to be retired
            // bool isReady = std::get<3>(coreQueues[currentCore].front());
            bool isReady = true;

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
