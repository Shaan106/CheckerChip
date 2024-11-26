#ifndef CC_PACKETSTATE_HH
#define CC_PACKETSTATE_HH

#include "mem/packet.hh"

// Custom state definition
class CC_PacketState : public gem5::Packet::SenderState {
public:
    int senderCoreID;
    int customInfo;      
    std::string tag;     

    CC_PacketState(int senderCoreID,
                   int info, 
                   const std::string &tagValue)
        : senderCoreID(senderCoreID),
          customInfo(info), 
          tag(tagValue) {}
};

#endif // CC_PACKETSTATE_HH
