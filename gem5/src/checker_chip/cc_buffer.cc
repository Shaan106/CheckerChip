
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CC_Buffer_Flag.hh"

#include "cpu/o3/dyn_inst.hh"

#include "sim/sim_exit.hh"

#include <iostream>

#include <deque> 

#include "cc_inst.hh" // for including new instruction class defn.


namespace gem5
{
/*
Constructor for the CC_buffer. 
*/
CC_Buffer::CC_Buffer(const CC_BufferParams &params) :
    ClockedObject(params), 
    event([this]{ processEvent(); }, name() + ".event"),
    bufferClockEvent([this]{ processBufferClockEvent(); }, name() + ".bufferClockEvent"),
    max_credits(params.maxCredits) // params are from CC_Buffer.py
{
    DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");

    // Initialize the buffer
    buffer = std::deque<CheckerInst>();  // Initialize an empty deque for now

    currentCredits = max_credits; //max number of items checker can hold

    cc_buffer_clock = 0; //clock starts at 0
    cc_buffer_clock_period = clockPeriod() + 5; // clock period (chip's period is 333 normally)

    // decode/execute latencies setting
    num_cycles_to_decode = 5;
    num_cycles_to_execute = 10;

    cc_buffer_bandwidth = 2; // for now max 2 numbers can be removed per cycle

    schedule(bufferClockEvent, curTick() + cc_buffer_clock_period); // start the async clock function
} 

/*
processBufferClockEvent is a function that gets called every cc_buffer_clock_period ticks
it essentially executes async and is in another clock domain to o3
this is where a lot of the scheduling of checker chip events occurs
*/
void CC_Buffer::processBufferClockEvent()
{
    
    //increase clock of cc_buffer by 1
    cc_buffer_clock = cc_buffer_clock + 1;

    // update the buffer contents (remove any instructions that are < 0 instExecuteCycle)
    updateBufferContents();

    // Reschedule the event to occur again in cc_buffer_clock_period ticks
    schedule(bufferClockEvent, curTick() + cc_buffer_clock_period);

    // DPRINTF(CC_Buffer_Flag, "Buffer clock executed, current cc_buffer_clock: %lu <-------------\n", cc_buffer_clock);
}


/*
function that updates the contents of the buffer
ie removes any instructions that can now be executed from buffer
*/
void 
CC_Buffer::updateBufferContents()
{
    int currentItemsRemoved = 0;

    // Iterate over the buffer to find and remove expired instructions
    for (auto it = buffer.begin(); it != buffer.end(); )
    {
        if (it->instExecuteCycle <= cc_buffer_clock) {
            // Print the instruction being removed

            DPRINTF(CC_Buffer_Flag, "---------Removing instruction: %s---------\n", it->getStaticInst()->getName());
            DPRINTF(CC_Buffer_Flag, "Current cc_buffer_clock: %lu\n", cc_buffer_clock);
            DPRINTF(CC_Buffer_Flag, "Inst instExecuteCycle: %d\n", it->instExecuteCycle);
            DPRINTF(CC_Buffer_Flag, "New num credits: %d\n", currentCredits + 1);

            // Remove the instruction from the buffer
            it = buffer.erase(it);

            currentCredits++;
            currentItemsRemoved++;
            if (currentItemsRemoved >= cc_buffer_bandwidth) {
                DPRINTF(CC_Buffer_Flag, "Max bandwidth of %d reached, no more insts removable\n", cc_buffer_bandwidth);
                return; //want to exit function here if more than cc_buffer_bandwidth number of items have been removed.
            }
        } else {
            ++it;
        }
    }
}


/*
returns the number of credits buffer has available
*/
uint
CC_Buffer::getNumCredits()
{
    return currentCredits;
}


/*
destructor in case we need to deal with memory stuff
*/
CC_Buffer::~CC_Buffer()
{
    // DPRINTF(CC_Buffer_Flag, "Destructor called\n");
}


/*
processEvent is a legacy function, not needed post v2.2
*/
void
CC_Buffer::processEvent()
{
    // // std::cout << "hi again..." << std::endl;
    // DPRINTF(CC_Buffer_Flag, "Clearing buffer...\n");
    // // remove 10 elements from the front of the buffer
    // for (int i = 0; i < 10; i++) {
    //     buffer.pop_front();
    // }
    // // increase num credits by 10
    // currentCredits += 10;
}


/*
 pushCommit called from o3 commit to try push instruction onto buffer stack
*/
void
CC_Buffer::pushCommit(const gem5::o3::DynInstPtr &instName)
{

    // convert instruction into custom checker type
    CheckerInst checkerInst = instantiateObject(instName);

    
    // random clock and other debug statements
    DPRINTF(CC_Buffer_Flag, "Current tick: %lu\n", curTick()); // Print the current simulation tick
    Tick period = clockPeriod(); // Get and print the clock period in ticks
    DPRINTF(CC_Buffer_Flag, "Clock period: %lu ticks\n", period);
    Cycles currentCycle = Cycles(clockEdge() / clockPeriod());     // Compute and print the current clock cycle
    DPRINTF(CC_Buffer_Flag, "Current clock cycle: %lu\n", currentCycle);
    DPRINTF(CC_Buffer_Flag, "pushed instruction name: %s\n", checkerInst.getStaticInst()->getName());


    // Add the string to the buffer
    buffer.push_back(checkerInst);
    // reduce num credits
    currentCredits--;

    // Ensure the buffer size does not exceed 20
    if (buffer.size() >= max_credits) {
        DPRINTF(CC_Buffer_Flag, "Max credits reached, cannot add more items, CPU stalled.\n");
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
}


/*
instantiateObject takes in a DynInst and returns a custom CheckerInst type that will be used by the checker chip
*/

CheckerInst 
CC_Buffer::instantiateObject(const gem5::o3::DynInstPtr &instName)
{
    unsigned long clockPeriodTicks = clockPeriod(); //clock period in ticks, random thing to try put in data struct

    DPRINTF(CC_Buffer_Flag, "\nCurrent cycle: %d, \nCurrent cc_buffer_clock + num_cycles_to_execute: %d\n", cc_buffer_clock, cc_buffer_clock + num_cycles_to_execute);

    // Create a CheckerInst object with credits as the parameter
    CheckerInst checkerInst(cc_buffer_clock + num_cycles_to_decode, //instDecodeCycle = currentCycle + num_cycles_to_decode (5)
                            cc_buffer_clock + num_cycles_to_execute, //instExecuteCycle = currentCycle + num_cycles_to_execute (10)
                            instName->staticInst // staticInst passed in (contains info about the instruction)
                            );

    // Return the created CheckerInst object
    return checkerInst;
}

} // namespace gem5
