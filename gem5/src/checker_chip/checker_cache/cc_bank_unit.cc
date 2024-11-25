// cc_bank_unit.cc

#include "checker_chip/checker_cache/cc_bank_unit.hh"

namespace gem5 {

CC_BankUnit::CC_BankUnit()
{
    // Constructor
}

CC_BankUnit::~CC_BankUnit()
{
    // Destructor
}

bool CC_BankUnit::addPacket(PacketPtr pkt)
{
    if (packetQueue.size() >= maxQueueSize) {
        // Queue is full; cannot add the packet
        return false;
    } else {
        packetQueue.push_back(pkt);
        return true;
    }
}

PacketPtr CC_BankUnit::removePacket()
{
    if (packetQueue.empty()) {
        return nullptr;
    }
    PacketPtr pkt = packetQueue.front();
    packetQueue.pop_front();
    return pkt;
}

bool CC_BankUnit::isEmpty() const
{
    return packetQueue.empty();
}

size_t CC_BankUnit::getQueueSize() const
{
    return packetQueue.size();
}

// Removed the printUnit() function and DPRINTF statements

} // namespace gem5
