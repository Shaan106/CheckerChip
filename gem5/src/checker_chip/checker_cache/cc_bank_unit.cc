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
    if (mainQueue.size() >= maxMainQueueSize) {
        // Queue is full; cannot add the packet
        return false;
    } else {
        mainQueue.push_back(pkt);
        return true;
    }
}

PacketPtr CC_BankUnit::removePacket()
{
    if (mainQueue.empty()) {
        return nullptr;
    }
    PacketPtr pkt = mainQueue.front();
    mainQueue.pop_front();
    return pkt;
}

bool CC_BankUnit::isEmpty() const
{
    return mainQueue.empty();
}

size_t CC_BankUnit::getQueueSize() const
{
    return mainQueue.size();
}

size_t CC_BankUnit::getMaxQueueSize() const
{
    return maxMainQueueSize;
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
