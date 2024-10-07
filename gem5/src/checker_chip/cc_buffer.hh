#ifndef __CC_BUFFER_HH__
#define __CC_BUFFER_HH__

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

    // instance of a functional unit
    FuncUnit funcUnit;
    uint num_functional_units;
    uint num_functional_units_free;
    void initializeFuncUnit(FuncUnit &funcUnit); // this is to set the initial "capabilities"

    //define functional_unit_pool here
    gem5::o3::FUPool *functional_unit_pool;  // Pointer to FUPool instance, from CC_Buffer.py

    // testing 

    unsigned long instCount;

  public:
    CC_Buffer(const CC_BufferParams &p);
    ~CC_Buffer(); /// Destructor

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

    // Function to get latency for an operation
    int getOperationLatency(OpClass op_class);
    
};

} // namespace gem5

#endif // __CC_BUFFER_HH__
