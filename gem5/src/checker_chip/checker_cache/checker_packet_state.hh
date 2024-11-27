#ifndef CC_PACKETSTATE_HH
#define CC_PACKETSTATE_HH

#include "mem/packet.hh"
#include <cstdint>


// Custom state definition
class CC_PacketState : public gem5::Packet::SenderState {
public:
    int senderCoreID;
    uint64_t uniqueInstSeqNum; // unique identifier for inst to later be marked as verified
    int customInfo;      
    std::string tag;     

    CC_PacketState(int senderCoreID,
                   uint64_t uniqueInstSeqNum,
                   int info, 
                   const std::string &tagValue)
        : senderCoreID(senderCoreID),
          uniqueInstSeqNum(uniqueInstSeqNum),
          customInfo(info), 
          tag(tagValue) {}
};

#endif // CC_PACKETSTATE_HH
