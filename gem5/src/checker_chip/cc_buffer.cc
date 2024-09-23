
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
    bufferClockEvent([this]{ processBufferClockEvent(); }, name() + ".bufferClockEvent"),
    max_credits(params.maxCredits) // params are from CC_Buffer.py
{
    DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");
    
    // this is now obsolete - just shows how we can pass in parameters from python
    // that's why this is still here.
    // max_credits; //max number of items checker can hold set from python side

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

    //functional units setup
    initializeFuncUnit(funcUnit);

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
 pushCommit called from o3 commit to try push instruction onto buffer stack
*/
void
CC_Buffer::pushCommit(const gem5::o3::DynInstPtr &instName)
{

    // convert instruction into custom checker type
    CheckerInst checkerInst = instantiateObject(instName);

    // random clock and other debug statements
    // DPRINTF(CC_Buffer_Flag, "Current tick: %lu\n", curTick()); // Print the current simulation tick
    // Tick period = clockPeriod(); // Get and print the clock period in ticks
    // // DPRINTF(CC_Buffer_Flag, "Clock period: %lu ticks\n", period);
    Cycles currentCycle = Cycles(clockEdge() / clockPeriod());     // Compute and print the current clock cycle
    DPRINTF(CC_Buffer_Flag, "Current CPU clock cycle: %lu\n", currentCycle);
    DPRINTF(CC_Buffer_Flag, "pushed instruction name: %s\n", checkerInst.getStaticInst()->getName());

    // test for functional unit
    int inst_latency = getOperationLatency(checkerInst.getStaticInst()->opClass());
    DPRINTF(CC_Buffer_Flag, "!!!!!!! ---------- Latency for operation is %d, cycle to execute is %d --------- !!!!!!!!!\n", inst_latency, checkerInst.instExecuteCycle);
    // DPRINTF(CC_Buffer_Flag, "!!!!!!! ---------- Latency for operation is %d --------- !!!!!!!!!\n", inst_latency);

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
    DPRINTF(CC_Buffer_Flag, "\nCurrent num credits: %d, \nCurrent decode_buffer contents:\n %s\n", decode_buffer_current_credits, decode_buffer_contents.c_str());
}


/*
instantiateObject takes in a DynInst and returns a custom CheckerInst type that will be used by the checker chip
*/

CheckerInst 
CC_Buffer::instantiateObject(const gem5::o3::DynInstPtr &instName)
{
    unsigned long clockPeriodTicks = clockPeriod(); //clock period in ticks, random thing to try put in data struct
    int inst_execute_latency = getOperationLatency(instName->staticInst->opClass());
    DPRINTF(CC_Buffer_Flag, "\nCurrent cycle: %d, \nCurrent cc_buffer_clock + inst_execute_latency: %d\n", cc_buffer_clock, cc_buffer_clock + inst_execute_latency);
    // DPRINTF(CC_Buffer_Flag, "\nCurrent cycle: %d, \nCurrent cc_buffer_clock + inst_execute_latency: %d\n", cc_buffer_clock, cc_buffer_clock + execute_buffer_latency);

    // Create a CheckerInst object with credits as the parameter
    CheckerInst checkerInst(cc_buffer_clock + decode_buffer_latency, //instDecodeCycle = currentCycle + decode_buffer_latency (5)
                            cc_buffer_clock + inst_execute_latency, //instExecuteCycle = currentCycle + inst execute latency
                            instName->staticInst // staticInst passed in (contains info about the instruction)
                            );

    // Return the created CheckerInst object
    return checkerInst;
}

/*
getOperationLatency gets the operation latency from a given operation and returns it.
*/
int CC_Buffer::getOperationLatency(OpClass op_class) {
    int returnLatency = funcUnit.getLatencyForOp(op_class);

    if (returnLatency == 0) {
        return 4; // default val, not having 0 latency
    }

    return returnLatency;
}

/*
initializeFuncUnit initializes the functional unit and gives each inst some latency
*/
void CC_Buffer::initializeFuncUnit(FuncUnit &funcUnit) {
    unsigned constant_latency = 5;

    // Add capabilities for all the OpClasses defined in op_class.hh
    // The names like IntAluOp are from op_class.hh which is within func_unit.hh

    // this is awful i don't like how this is done but idk how else for now.

    //int
    funcUnit.addCapability(IntAluOp, constant_latency, false);
    funcUnit.addCapability(IntMultOp, constant_latency, false);
    funcUnit.addCapability(IntDivOp, constant_latency, false);

    //float
    funcUnit.addCapability(FloatAddOp, constant_latency, false);
    funcUnit.addCapability(FloatCmpOp, constant_latency, false);
    funcUnit.addCapability(FloatCvtOp, constant_latency, false);
    funcUnit.addCapability(FloatMultOp, constant_latency, false);
    funcUnit.addCapability(FloatMultAccOp, constant_latency, false);
    funcUnit.addCapability(FloatDivOp, constant_latency, false);
    funcUnit.addCapability(FloatMiscOp, constant_latency, false);
    funcUnit.addCapability(FloatSqrtOp, constant_latency, false);

    // funcUnit.addCapability(SimdAddOp, constant_latency, false);
    // funcUnit.addCapability(SimdAddAccOp, constant_latency, false);
    // funcUnit.addCapability(SimdAluOp, constant_latency, false);
    // funcUnit.addCapability(SimdCmpOp, constant_latency, false);
    // funcUnit.addCapability(SimdCvtOp, constant_latency, false);
    // funcUnit.addCapability(SimdMiscOp, constant_latency, false);
    // funcUnit.addCapability(SimdMultOp, constant_latency, false);
    // funcUnit.addCapability(SimdMultAccOp, constant_latency, false);
    // funcUnit.addCapability(SimdMatMultAccOp, constant_latency, false);
    // funcUnit.addCapability(SimdShiftOp, constant_latency, false);
    // funcUnit.addCapability(SimdShiftAccOp, constant_latency, false);
    // funcUnit.addCapability(SimdDivOp, constant_latency, false);
    // funcUnit.addCapability(SimdSqrtOp, constant_latency, false);
    // funcUnit.addCapability(SimdReduceAddOp, constant_latency, false);
    // funcUnit.addCapability(SimdReduceAluOp, constant_latency, false);
    // funcUnit.addCapability(SimdReduceCmpOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatAddOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatAluOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatCmpOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatCvtOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatDivOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatMiscOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatMultOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatMultAccOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatMatMultAccOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatSqrtOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatReduceCmpOp, constant_latency, false);
    // funcUnit.addCapability(SimdFloatReduceAddOp, constant_latency, false);
    // funcUnit.addCapability(SimdAesOp, constant_latency, false);
    // funcUnit.addCapability(SimdAesMixOp, constant_latency, false);
    // funcUnit.addCapability(SimdSha1HashOp, constant_latency, false);
    // funcUnit.addCapability(SimdSha1Hash2Op, constant_latency, false);
    // funcUnit.addCapability(SimdSha256HashOp, constant_latency, false);
    // funcUnit.addCapability(SimdSha256Hash2Op, constant_latency, false);
    // funcUnit.addCapability(SimdShaSigma2Op, constant_latency, false);
    // funcUnit.addCapability(SimdShaSigma3Op, constant_latency, false);
    // funcUnit.addCapability(SimdPredAluOp, constant_latency, false);
    // funcUnit.addCapability(MatrixOp, constant_latency, false);
    // funcUnit.addCapability(MatrixMovOp, constant_latency, false);
    // funcUnit.addCapability(MatrixOPOp, constant_latency, false);
    // funcUnit.addCapability(MemReadOp, constant_latency, false);
    // funcUnit.addCapability(MemWriteOp, constant_latency, false);
    // funcUnit.addCapability(FloatMemReadOp, constant_latency, false);
    // funcUnit.addCapability(FloatMemWriteOp, constant_latency, false);
    // funcUnit.addCapability(IprAccessOp, constant_latency, false);
    // funcUnit.addCapability(InstPrefetchOp, constant_latency, false);
    // funcUnit.addCapability(VectorUnitStrideLoadOp, constant_latency, false);
    // funcUnit.addCapability(VectorUnitStrideStoreOp, constant_latency, false);
    // funcUnit.addCapability(VectorUnitStrideMaskLoadOp, constant_latency, false);
    // funcUnit.addCapability(VectorUnitStrideMaskStoreOp, constant_latency, false);
    // funcUnit.addCapability(VectorStridedLoadOp, constant_latency, false);
    // funcUnit.addCapability(VectorStridedStoreOp, constant_latency, false);
    // funcUnit.addCapability(VectorIndexedLoadOp, constant_latency, false);
    // funcUnit.addCapability(VectorIndexedStoreOp, constant_latency, false);
    // funcUnit.addCapability(VectorUnitStrideFaultOnlyFirstLoadOp, constant_latency, false);
    // funcUnit.addCapability(VectorWholeRegisterLoadOp, constant_latency, false);
    // funcUnit.addCapability(VectorWholeRegisterStoreOp, constant_latency, false);
    // funcUnit.addCapability(VectorIntegerArithOp, constant_latency, false);
    // funcUnit.addCapability(VectorFloatArithOp, constant_latency, false);
    // funcUnit.addCapability(VectorFloatConvertOp, constant_latency, false);
    // funcUnit.addCapability(VectorIntegerReduceOp, constant_latency, false);
    // funcUnit.addCapability(VectorFloatReduceOp, constant_latency, false);
    // funcUnit.addCapability(VectorMiscOp, constant_latency, false);
    // funcUnit.addCapability(VectorIntegerExtensionOp, constant_latency, false);
    // funcUnit.addCapability(VectorConfigOp, constant_latency, false);
    // funcUnit.addCapability(Num_OpClasses, constant_latency, false);
    }

} // namespace gem5
