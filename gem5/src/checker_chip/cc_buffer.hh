
#ifndef __CC_BUFFER_HH__
#define __CC_BUFFER_HH__

#include <string>

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
    // void fillBuffer();

    /// The bytes processed per tick
    // float bandwidth;

    /// The size of the buffer we are going to fill
    // int bufferSize;

    /// The buffer we are putting our message in
    // char *buffer;

    /// The message to put into the buffer.
    // std::string message;

    /// The amount of the buffer we've used so far.
    // int bufferUsed;

  public:
    CC_Buffer(const CC_BufferParams &p);
    ~CC_Buffer(); /// this is a destructor

    /**
     * Called by an outside object. Starts off the events to fill the buffer
     * with a goodbye message.
     *
     * @param name the name of the object we are saying goodbye to.
     */
     void pushCommit(const std::string &instName);
    // void pushCommit(std::string name);
    
};

} // namespace gem5

#endif // __CC_BUFFER_HH__
