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
    

    // DPRINTF(CC_BankedCache, "Updating core queues\n");
    updateCoreQueues(); //this adds packets from main queue to core queues

    // DPRINTF(CC_BankedCache, "Retiring from core queues\n");
    retireFromCoreQueue(); // this checks if any packets are ready to be retired, and retires them

    // DPRINTF(CC_BankedCache, "Done clock update\n");
    // remove should be done here too in future.
}

bool CC_BankUnit::addPacket(PacketPtr pkt, int senderCoreID)
{

    if (mainQueue.size() >= maxMainQueueSize) {
        DPRINTF(CC_BankedCache, "Main queue is full; cannot add the packet\n");
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
                // this is false because we don't ever want a load to go to the per-core queues
                isReady = false; 
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

        mainQueue.push_back(std::make_tuple(pkt, senderCoreID, uniqueInstSeqNum, isReady, false)); //dispatchLoad always false for now

        // print main queue
        DPRINTF(CC_BankedCache, "Main queue: \n");
        for (auto &packet : mainQueue) {
            DPRINTF(CC_BankedCache, "Packet: %s\n", std::get<0>(packet)->print());
        }
        
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


        PacketCoreTuple& currentMemInst = mainQueue.front();
        
        PacketPtr pkt = std::get<0>(currentMemInst);
        int senderCoreID = std::get<1>(currentMemInst);
        // bool isReady = std::get<2>(mainQueue.front());
        uint64_t uniqueInstSeqNum = std::get<2>(currentMemInst);
        // bool isReady = std::get<3>(currentMemInst);
        bool dispatchLoad = std::get<4>(currentMemInst);

        // store address of packet
        Addr pktAddr = pkt->getAddr();

        CC_PacketState *cc_packet_state = dynamic_cast<CC_PacketState *>(pkt->senderState);

        if (cc_packet_state->storeType == StoreType::complete) {
            DPRINTF(CC_BankedCache, "ST::complete in core queue %d\n", senderCoreID);
            
            // get unique instruction ID from packet
            // uint64_t uniqueInstSeqNum = cc_packet_state->uniqueInstSeqNum;
            
            // int temp_counter = 0;
            // check if uniqueInstSeqNum is in the relevant core queue
            for (auto &packet : coreQueues[senderCoreID]) { // loop over all packets in the core queue
                if (std::get<2>(packet) == uniqueInstSeqNum) { // if coreQueue[senderCoreID][i].uniqueInstSeqNum == uniqueInstSeqNum
                    std::get<3>(packet) = true; // set isReady to true
                }
                // std::get<3>(packet) = true; //TODO: only for now
                
            }

            mainQueue.pop_front();
        // if store type is non-store, then continue for now (no need to add to core queue)
        } else if (cc_packet_state->storeType == StoreType::commit) {
            if (coreQueues[senderCoreID].size() < maxCoreQueueSize)
            {
                totalCoreQueueSize++;
                DPRINTF(CC_BankedCache, "ST::commit in core queue %d\n", senderCoreID);
                coreQueues[senderCoreID].push_back(mainQueue.front());
                mainQueue.pop_front();
            }
            
        } else if (cc_packet_state->storeType == StoreType::non_store) {

            bool canBypassLoad = false;

            // go over every packet in the core queue and send packet if found correct val.

            for (auto &packet : coreQueues[senderCoreID]) { // loop over all packets in the core queue
                    if (std::get<0>(packet)->getAddr() == pktAddr) {  
                        // if address is the same then dispatch from here
                        // TODO: this is doing a cache access, need to change cc_dispatchFromCoreQueue to do instant returns with less latency
                        DPRINTF(CC_BankedCache, "LD::non_store found packet in core queue %d\n", senderCoreID);
                        // mainQueue.pop_front();
                        // parentCache->cc_dispatchFromCoreQueue(pkt);
                        // totalCoreQueueSize++;
                        // std::get<3>(currentMemInst) = true; // set isReady to true
                        // coreQueues[senderCoreID].push_back(mainQueue.front());
                        // mainQueue.pop_front();
                        canBypassLoad = true;
                    }
            }

            if (canBypassLoad == false) {
                // 
                // TEMP: this is a temp working soln
                totalCoreQueueSize++;
                DPRINTF(CC_BankedCache, "LD::non_store in core queue (bypassed) %d\n", senderCoreID);
                // std::get<3>(mainQueue.front()) = true; // set isReady to true
                std::get<3>(currentMemInst) = true; // set isReady to true
                // coreQueues[senderCoreID].push_back(mainQueue.front());
                parentCache->cc_dispatchFromCoreQueue(pkt, false); //isLoadBypassed is false
                mainQueue.pop_front();
            } else {
                totalCoreQueueSize++;
                DPRINTF(CC_BankedCache, "LD::non_store in cache (no bypass) %d\n", senderCoreID);
                // std::get<3>(mainQueue.front()) = true; // set isReady to true
                std::get<3>(currentMemInst) = true; // set isReady to true
                // coreQueues[senderCoreID].push_back(mainQueue.front());
                parentCache->cc_dispatchFromCoreQueue(pkt, true); // isLoadBypassed is true
                mainQueue.pop_front();
            }


        }

        // print all main and core queues
        // DPRINTF(CC_BankedCache, "Main queue: \n");
        // for (auto &packet : mainQueue) {
        //     // print seqNum
        //     DPRINTF(CC_BankedCache, "mainQ SeqNum: %llu\n", std::get<2>(packet));
        // }

        // DPRINTF(CC_BankedCache, "Core queues: \n");
        // for (int i = 0; i < numCores; i++) {
        //     DPRINTF(CC_BankedCache, "Core queue %d: \n", i);
        //     for (auto &packet : coreQueues[i]) {
        //         DPRINTF(CC_BankedCache, "coreQ SeqNum: %llu\n", std::get<2>(packet));
        //     }
        // }
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

    PacketPtr currentPkt;
    bool currentInstIsReady = false;
    int currentCore;

    // loop over all numCores core queues, starting from coreToCheck
    for (int i = 0; i < numCores; i++) {
        currentCore = (coreToCheck + i) % numCores;
        
        if (!coreQueues[currentCore].empty()) {
            // DPRINTF(CC_BankedCache, "Checking core queue %d\n", currentCore);
            // DPRINTF(CC_BankedCache, "int i: %d\n", i);
            currentPkt = std::get<0>(coreQueues[currentCore].front());

            // check if packet is ready to be retired
            currentInstIsReady = std::get<3>(coreQueues[currentCore].front());
            // bool isReady = true;

            if (currentInstIsReady) {
                DPRINTF(CC_BankedCache, "Retiring packet from core queue %d, size: %d\n", currentCore, coreQueues[currentCore].size());                
                // print current packet seqNum
                DPRINTF(CC_BankedCache, "Current packet seqNum: %llu\n", std::get<2>(coreQueues[currentCore].front()));
                // print type of current packet (storeType)
                CC_PacketState *cc_packet_state = dynamic_cast<CC_PacketState *>(currentPkt->senderState);
                if (cc_packet_state->storeType == StoreType::commit) {
                    DPRINTF(CC_BankedCache, "Current packet is commit\n");
                } else if (cc_packet_state->storeType == StoreType::complete) {
                    DPRINTF(CC_BankedCache, "Current packet is complete\n");
                } else if (cc_packet_state->storeType == StoreType::non_store) {
                    DPRINTF(CC_BankedCache, "Current packet is non-store\n");
                }
                coreQueues[currentCore].pop_front();
                totalCoreQueueSize--;
                parentCache->cc_dispatchFromCoreQueue(currentPkt);
                return;
            }
        }
    }

    return;

    // DPRINTF(CC_BankedCache, "No packet found in any core queue\n");

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
