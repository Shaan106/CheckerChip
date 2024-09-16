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


namespace gem5
{

// class CC_Buffer : public SimObject // <--------- change to inherit from "ClockedObject"
class CC_Buffer : public ClockedObject
{
  private:
    /**
     * Fill the buffer with the next chunk of data
     */
    void processEvent();

    /// An event that wraps the above function
    EventFunctionWrapper event;

    /**
     * Fills the buffer for one iteration. If the buffer isn't full, this
     * function will enqueue another event to continue filling.
     */
    void linkedFunc();

    // Function that creates and returns a CheckerInst object
    CheckerInst instantiateObject(const gem5::o3::DynInstPtr &instName);

    /// A deque to hold the stack of strings (buffer)
    // std::deque<std::string> buffer;
    // std::deque<StaticInstPtr> buffer;
    std::deque<CheckerInst> buffer;

    /// The maximum size of the buffer
    uint maxCredits;

    // current number of credits
    uint currentCredits;

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
    
};

} // namespace gem5

#endif // __CC_BUFFER_HH__
