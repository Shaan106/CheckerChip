#ifndef CC_PACKETSTATE_HH
#define CC_PACKETSTATE_HH

#include "mem/packet.hh"

// Custom state definition
class CC_PacketState : public gem5::Packet::SenderState {
public:
    int customInfo;      // Example field
    std::string tag;     // Example field

    CC_PacketState(int info, const std::string &tagValue)
        : customInfo(info), tag(tagValue) {}
};

#endif // CC_PACKETSTATE_HH
