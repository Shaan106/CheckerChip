#ifndef __CC_BUFFER_HH__
#define __CC_BUFFER_HH__

// #include <vector>
#include <unordered_map>
#include <string>
#include <deque>  // Include deque for buffer storage

#include "params/CC_Buffer.hh"
// #include "sim/sim_object.hh" // <--------- change to include "sim/clocked_object.hh"
#include "sim/clocked_object.hh"

// #include "cpu/static_inst.hh"
#include "cpu/o3/dyn_inst_ptr.hh"
#include "checker_chip/cc_inst.hh"

#include "cpu/func_unit.hh" // this is for the func unit and doing the opClass -> latency conversion

#include "checker_chip/cc_creditSystem.hh" // this is for the new credit latency system

#include "cpu/o3/fu_pool.hh" // accessing FUPool

#include "checker_chip/cc_regfile.hh" // for custom regfile

#include "base/statistics.hh" //for custom stats

#include "checker_chip/cc_tlb.hh"

#include "sim/port.hh" //for ports for sending packets to cache
#include "mem/cache/cache.hh"

#include "mem/request.hh"
#include "sim/system.hh"       // Include for System pointer
#include "mem/request.hh" // For RequestorID and InvalidRequestorId

#include "cpu/op_class.hh"

#include "checker_chip/checker_cache/cc_banked_cache.hh"

namespace gem5
{

// class CC_Buffer : public SimObject // <--------- change to inherit from "ClockedObject"
class CC_Buffer : public ClockedObject
{
  private:    
    /*
      this is an event that will engage the buffer's clock to take the next cycle
      processBufferClockEvent() does the actual things, bufferClockEvent is the event
    */
    EventFunctionWrapper bufferClockEvent;

    void processBufferClockEvent();

    // decode_buffer update
    // when decode ready push to execute_buffer
    void updateDecodeBufferContents();

    // execute_buffer update
    // when execute ready, remove from pipe and "execute"
    void updateExecuteBufferContents();

    // Function that creates and returns a CheckerInst object
    CheckerInst instantiateObject(const gem5::o3::DynInstPtr &instName);

    /// The maximum size of the buffer
    /// obsolete now, here to demonstrate passing vals from Python side to c++
    uint max_credits;

    // decode buffer modelling parameters
    std::deque<CheckerInst> decode_buffer; //actual buffer
    uint decode_buffer_bandwidth; //decode bandwidth
    uint decode_buffer_latency; // decode latency, replaces num_cycles_to_decode

    // execute buffer modelling parameters
    std::deque<CheckerInst> execute_buffer;

    // new credit system
    CheckerCreditSystem decode_buffer_credits; // Declare without parameters
    CheckerCreditSystem execute_buffer_credits; // Declare without parameters

    // cc_buffer_clock tracks how many cc_buffer clock cycles have occured
    unsigned long cc_buffer_clock;

    // cc_buffer_clock_period is how many ticks per cc_buffer_clock cycle
    unsigned long cc_buffer_clock_period;

    //define functional_unit_pool here
    gem5::o3::FUPool *functional_unit_pool;  // Pointer to functional unit pool instance, from CC_Buffer.py

    // custom regfile

    CheckerRegfile checker_regfile;

    // testing/debug
    unsigned long instCount;
    std::unordered_map<std::string, int> debugStringMap;

    // TLB instance
    CheckerTLB tlb;

    System *system; // Add a pointer to the System
    RequestorID requestorId;

  protected:

    /**
     * Port on the memory-side that receives responses.
     * Mostly just forwards requests to the cache (owner)
     */
    class CC_MemSidePort : public RequestPort
    {
      private:
        /// The object that owns this object (CC_SimpleCache)
        CC_Buffer *owner;

        /// If we tried to send a packet and it was blocked, store it here
        PacketPtr blockedPacket;

      public:
        /**
         * Constructor. Just calls the superclass constructor.
         */
        CC_MemSidePort(const std::string& name, CC_Buffer *owner) :
            RequestPort(name), owner(owner), blockedPacket(nullptr)
        { }

        /**
         * Send a packet across this port. This is called by the owner and
         * all of the flow control is hanled in this function.
         * This is a convenience function for the CC_SimpleCache to send pkts.
         *
         * @param packet to send.
         */
        void sendPacket(PacketPtr pkt);

        // returns cacheBlockSize
        unsigned getCacheBlockSize();

      protected:
        /**
         * Receive a timing response from the response port.
         */
        bool recvTimingResp(PacketPtr pkt) override;

        /**
         * Called by the response port if sendTimingReq was called on this
         * request port (causing recvTimingReq to be called on the response
         * port) and was unsuccesful.
         */
        void recvReqRetry() override;

        /**
         * Called to receive an address range change from the peer response
         * port. The default implementation ignores the change and does
         * nothing. Override this function in a derived class if the owner
         * needs to be aware of the address ranges, e.g. in an
         * interconnect component like a bus.
         */
        void recvRangeChange() override;
    };

    CC_MemSidePort cc_mem_side_port;

  public:
    CC_Buffer(const CC_BufferParams &p);
    ~CC_Buffer(); /// Destructor

    // Override the init method
    void init() override;

    // Implement getPort method
    Port &getPort(const std::string &if_name, PortID idx = InvalidPortID) override;

    /**
     * Method to create and send a dummy packet through cc_mem_side_port.
     */
    void sendDummyPacket();

    /**
     * Method to create and send a readReq packet through cc_mem_side_port.
     */
    // void sendReadReqPacket(CheckerInst memInst);
    void sendReadReqPacket(CheckerInst memInst);

    /**
     * Method to create and send a readWrite packet through cc_mem_side_port.
     */
    // void sendWriteReqPacket(CheckerInst memInst);
    void sendWriteReqPacket(CheckerInst memInst);

    /**
     * Called by an outside object. Starts off the events to fill the buffer
     * with a goodbye message.
     *
     * @param instName the name of the instruction to be added to the buffer.
     */
    // void pushCommit(const std::string &instName); gem5::RefCountingPtr<gem5::o3::DynInst>
    // void pushCommit(const StaticInstPtr &instName);
    void pushCommit(const gem5::o3::DynInstPtr &instName);

    /**
     * Returns the number of credits in the buffer.
    */
    uint getNumCredits();

    /*
    stats to measure
    */
    // Override the regStats method
    void regStats() override;

    statistics::Scalar cc_buffer_cycles;
    statistics::Scalar ooo_stall_signals;
    void addStallCycle();


    statistics::Scalar regfile_insts_processed;

    // statistics::Scalar decode_buffer_occupancy_total;
    // statistics::Formula decode_buffer_occupancy_avg;

    // statistics::Scalar decode_buffer_occupancy_maximum;

    statistics::Distribution decode_buffer_occupancy_histogram;

    statistics::Distribution execute_buffer_occupancy_histogram;

    
};

} // namespace gem5

#endif // __CC_BUFFER_HH__
