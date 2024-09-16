
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CC_Buffer_Flag.hh"

#include "cpu/o3/dyn_inst.hh"

#include "sim/sim_exit.hh"
// #include "sim/clocked_object.hh"  // Include clocked_object instead of sim_object

#include <iostream>

#include <deque> 

#include "cc_inst.hh" // for including new instruction class defn.


namespace gem5
{
/*
Constructor for the GoodbyeObject. 
*/
CC_Buffer::CC_Buffer(const CC_BufferParams &params) :
    ClockedObject(params),  // Change SimObject to ClockedObject
    // SimObject(params), 
    event([this]{ processEvent(); }, name() + ".event"),
    maxCredits(params.maxCredits)
{
    DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");

    // Initialize the buffer
    buffer = std::deque<CheckerInst>();  // Initialize an empty deque for now

    currentCredits = maxCredits;
}

uint
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
// CC_Buffer::pushCommit(const StaticInstPtr &instName)
CC_Buffer::pushCommit(const gem5::o3::DynInstPtr &instName)
{
    // std::cout << "hi" << std::endl;

    CheckerInst checkerInst = instantiateObject(instName);

    DPRINTF(CC_Buffer_Flag, "Debug statement... in cc_buffer now\n");

    // DPRINTF(CC_Buffer_Flag, "pushed instruction name: %s\n", instName->getName().c_str());
    DPRINTF(CC_Buffer_Flag, "pushed instruction name: %s\n", checkerInst.getStaticInst()->getName());

    // Add the string to the buffer
    buffer.push_back(checkerInst);
    // reduce num credits
    currentCredits--;

    // Ensure the buffer size does not exceed 20
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
        bufferContents += (*it).getStaticInst()->getName();
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

CheckerInst 
CC_Buffer::instantiateObject(const gem5::o3::DynInstPtr &instName)
{
    // Assuming instName->credits is an int
    int credits = instName->staticInst->numSrcRegs(); // Access the credits attribute

    // Create a CheckerInst object with credits as the parameter
    CheckerInst checkerInst(credits,
                            instName->staticInst);

    // Return the created CheckerInst object
    return checkerInst;
}

} // namespace gem5
