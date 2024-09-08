
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CC_Buffer_Flag.hh"

#include "sim/sim_exit.hh"

#include <iostream>

#include <deque> //new

namespace gem5
{

/*
Constructor for the GoodbyeObject. 
*/
CC_Buffer::CC_Buffer(const CC_BufferParams &params) :
    SimObject(params), 
    event([this]{ processEvent(); }, name() + ".event")
{
    DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");

    // Initialize the buffer
    buffer = std::deque<std::string>();  // Initialize an empty deque for now
}

CC_Buffer::~CC_Buffer()
{
    // DPRINTF(CC_Buffer_Flag, "Destructor called\n");
}

void
CC_Buffer::processEvent()
{
    std::cout << "hi again..." << std::endl;

}

void
CC_Buffer::pushCommit(const std::string &instName)
{
    // std::cout << "hi" << std::endl;


    DPRINTF(CC_Buffer_Flag, "Debug statement??\n");

    DPRINTF(CC_Buffer_Flag, "instName: %s\n", instName.c_str());

    // Add the string to the buffer
    buffer.push_back(instName);

    // Ensure the buffer size does not exceed 20
    const int maxBufferSize = 20;
    if (buffer.size() > maxBufferSize) {
        buffer.pop_front();  // Remove the oldest entry to keep the buffer size at 20
    }

    //print buffer contents for debug
    std::string bufferContents = "[";
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        bufferContents += *it;
        if (std::next(it) != buffer.end()) {
            bufferContents += ", ";
        }
    }
    bufferContents += "]";

    // Output the buffer contents in one line
    DPRINTF(CC_Buffer_Flag, "Current buffer contents: %s\n", bufferContents.c_str());

    linkedFunc();
}

void 
CC_Buffer::linkedFunc()
{
    // std::cout << "linkedFunc called" << std::endl;
    DPRINTF(CC_Buffer_Flag, "debug statement from linked func\n");
}


} // namespace gem5
