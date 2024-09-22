
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
    
    // this is now obsolete - just shows how we can pass in parameters from python
    // that's why this is still here.
    currentCredits = max_credits; //max number of items checker can hold

    //decode buffer setup
    decode_buffer = std::deque<CheckerInst>();
    decode_buffer_max_credits = 20;
    decode_buffer_current_credits = decode_buffer_max_credits;
    decode_buffer_bandwidth = 2;
    decode_buffer_latency = 5;

    //execute buffer setup
    execute_buffer = std::deque<CheckerInst>();
    execute_buffer_max_credits = 20;
    execute_buffer_current_credits = execute_buffer_max_credits;
    execute_buffer_bandwidth = 2;
    execute_buffer_latency = 5;

    //buffer clock setup
    cc_buffer_clock = 0; //clock cycle count, starts at 0
    cc_buffer_clock_period = clockPeriod() + 5; // clock period (chip's period is 333 normally)

    // Initialize the buffer
    buffer = std::deque<CheckerInst>();  // Initialize an empty deque for now
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

    // update the decode buffer contents (remove any instructions that are < 0 instExecuteCycle)
    // pushes items into the execute buffer
    updateDecodeBufferContents();

    // update execute buffer contents
    updateExecuteBufferContents();

    // Reschedule the event to occur again in cc_buffer_clock_period ticks
    schedule(bufferClockEvent, curTick() + cc_buffer_clock_period);
}


void 
CC_Buffer::updateDecodeBufferContents()
{
    int currentItemsRemoved = 0;
    
    // stall the buffer system?
    int buffer_system_stall_flag = 0;

    // Iterate over the decode buffer to find and remove expired instructions
    for (auto it = decode_buffer.begin(); it != decode_buffer.end(); )
    {
        if (it->instDecodeCycle <= cc_buffer_clock) {
            // Print the instruction being moved to execute

            DPRINTF(CC_Buffer_Flag, "---------Decoding instruction: %s---------\n", it->getStaticInst()->getName());
            DPRINTF(CC_Buffer_Flag, "Current cc_buffer_clock: %lu\n", cc_buffer_clock);
            DPRINTF(CC_Buffer_Flag, "Inst instDecodeCycle: %d\n", it->instDecodeCycle);
            DPRINTF(CC_Buffer_Flag, "Num decode credits: %d\n", decode_buffer_current_credits + 1);
            DPRINTF(CC_Buffer_Flag, "Num execute credits: %d\n", execute_buffer_current_credits - 1);

            //TODO: push items here to the execute_buffer
            //TODO: make sure execute_buffer not full
            // Add the string to the buffer

            if (execute_buffer_current_credits <= 0) {
                //ASK: Stall the system?
                buffer_system_stall_flag = 1;
            } else {
                
                //execute buffer not full, so push inst.
                execute_buffer.push_back(*it);
                // reduce num decode credits
                execute_buffer_current_credits--;
                // Remove the instruction from the buffer
                it = decode_buffer.erase(it);
                // update credits available
                decode_buffer_current_credits++;
                currentItemsRemoved++;
            }

            
            if (currentItemsRemoved >= decode_buffer_bandwidth) {
                DPRINTF(CC_Buffer_Flag, "Max bandwidth of %d reached, no more insts removable\n", decode_buffer_bandwidth);
                return; //want to exit function here if more than decode_buffer_bandwidth number of items have been removed.
            } else if (buffer_system_stall_flag==1) {
                DPRINTF(CC_Buffer_Flag, "Execute buffer reached max credits, no more insts removable\n");
                return; //want to exit function here if execute buffer has no more credits available
            }
        } else {
            ++it;
        }
    }
}


void 
CC_Buffer::updateExecuteBufferContents()
{
    int currentItemsRemoved = 0;

    // Iterate over the execute buffer to find and remove expired instructions
    for (auto it = execute_buffer.begin(); it != execute_buffer.end(); )
    {
        if (it->instExecuteCycle <= cc_buffer_clock) {
            // Print the instruction being moved to execute

            DPRINTF(CC_Buffer_Flag, "---------Executing instruction: %s---------\n", it->getStaticInst()->getName());
            DPRINTF(CC_Buffer_Flag, "Current cc_buffer_clock: %lu\n", cc_buffer_clock);
            DPRINTF(CC_Buffer_Flag, "Inst instExecuteCycle: %d\n", it->instExecuteCycle);
            DPRINTF(CC_Buffer_Flag, "Num decode credits: %d\n", decode_buffer_current_credits);
            DPRINTF(CC_Buffer_Flag, "Num execute credits: %d\n", execute_buffer_current_credits + 1);

            // Remove the instruction from the buffer
            it = execute_buffer.erase(it);

            execute_buffer_current_credits++;
            currentItemsRemoved++;
            if (currentItemsRemoved >= execute_buffer_bandwidth) {
                DPRINTF(CC_Buffer_Flag, "Max bandwidth of %d reached, no more insts removable\n", execute_buffer_bandwidth);
                return; //want to exit function here if more than execute_buffer_bandwidth number of items have been removed.
            }
        } else {
            ++it;
        }
    }
}


/*
returns the number of credits decode_buffer_bandwidth has available
*/
uint
CC_Buffer::getNumCredits()
{
    // TODO: need to change this to decode_buffer_current_credits available
    return decode_buffer_current_credits;
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
 //empty for now, redundant
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
    decode_buffer.push_back(checkerInst);
    // reduce num decode credits
    decode_buffer_current_credits--;

    // Ensure the buffer size does not exceed max_credits
    if (decode_buffer.size() >= decode_buffer_max_credits) {
        DPRINTF(CC_Buffer_Flag, "Max credits reached, cannot add more items, CPU stalled.\n");
    }

    //print buffer contents for debug
    std::string decode_buffer_contents = "[";
    for (auto it = decode_buffer.begin(); it != decode_buffer.end(); ++it) {
        decode_buffer_contents += (*it).getStaticInst()->getName();
        if (std::next(it) != decode_buffer.end()) {
            decode_buffer_contents += ", ";
        }
    }
    decode_buffer_contents += "]";

    // Output the buffer contents in one line
    DPRINTF(CC_Buffer_Flag, "\nCurrent num credits: %d, \nCurrent decode_buffer contents:\n %s\n", currentCredits, decode_buffer_contents.c_str());
}


/*
instantiateObject takes in a DynInst and returns a custom CheckerInst type that will be used by the checker chip
*/

CheckerInst 
CC_Buffer::instantiateObject(const gem5::o3::DynInstPtr &instName)
{
    unsigned long clockPeriodTicks = clockPeriod(); //clock period in ticks, random thing to try put in data struct

    DPRINTF(CC_Buffer_Flag, "\nCurrent cycle: %d, \nCurrent cc_buffer_clock + execute_buffer_latency: %d\n", cc_buffer_clock, cc_buffer_clock + execute_buffer_latency);

    // Create a CheckerInst object with credits as the parameter
    CheckerInst checkerInst(cc_buffer_clock + decode_buffer_latency, //instDecodeCycle = currentCycle + decode_buffer_latency (5)
                            cc_buffer_clock + execute_buffer_latency, //instExecuteCycle = currentCycle + execute_buffer_latency (10)
                            instName->staticInst // staticInst passed in (contains info about the instruction)
                            );

    // Return the created CheckerInst object
    return checkerInst;
}

} // namespace gem5
