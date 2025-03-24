// cc_bank_unit.hh

#ifndef __CC_BANK_UNIT_HH__
#define __CC_BANK_UNIT_HH__

#include <deque>
#include <tuple>
#include "mem/packet.hh"
#include "base/types.hh"
#include "base/statistics.hh" //for custom stats
#include "checker_chip/checker_cache/checker_packet_state.hh" // for CC_PacketState

#include "sim/clocked_object.hh"
#include <array>

namespace gem5 {

class CC_BankedCache; // Forward declaration

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

    // total number of packets in core queues (sum of all core queues)
    int totalCoreQueueSize = 0;

    // Define type for packet and core ID tuple
    using PacketCoreTuple = std::tuple<PacketPtr, int>;

    std::deque<PacketCoreTuple> mainQueue;
    std::array<std::deque<PacketCoreTuple>, numCores> coreQueues;

    // Pointer to parent cache
    CC_BankedCache* parentCache;
    unsigned bankId;

  public:
    CC_BankUnit();
    ~CC_BankUnit();
    
    // Set the parent cache and bank ID
    void setParentCache(CC_BankedCache* cache, unsigned id) {
        parentCache = cache;
        bankId = id;
    }

    // called by cc_banked_cache every checker clock cycle
    void clock_update();
    
    bool addPacket(PacketPtr pkt, int senderCoreID); // add packet to main queue
    void updateCoreQueues(); // add packet to corresponding core queue
    PacketPtr removePacket();

    bool isEmpty() const;
    size_t getQueueSize() const; // Function to get current queue size

    size_t getMaxQueueSize() const;
};

} // namespace gem5

#endif // __CC_BANK_UNIT_HH__
