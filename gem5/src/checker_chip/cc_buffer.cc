
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CC_Buffer_Flag.hh"

#include "cpu/o3/dyn_inst.hh"

#include "sim/sim_exit.hh"

#include <iostream>

#include <deque> 

#include "cc_inst.hh" // for including new instruction class defn.

#include "checker_chip/checker_cache/checker_packet_state.hh"


namespace gem5
{
/*
Constructor for the CC_buffer. 
*/
CC_Buffer::CC_Buffer(const CC_BufferParams &params)
    : ClockedObject(params), // Initialize base class ClockedObject with params
      bufferClockEvent([this] { processBufferClockEvent(); }, name() + ".bufferClockEvent"), // Initialize bufferClockEvent with the provided lambda function
      max_credits(128), // Initialize max_credits using the value from params

      decode_buffer(std::deque<CheckerInst>()), // Initialize decode_buffer as an empty deque explicitly

      decode_buffer_bandwidth(2), // Set decode_buffer_bandwidth to 2
      decode_buffer_latency(3), // Set decode_buffer_latency to 5

      decode_buffer_credits(
                        &cc_buffer_clock,
                        128, //params.maxCredits, //max_credits
                        1, //unsigned long default_latency_add = 1
                        0 //unsigned long default_latency_remove = 0
                        ), // Initialize decode_buffer_credits using   

      execute_buffer_credits(
                        &cc_buffer_clock,
                        128, //params.maxCredits, //max_credits
                        1, //unsigned long default_latency_add = 1
                        0 //unsigned long default_latency_remove = 0
                        ), // Initialize decode_buffer_credits using   

      execute_buffer(std::deque<CheckerInst>()), // Initialize execute_buffer as an empty deque explicitly

      cc_buffer_clock(0), // Initialize cc_buffer_clock to 0
      cc_buffer_clock_period(clockPeriod()*4), // Set cc_buffer_clock_period using clockPeriod() + 5
      
      functional_unit_pool(params.checkerFUPool), // FU pool

      checker_regfile(
            &cc_buffer_clock,
            8, //latency
            4 //bandwidth
      ), //Checker regfile

      tlb(params.tlbEntries, params.tlbAssociativity, params.tlbHitLatency, params.tlbMissLatency),

      instCount(0),
      debugStringMap({}),
      cc_mem_side_port(name() + ".cc_mem_side_port", this), // for ports
      system(params.system), // Initialize the system pointer
      requestorId(1000),
      memVerifyAddrSet()
{
    // DPRINTF(CC_Buffer_Flag, "CC_Buffer: Constructor called\n");

    std::string obj_name = name();
    // DPRINTF(CC_Buffer_Flag, "CC_Buffer: Object name is %s\n", obj_name);
    
    // stats registered in regStats

    functional_unit_pool->dump(); // debug statement to check if functional pools exist

    // Schedule the buffer clock event to trigger after the initial period
    schedule(bufferClockEvent, curTick() + cc_buffer_clock_period);
}

// Override the init() method
void
CC_Buffer::init()
{
    // Call the base class init()
    ClockedObject::init();

    // Obtain the RequestorID
    requestorId = system->getRequestorId(this);

    // You can add any additional initialization here
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

    // Process units that need to be freed at the end of this cycle (has to be after the updating of buffer contents)
    functional_unit_pool->processFreeUnits();

    // test with new system
    decode_buffer_credits.updateCredits();
    execute_buffer_credits.updateCredits();

    // update Regfile stuff
    regfile_insts_processed = regfile_insts_processed.value() + checker_regfile.updateRegfile();

    //stats update for the current clock cycle
    cc_buffer_cycles++;
    decode_buffer_occupancy_histogram.sample(decode_buffer.size());
    execute_buffer_occupancy_histogram.sample(execute_buffer.size());

    // Reschedule the event to occur again in cc_buffer_clock_period ticks
    schedule(bufferClockEvent, curTick() + cc_buffer_clock_period);

    //DEBUG for buffer clocks
    if (cc_buffer_clock % 100 == 0) {
        // DPRINTF(CC_Buffer_Flag, "clock_cycle: %lu\n", cc_buffer_clock);

        // sendDummyPacket();

        for (const auto& pair : debugStringMap) {
            // DPRINTF(CC_Buffer_Flag, "Key: %s, Value: %d\n", pair.first.c_str(), pair.second);
        }
    }

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

            // DPRINTF(CC_Buffer_Flag, "---------Decoding instruction: %s---------\n", it->getStaticInst()->getName());
            // DPRINTF(CC_Buffer_Flag, "Current cc_buffer_clock: %lu\n", cc_buffer_clock);
            // DPRINTF(CC_Buffer_Flag, "Inst instDecodeCycle: %d\n", it->instDecodeCycle);
            // // DPRINTF(CC_Buffer_Flag, "Num decode credits: %d\n", decode_buffer_current_credits + 1);
            // DPRINTF(CC_Buffer_Flag, "Num decode credits: %d\n", decode_buffer_credits.getCredits() + 1);
            // // DPRINTF(CC_Buffer_Flag, "Num execute credits: %d\n", execute_buffer_current_credits - 1);
            // DPRINTF(CC_Buffer_Flag, "New num execute credits: %d\n", execute_buffer_credits.getCredits() - 1);

            //TODO: push items here to the execute_buffer
            //TODO: make sure execute_buffer not full
            // Add the string to the buffer

            // if (execute_buffer_current_credits <= 0) {
            if (execute_buffer_credits.getCredits() <= 0) {
                //ASK: Stall the system?
                buffer_system_stall_flag = 1;

            } else {
                //execute buffer not full, so push inst.
                execute_buffer.push_back(*it);
                // reduce num decode credits
                // execute_buffer_current_credits--;
                execute_buffer_credits.decrementCredit();
                // Remove the instruction from the buffer
                it = decode_buffer.erase(it);
                // update credits available
                // decode_buffer_current_credits++;
                decode_buffer_credits.addCredit();
                currentItemsRemoved++;
            }

            
            if (currentItemsRemoved >= decode_buffer_bandwidth) {
                // DPRINTF(CC_Buffer_Flag, "Max bandwidth of %d reached, no more insts removable\n", decode_buffer_bandwidth);
                return; //want to exit function here if more than decode_buffer_bandwidth number of items have been removed.
            } else if (buffer_system_stall_flag==1) {
                // DPRINTF(CC_Buffer_Flag, "Execute buffer reached max credits, no more insts removable\n");
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
    for (auto it = execute_buffer.begin(); it != execute_buffer.end(); )
    {
        if (it->instExecuteCycle <= cc_buffer_clock && it->instInFU == true) {
            // DPRINTF(CC_Buffer_Flag, "---------Finished executing instruction: %s---------\n", it->getStaticInst()->getName());
            // DPRINTF(CC_Buffer_Flag, "Current cc_buffer_clock: %lu\n", cc_buffer_clock);
            // DPRINTF(CC_Buffer_Flag, "Inst instExecuteCycle: %d\n", it->instExecuteCycle);

            if (!it->execVerify_bit) {
                it->execVerify_bit = true;
                // DPRINTF(CC_Buffer_Flag, "Marking instruction as functionally verified and freeing functional unit %d\n", it->functional_unit_index);
                functional_unit_pool->freeUnitNextCycle(it->functional_unit_index);
            }

            if (!it->memVerify_bit) {
                // if memVerify is false inst is a mem inst
                // DPRINTF(CC_Buffer_Flag, "Marking memory instruction as functionally verified and freeing functional unit %d\n", it->functional_unit_index);
                auto searchedAddr = memVerifyAddrSet.find(it->uniqueInstSeqNum); //try to find uniqueInstSeqNum in set

                // DPRINTF(CC_Buffer_Flag, "%d\n", memVerifyAddrSet.size());

                if (searchedAddr == memVerifyAddrSet.end()){ // if uniqueInstSeqNum is not in set then it is verified
                    it->memVerify_bit = true;
                    functional_unit_pool->freeUnitNextCycle(it->functional_unit_index);
                }
            }

            bool inst_verified = it->execVerify_bit && it->iVerify_bit && it->memVerify_bit;

            if (inst_verified) {
                // DPRINTF(CC_Buffer_Flag, "Instruction verified: %s\n", it->getStaticInst()->getName());

                if (!checker_regfile.isBandwidthFull()) {
                    // DPRINTF(CC_Buffer_Flag, "Staging instruction to regfile, not full yet.\n");
                    it = execute_buffer.erase(it);
                    checker_regfile.stageInstToRegfile();
                    execute_buffer_credits.addCredit();
                } else {
                    // DPRINTF(CC_Buffer_Flag, "Bandwidth full, instruction cannot be staged.\n");
                    ++it;  // Ensure iterator is advanced to prevent re-processing
                    return;
                }

            } else {
                // DPRINTF(CC_Buffer_Flag, "Instruction not fully verified: %s\n", it->getStaticInst()->getName());
                return;  // If not verified, exit early and recheck in the next cycle
            }

        } else {
            int free_FU_idx = functional_unit_pool->getUnit(it->getStaticInst()->opClass());

            if (free_FU_idx >= 0) {
                it->instInFU = true;
                
                // it->memVerify_bit = true;
                it->functional_unit_index = free_FU_idx;
                // DPRINTF(CC_Buffer_Flag, "Assigned functional unit %d to instruction %s\n", free_FU_idx, it->getStaticInst()->getName());

                // OpClass op_class = it->getStaticInst()->opClass();

                if (it->isReadInst()) {
                    // DPRINTF(CC_Buffer_Flag, "A memory read operation, sending MemReadPacket, memVerifyAddrSet.size(): %d.\n", memVerifyAddrSet.size());

                    if (sendReadReqPacket(*it)) { // if able to send inst to mem (buffer not full)
                        it->execVerify_bit = true;
                        it->instExecuteCycle = cc_buffer_clock;
                    } else {
                        it->instInFU = false;
                        functional_unit_pool->freeUnitNextCycle(it->functional_unit_index);
                    }

                    // sendDummyPacket();
                } else if (it->isWriteInst()) {
                    // DPRINTF(CC_Buffer_Flag, "A memory write operation, sending MemWritePacket, memVerifyAddrSet.size(): %d.\n", memVerifyAddrSet.size());
                    // it->execVerify_bit = true; // no need for exec verify, so set true now
                    // it->instExecuteCycle = cc_buffer_clock;
                    // sendWriteReqPacket(*it);

                    if (sendWriteReqPacket(*it)) { // if able to send inst to mem (buffer not full)
                        it->execVerify_bit = true;
                        it->instExecuteCycle = cc_buffer_clock;
                    } else {
                        it->instInFU = false;
                        functional_unit_pool->freeUnitNextCycle(it->functional_unit_index);
                    }
                    // sendDummyPacket();
                } else {
                    // it->memVerify_bit = true; // no need for mem verify, so set true now
                    it->memVerify_bit = true;
                    it->instExecuteCycle = cc_buffer_clock + functional_unit_pool->getOpLatency(it->getStaticInst()->opClass());
                    // DPRINTF(CC_Buffer_Flag, "Instruction is not a memory operation.\n");
                }

                ++it;

            } else if (free_FU_idx == -1) {
                // DPRINTF(CC_Buffer_Flag, "No functional units free for instruction: %s\n", it->getStaticInst()->getName());
                return;

            } else {
                if (it->getStaticInst()->getName() == "fault") {
                    it = execute_buffer.erase(it);
                    execute_buffer_credits.addCredit();
                } else {
                    // DPRINTF(CC_Buffer_Flag, "No FUs capable of executing instruction: %s\n", it->getStaticInst()->getName());
                    return;
                }
            }
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
    // return decode_buffer_current_credits;
    return decode_buffer_credits.getCredits();

}


/*
destructor in case we need to deal with memory stuff
*/
CC_Buffer::~CC_Buffer()
{
    // // DPRINTF(CC_Buffer_Flag, "Destructor called\n");
}


/*
 pushCommit called from o3 commit to try push instruction onto buffer stack
*/
void
CC_Buffer::pushCommit(const gem5::o3::DynInstPtr &instName)
{
    //
    instCount = instCount + 1;

    // convert instruction into custom checker type
    CheckerInst checkerInst = instantiateObject(instName);

    // set verification bits for all things not implemented yet
    checkerInst.iVerify_bit = true;

    Cycles currentCycle = Cycles(clockEdge() / clockPeriod());     // Compute and print the current clock cycle
    // DPRINTF(CC_Buffer_Flag, "Current CPU clock cycle: %lu\n", currentCycle);
    // DPRINTF(CC_Buffer_Flag, "Current cc_buffer clock cycle: %lu\n", cc_buffer_clock);
    // DPRINTF(CC_Buffer_Flag, "pushed instruction name: %s\n", checkerInst.getStaticInst()->getName());

    // test for functional unit
    debugStringMap[checkerInst.getStaticInst()->getName()] = functional_unit_pool->getOpLatency(checkerInst.getStaticInst()->opClass());
    int inst_latency = functional_unit_pool->getOpLatency(checkerInst.getStaticInst()->opClass());
    // DPRINTF(CC_Buffer_Flag, "!!!!!!! ---------- Latency for operation is %d, cycle to execute is %lu --------- !!!!!!!!!\n", inst_latency, cc_buffer_clock + inst_latency);


    // Add the string to the buffer
    decode_buffer.push_back(checkerInst);

    // reduce num decode credits
    decode_buffer_credits.decrementCredit();

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
    // // DPRINTF(CC_Buffer_Flag, "\nCurrent num credits: %d, \nCurrent decode_buffer contents:\n %s\n", decode_buffer_current_credits, decode_buffer_contents.c_str());
    // DPRINTF(CC_Buffer_Flag, "\nCurrent num credits: %d, \nCurrent decode_buffer contents:\n %s\n", decode_buffer_credits.getCredits(), decode_buffer_contents.c_str());
}


/*
instantiateObject takes in a DynInst and returns a custom CheckerInst type that will be used by the checker chip
*/

CheckerInst 
CC_Buffer::instantiateObject(const gem5::o3::DynInstPtr &instName)
{
    unsigned long clockPeriodTicks = clockPeriod(); //clock period in ticks, random thing to try put in data struct
    
    Addr inst_addr = instName->pcState().instAddr();
    unsigned int tlb_latency = tlb.translate(inst_addr);

    // DPRINTF(CC_Buffer_Flag, " inst->seqNum : %lu\n", instName->seqNum);

    // Create a CheckerInst object with credits as the parameter
    CheckerInst checkerInst(cc_buffer_clock + decode_buffer_latency, //instDecodeCycle = currentCycle + decode_buffer_latency (5)
                            0, //instExecuteCycle is initially 0
                            cc_buffer_clock + tlb_latency, //instTranslationCycle, when the inst is translated
                            instName->seqNum,// unique identifier num
                            false, // instInFU
                            instName->staticInst // staticInst passed in (contains info about the instruction)
                            );

    // DPRINTF(CC_Buffer_Flag, "!!!!!!!!!!!!TLB LATENCY !!!!!!!!!!!!!!!!!! : %lu\n", checkerInst.instTranslationCycle);

    if (instName->isMemRef()) {

    //         /** The size of the request */
    // unsigned effSize;

    // /** Pointer to the data for the memory access. */
    // uint8_t *memData = nullptr;
        if (instName->isStore()) {
            checkerInst.setMemAddresses(instName->effAddr, 
                            instName->physEffAddr,
                            instName->getStoreDataSize(),
                            instName->getStoreData());
        } else {
            checkerInst.setMemAddresses(instName->effAddr, 
                            instName->physEffAddr,
                            instName->effSize,
                            instName->memData);
        }


        // DPRINTF(CC_Buffer_Flag, "inst: %s, v_addr: 0x%x, p_addr = 0x%x\n", checkerInst.getStaticInst()->getName(), checkerInst.v_addr, checkerInst.p_addr);
        // DPRINTF(CC_Buffer_Flag, "mem_data_size: %d, mem_data: \n", checkerInst.mem_access_data_size);
        for (unsigned i = 0; i < checkerInst.mem_access_data_size; ++i) {
            if (checkerInst.mem_access_data_ptr == nullptr) {
                // DPRINTF(CC_Buffer_Flag, "Error: mem_data_ptr is null.\n");
            } else {
                // DPRINTF(CC_Buffer_Flag, "0x%x \n", checkerInst.mem_access_data_ptr[i]);
            }
        }
        // DPRINTF(CC_Buffer_Flag, "\n");
    }
    // Return the created CheckerInst object
    return checkerInst;
}

void
CC_Buffer::addStallCycle() 
{
    ooo_stall_signals++;
}

void CC_Buffer::regStats()
{
    ClockedObject::regStats(); // Call base class regStats()

    // Initialize statistics
    cc_buffer_cycles
        .name(name() + ".cc_buffer_cycles")
        .desc("Number of cc_buffer cycles executed")
        .flags(statistics::total);

    ooo_stall_signals
        .name(name() + ".ooo_stall_signals")
        .desc("Number of cycles stalled due to buffer")
        .flags(statistics::total);

    regfile_insts_processed
        .name(name() + ".regfile_insts_processed")
        .desc("Number of cycles stalled due to buffer")
        .flags(statistics::total);

    decode_buffer_occupancy_histogram
        .init(int64_t(0), int64_t(max_credits), int64_t(1))
        .name(name() + ".decode_buffer_occupancy_histogram")
        .desc("Distribution of decode buffer occupancy")
        .flags(statistics::pdf | statistics::display);

    execute_buffer_occupancy_histogram
        .init(int64_t(0), int64_t(max_credits), int64_t(1))
        .name(name() + ".execute_buffer_occupancy_histogram")
        .desc("Distribution of decode buffer occupancy")
        .flags(statistics::pdf | statistics::display);
    // Set up the formula for average occupancy
    // decode_buffer_occupancy_avg = decode_buffer_occupancy_avg / cc_buffer_cycles;

    // initially = 0
    // decode_buffer_occupancy_maximum = 0;
}


bool
CC_Buffer::sendReadReqPacket(CheckerInst memInst)
{
    // DPRINTF(CC_Buffer_Flag, "CC_Buffer: Creating and sending packet(s) for address 0x%x, size %d.\n", memInst.p_addr, memInst.mem_access_data_size);

    Addr addr = memInst.p_addr; // Starting address

    memVerifyAddrSet.insert(memInst.uniqueInstSeqNum); // mark inst as being verified in mem

    unsigned size = memInst.mem_access_data_size; // Request size in bytes
    unsigned blkSize = cc_mem_side_port.getCacheBlockSize(); // Cache block size
    unsigned remaining = size;

    while (remaining > 0) {
        // Determine the size for the current packet
        unsigned offset = addr % blkSize;
        unsigned currSize = std::min(remaining, blkSize - offset);

        // Create the request and packet
        RequestPtr req = std::make_shared<Request>(addr, currSize, 0, requestorId);
        PacketPtr pkt = new Packet(req, MemCmd::ReadReq);
        pkt->allocate();
        pkt->senderState = new CC_PacketState(requestorId-53, //coreID (requestorIDs were 53-60 experimentally)
                                              memInst.uniqueInstSeqNum, //unique identifier for verification on return
                                              42, // custom info
                                              "CustomTag" //custom info
                                              );

        // Send the packet
        // DPRINTF(CC_Buffer_Flag, "Sending packet: addr = 0x%x, size = %d, offset = %d\n", addr, currSize, offset);
        bool sendSuccess = cc_mem_side_port.sendPacket(pkt);
        
        if (!sendSuccess) {
            DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Packet BLOCKED (sendReadReqPacket).\n");
        }

        // Update for the next packet
        addr += currSize;
        remaining -= currSize;

        return sendSuccess;
    }
}


bool
CC_Buffer::sendWriteReqPacket(CheckerInst memInst)
{
    // DPRINTF(CC_Buffer_Flag, "CC_Buffer: Creating and sending write request packet(s) for address 0x%x, size %d.\n", memInst.p_addr, memInst.mem_access_data_size);

    Addr addr = memInst.p_addr; // Starting address of the write request

    memVerifyAddrSet.insert(memInst.uniqueInstSeqNum);

    unsigned size = memInst.mem_access_data_size; // Total size of the data to write
    unsigned blkSize = cc_mem_side_port.getCacheBlockSize(); // Cache block size
    unsigned remaining = size;

    const uint8_t *data_ptr = memInst.mem_access_data_ptr;

    while (remaining > 0) {
        // Calculate the size for the current packet
        unsigned offset = addr % blkSize;
        unsigned currSize = std::min(remaining, blkSize - offset);

        // Create a request for the current packet
        RequestPtr req = std::make_shared<Request>(addr, currSize, 0, requestorId);

        // Create a WriteReq packet
        PacketPtr pkt = new Packet(req, MemCmd::WriteReq);
        pkt->allocate();

        pkt->senderState = new CC_PacketState(requestorId-53, //coreID (requestorIDs were 53-60 experimentally)
                                        memInst.uniqueInstSeqNum, //unique identifier for return
                                        42, // custom info
                                        "CustomTag" //custom info
                                        );

        // Copy the data for the current packet
        uint8_t *pktData = pkt->getPtr<uint8_t>();
        memcpy(pktData, data_ptr, currSize);

        // Send the packet
        // DPRINTF(CC_Buffer_Flag, "Sending write packet: addr = 0x%x, size = %d, offset = %d\n", addr, currSize, offset);
        bool sendSuccess = cc_mem_side_port.sendPacket(pkt);
        if (!sendSuccess) {
            DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Packet BLOCKED (sendWriteReqPacket).\n");
        }

        // Update the address, data pointer, and remaining size
        addr += currSize;
        data_ptr += currSize;
        remaining -= currSize;

        return sendSuccess;
    }
}


void
CC_Buffer::sendDummyPacket()
{
    // DPRINTF(CC_Buffer_Flag, "CC_Buffer: Creating and sending a dummy packet.\n");

    // Create a dummy request
    Addr addr = 0x0; // Dummy address
    unsigned size = 64; // Size of the data in bytes

    RequestPtr req = std::make_shared<Request>(addr, size, 0, requestorId);

    // Create a packet with the request
    PacketPtr pkt = new Packet(req, MemCmd::ReadReq);

    // Allocate space for data (even for ReadReq, to store read data)
    pkt->allocate();

    // Optionally, initialize data (for write requests)
    // For ReadReq, this is not necessary

    // Send the packet through the memory-side port
    cc_mem_side_port.sendPacket(pkt);
}


Port &
CC_Buffer::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "cc_mem_side_port") {
        return cc_mem_side_port;
    } else {
        return SimObject::getPort(if_name, idx);
    }
}

//////////////
//
// CC_MemSidePort Implementation
//
//////////////

unsigned
CC_Buffer::CC_MemSidePort::getCacheBlockSize()
{
    if (isConnected()) {
        // Use getPeer() to get the connected peer
        auto &connected_port = getPeer();

        // Attempt to cast the connected port to CC_CPUSidePort
        auto *cc_cpu_port = dynamic_cast<gem5::CC_BankedCache::CC_CPUSidePort *>(&connected_port);
        if (cc_cpu_port) {
            return cc_cpu_port->getCacheBlockSize();
        } else {
            fatal("Connected port is not a CC_CPUSidePort and does not support cache block size queries!");
        }
    } else {
        fatal("CC_MemSidePort: Port not connected to any peer!");
    }
}



bool
CC_Buffer::CC_MemSidePort::sendPacket(PacketPtr pkt)
{
    // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Sending packet: %s\n", pkt->print());

    // Send the packet
    // if (!sendTimingReq(pkt)) {
    //     // If unable to send, store the packet and wait for retry
    //     // blockedPacket = pkt;
    //     // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Packet blocked, waiting for retry.\n");
    // } else {
    //     // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Packet sent successfully.\n");
    // }

    bool success = sendTimingReq(pkt);
    
    if (!success) {
        DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Packet BLOCKED (sendPacket).\n");
    }
    
    return success;
}

bool
CC_Buffer::CC_MemSidePort::recvTimingResp(PacketPtr pkt)
{
    // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Received timing response: %s\n", pkt->print());

    // Addr addr = pkt->getAddr();


    CC_PacketState *cc_packet_state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    
    owner->memVerifyAddrSet.erase(cc_packet_state->uniqueInstSeqNum);

    // Process the response as needed
    // For now, we'll just delete the packet

    // Not deleting packet, because right now sending all packets - deleted elsewhere
    // delete pkt;

    // only receive cc_state packets
    delete pkt->senderState;
    delete pkt;

    return true;
}

void
CC_Buffer::CC_MemSidePort::recvReqRetry()
{
    // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Received request retry.\n");

    if (blockedPacket) {
        PacketPtr pkt = blockedPacket;
        blockedPacket = nullptr;

        if (!sendTimingReq(pkt)) {
            // If still blocked, keep the packet
            blockedPacket = pkt;
            // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Retry failed, still blocked.\n");
        } else {
            // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Retry successful, packet sent.\n");
        }
    }
}

void
CC_Buffer::CC_MemSidePort::recvRangeChange()
{
    // DPRINTF(CC_Buffer_Flag, "CC_MemSidePort: Received range change.\n");
    // For simplicity, we ignore range changes
}


} // namespace gem5
