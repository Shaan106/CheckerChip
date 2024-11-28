// cc_banked_cache.hh

#ifndef __CC_MEM_CACHE_BANKED_CACHE_HH__
#define __CC_MEM_CACHE_BANKED_CACHE_HH__

#include "mem/cache/cache.hh"
#include "params/CC_BankedCache.hh"
#include "sim/port.hh"
#include "base/logging.hh" // For DPRINTF
#include "debug/CC_BankedCache.hh"
#include "base/statistics.hh" //for custom stats

#include "checker_chip/checker_cache/cc_bank_unit.hh"

#include "mem/packet.hh" // For PacketPtr and PacketList
#include "base/types.hh" // For Cycles
#include <list>          // For PacketList handling
#include <string>        // For string operations, if needed

namespace gem5
{

class CC_BankedCache : public Cache
{
  public:
    // Constructor
    CC_BankedCache(const CC_BankedCacheParams &p);

    // Destructor
    ~CC_BankedCache() = default;

    // Override the getPort method
    Port &getPort(const std::string &if_name, PortID idx = InvalidPortID) override;

  // private:
    // EventFunctionWrapper freeBankClockEvent;

    // Override the regStats method
    void regStats() override;

  protected:
    // Number of banks
    unsigned numBanks;

    // checker chip additional structures around banks
    std::vector<CC_BankUnit> bankUnits;

    // stats
    // std::vector<statistics::Distribution> bank_queue_occupancy_histogram_arr;
    // statistics::Distribution bank_queue_occupancy_histogram;

    std::vector<statistics::Distribution*> bank_queue_occupancy_histogram_arr;
    // size_t numBanks;  // Number of banks (if needed)

    // bankFreeList
    std::vector<bool> bankFreeList;

    //cc_cache_controller - takes in packets and puts them into
    // the appropriate bank queue
    bool cc_cacheController(PacketPtr pkt);

    //free a certain bank after some set delay
    void cc_dispatchEvent(); 

    //free a certain bank after some set delay
    void freeBank(unsigned bankID); 

    // Bank selection function
    unsigned calculateBankId(Addr addr);

    // Override the access() method
    bool access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,
                PacketList &writebacks) override;

    // custom recvTimingReq implementation
    void recvTimingReq(PacketPtr pkt) override;

    // override handling timing reqs (HIT)
    void handleTimingReqHit(PacketPtr pkt, CacheBlk *blk,
                        Tick request_time) override;
    
    // override handling timing reqs (MISS)
    void handleTimingReqMiss(PacketPtr pkt, CacheBlk *blk,
                          Tick forward_time,
                          Tick request_time) override;

  public:
    /**
     * Port on the CPU-side that receives requests.
     * Mostly just forwards requests to the cache (owner)
     */
    class CC_CPUSidePort : public ResponsePort
    {
      private:
        /// Since this is a vector port, need to know what number this one is
        int id;

        /// The object that owns this object (CC_BankedCache)
        CC_BankedCache *owner;

        /// True if the port needs to send a retry req.
        bool needRetry;

        /// If we tried to send a packet and it was blocked, store it here
        PacketPtr blockedPacket;

      public:
        /**
         * Constructor. Just calls the superclass constructor.
         */
        CC_CPUSidePort(const std::string& name, int id, CC_BankedCache *owner) :
            ResponsePort(name, owner), id(id), owner(owner), needRetry(false),
            blockedPacket(nullptr)
        { }

        /**
         * Send a packet across this port. This is called by the owner and
         * all of the flow control is handled in this function.
         * This is a convenience function for the CC_BankedCache to send pkts.
         *
         * @param packet to send.
         */
        void sendPacket(PacketPtr pkt);

        /**
         * Get a list of the non-overlapping address ranges the owner is
         * responsible for. All response ports must override this function
         * and return a populated list with at least one item.
         *
         * @return a list of ranges responded to
         */
        AddrRangeList getAddrRanges() const override;

        /**
         * Send a retry to the peer port only if it is needed. This is called
         * from the CC_BankedCache whenever it is unblocked.
         */
        void trySendRetry();

        /**
         * Create and send a dummy response packet after a specified latency.
         * This is called when a timing request requires a response.
         *
         * @param pkt The original request packet
         */
        void createAndSendDummyResponse(PacketPtr pkt);

        // returns the cache's block size
        unsigned getCacheBlockSize();

      protected:
        /**
         * Receive an atomic request packet from the request port.
         * No need to implement in this simple cache.
         */
        Tick recvAtomic(PacketPtr pkt) override
        { panic("recvAtomic unimpl."); }

        /**
         * Receive a functional request packet from the request port.
         * Performs a "debug" access updating/reading the data in place.
         *
         * @param packet the requestor sent.
         */
        void recvFunctional(PacketPtr pkt) override;

        /**
         * Receive a timing request from the request port.
         *
         * @param the packet that the requestor sent
         * @return whether this object can consume to packet. If false, we
         *         will call sendRetry() when we can try to receive this
         *         request again.
         */
        bool recvTimingReq(PacketPtr pkt) override;

        /**
         * Called by the request port if sendTimingResp was called on this
         * response port (causing recvTimingResp to be called on the request
         * port) and was unsuccessful.
         */
        void recvRespRetry() override;
    };
  
  protected:
    // CC_CPUSidePort cc_cpu_port;
    std::vector<CC_CPUSidePort> cc_cpu_port;

    
};

} // namespace gem5

#endif // __CC_MEM_CACHE_BANKED_CACHE_HH__
