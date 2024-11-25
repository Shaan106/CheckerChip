// cc_bank_unit.hh

#ifndef __CC_BANK_UNIT_HH__
#define __CC_BANK_UNIT_HH__

#include <deque>
#include "mem/packet.hh"
#include "base/types.hh"

namespace gem5 {

class CC_BankUnit
{
  private:
    static const size_t maxQueueSize = 16;
    std::deque<PacketPtr> packetQueue;

  public:
    CC_BankUnit();
    ~CC_BankUnit();

    bool addPacket(PacketPtr pkt);
    PacketPtr removePacket();

    bool isEmpty() const;
    size_t getQueueSize() const; // Function to get current queue size
};

} // namespace gem5

#endif // __CC_BANK_UNIT_HH__
