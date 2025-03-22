// cc_bank_unit.hh

#ifndef __CC_BANK_UNIT_HH__
#define __CC_BANK_UNIT_HH__

#include <deque>
#include "mem/packet.hh"
#include "base/types.hh"
#include "base/statistics.hh" //for custom stats

#include "sim/clocked_object.hh"
#include <array>

namespace gem5 {

class CC_BankUnit
{
  private:
    // static const size_t maxQueueSize = 16; // TODO change
    // std::deque<PacketPtr> packetQueue;

    // one main queue
    // num_cores queues for each core.

    static const size_t numCores = 8;
    static const size_t maxMainQueueSize = 16;
    static const size_t maxCoreQueueSize = 8;

    std::deque<PacketPtr> mainQueue;
    std::array<std::deque<PacketPtr>, numCores> coreQueues;

  public:
    CC_BankUnit();
    ~CC_BankUnit();

    bool addPacket(PacketPtr pkt);
    PacketPtr removePacket();

    bool isEmpty() const;
    size_t getQueueSize() const; // Function to get current queue size

    size_t getMaxQueueSize() const;

};

} // namespace gem5

#endif // __CC_BANK_UNIT_HH__
