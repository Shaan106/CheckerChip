#ifndef __CC_BUFFER_HH__
#define __CC_BUFFER_HH__

#include <string>
#include <deque>  // Include deque for buffer storage

#include "params/CC_Buffer.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class CC_Buffer : public SimObject
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

    /// A deque to hold the stack of strings (buffer)
    std::deque<std::string> buffer;

    /// The maximum size of the buffer
    static const int maxBufferSize = 20;

  public:
    CC_Buffer(const CC_BufferParams &p);
    ~CC_Buffer(); /// Destructor

    /**
     * Called by an outside object. Starts off the events to fill the buffer
     * with a goodbye message.
     *
     * @param instName the name of the instruction to be added to the buffer.
     */
    void pushCommit(const std::string &instName);
    
};

} // namespace gem5

#endif // __CC_BUFFER_HH__
