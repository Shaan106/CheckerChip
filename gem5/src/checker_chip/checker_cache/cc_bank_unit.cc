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

size_t CC_BankUnit::getMaxQueueSize() const
{
    return maxQueueSize;
}

// void CC_BankUnit::regStats() 
// {
//     bank_queue_occupancy_histogram
//             .init(0, maxQueueSize, 1)  // Initialize with min, max, and step
//             .name(name() + ".cc_buffer_test_bank_test")  // Set the dynamic name
//             .desc("Distribution of bank queue")
//             .flags(statistics::pdf | statistics::display);  // Set flags
// }

} // namespace gem5
