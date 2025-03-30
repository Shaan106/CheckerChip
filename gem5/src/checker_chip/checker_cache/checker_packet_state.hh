#ifndef CC_PACKETSTATE_HH
#define CC_PACKETSTATE_HH

#include "mem/packet.hh"
#include <cstdint>

// Enum to track store operation types
enum class StoreType {
    commit,
    complete,
    non_store
};

// Custom state definition
class CC_PacketState : public gem5::Packet::SenderState {
public:
    int senderCoreID;
    uint64_t uniqueInstSeqNum; // unique identifier for inst to later be marked as verified
    int customInfo;      
    std::string tag;     
    StoreType storeType;     

    CC_PacketState(int senderCoreID,
                   uint64_t uniqueInstSeqNum,
                   int info, 
                   const std::string &tagValue,
                   StoreType type = StoreType::non_store)
        : senderCoreID(senderCoreID),
          uniqueInstSeqNum(uniqueInstSeqNum),
          customInfo(info), 
          tag(tagValue),
          storeType(type) {}
};

#endif // CC_PACKETSTATE_HH
