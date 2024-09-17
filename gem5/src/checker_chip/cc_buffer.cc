
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
    bufferClockEvent([this]{ processBufferClockEvent(); }, name() + ".bufferClockEvent"),
    maxCredits(params.maxCredits)
{
    DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");

    // Initialize the buffer
    buffer = std::deque<CheckerInst>();  // Initialize an empty deque for now

    currentCredits = maxCredits;

    creditsFreed = 0;

    schedule(bufferClockEvent, curTick() + 1000);
}

// Implement bufferClockEvent fire method
void CC_Buffer::processBufferClockEvent()
{
    DPRINTF(CC_Buffer_Flag, "fire!!!! \n");

    // Reschedule the event to occur again in 1000 ticks
    schedule(bufferClockEvent, curTick() + 1000);
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
    // remove 10 elements from the front of the buffer
    for (int i = 0; i < 10; i++) {
        buffer.pop_front();
    }
    // increase num credits by 10
    currentCredits += 10;
}

void
CC_Buffer::pushCommit(const gem5::o3::DynInstPtr &instName)
{
    // std::cout << "hi" << std::endl;

    CheckerInst checkerInst = instantiateObject(instName);

    // Print the current simulation tick
    DPRINTF(CC_Buffer_Flag, "Current tick: %lu\n", curTick());

    // Get and print the clock period in ticks
    Tick period = clockPeriod();
    DPRINTF(CC_Buffer_Flag, "Clock period: %lu ticks\n", period);

    // Compute and print the current clock cycle
    Cycles currentCycle = Cycles(clockEdge() / clockPeriod());
    DPRINTF(CC_Buffer_Flag, "Current clock cycle: %lu\n", currentCycle);
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
    unsigned long clockPeriodTicks = clockPeriod(); //clock period in ticks

    // Create a CheckerInst object with credits as the parameter
    CheckerInst checkerInst(clockPeriodTicks,
                            5, //timeUntilDecode
                            10, // timeUnitlExecute
                            instName->staticInst);

    DPRINTF(CC_Buffer_Flag, "the associated clk period is %d ticks\n", checkerInst.timeUntilExecute);

    // Return the created CheckerInst object
    return checkerInst;
}

} // namespace gem5
