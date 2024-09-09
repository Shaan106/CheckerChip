
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CC_Buffer_Flag.hh"

// #include "cpu/o3/dyn_inst.hh"

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
    event([this]{ processEvent(); }, name() + ".event"),
    maxCredits(params.maxCredits)
{
    DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");

    // Initialize the buffer
    buffer = std::deque<StaticInstPtr>();  // Initialize an empty deque for now

    currentCredits = maxCredits;
}

int
CC_Buffer::getNumCredits()
{
    return currentCredits;
}

CC_Buffer::~CC_Buffer()
{
    // DPRINTF(CC_Buffer_Flag, "Destructor called\n");
}

void
CC_Buffer::processEvent()
{
    // std::cout << "hi again..." << std::endl;
    DPRINTF(CC_Buffer_Flag, "Clearing buffer...\n");
    // remove 4 elements from the front of the buffer
    for (int i = 0; i < 10; i++) {
        buffer.pop_front();
    }
    // increase num credits by 4
    currentCredits += 10;
    // buffer.pop_front();  // Remove the oldest entry to keep the buffer size at 20
}

void
// CC_Buffer::pushCommit(const std::string &instName)
CC_Buffer::pushCommit(const StaticInstPtr &instName)
{
    // std::cout << "hi" << std::endl;

    DPRINTF(CC_Buffer_Flag, "Debug statement... in cc_buffer now\n");

    DPRINTF(CC_Buffer_Flag, "pushed instruction name: %s\n", instName->getName().c_str());

    // Add the string to the buffer
    buffer.push_back(instName);
    // reduce num credits
    currentCredits--;

    // Ensure the buffer size does not exceed 20
    // const int maxBufferSize = 20;
    if (buffer.size() >= maxCredits) {
        DPRINTF(CC_Buffer_Flag, "Max credits reached, scheduling buffer clear...\n");

        // schedule event to clear buffer in 50000 ticks (the buffer "clear" delay)
        if (!event.scheduled()) {
            schedule(event, curTick() + 50000);
        } else {
            DPRINTF(CC_Buffer_Flag, "Event is already scheduled, skipping re-scheduling, maybe stall?\n");
        }
    }

    //print buffer contents for debug
    std::string bufferContents = "[";
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        bufferContents += (*it)->getName();
        if (std::next(it) != buffer.end()) {
            bufferContents += ", ";
        }
    }
    bufferContents += "]";

    // Output the buffer contents in one line
    DPRINTF(CC_Buffer_Flag, "\nCurrent num credits: %d, \nCurrent buffer contents:\n %s\n", currentCredits, bufferContents.c_str());

    linkedFunc();
}

void 
CC_Buffer::linkedFunc()
{
    // std::cout << "linkedFunc called" << std::endl;
    DPRINTF(CC_Buffer_Flag, "debug statement from linked func\n");
}


} // namespace gem5
