// cc_banked_cache.cc

#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "base/logging.hh"
#include "debug/CC_BankedCache.hh"
#include "sim/system.hh"

#include "checker_chip/checker_cache/checker_packet_state.hh"

namespace gem5
{

CC_BankedCache::CC_BankedCache(const CC_BankedCacheParams &p)
    : Cache(p),
      numBanks(p.num_banks)//,
    //   freeBankClockEvent([this] { freeBank(); }, name() + ".freeBankClockEvent"), 
    //   cc_cpu_port(name() + ".cc_cpu_port", /*id*/ 0, this)
{
    // Since the CPU side ports are a vector of ports, create an instance of
    // the CPUSidePort for each connection. This member of params is
    // automatically created depending on the name of the vector port and
    // holds the number of connections to this port name
    for (int i = 0; i < 8; ++i) {
        cc_cpu_port.emplace_back(name() + csprintf(".cc_cpu_port[%d]", i), i, this);
    }

    // Initialize bankFreeList with all entries set to true
    bankFreeList = std::vector<bool>(numBanks, true);
}

void
CC_BankedCache::freeBank(unsigned bankID)
{
    // Mark the bank as free
    bankFreeList[bankID] = true;

    // Debug output
    DPRINTF(CC_BankedCache, "Bank %u is now free.\n", bankID);
}

unsigned
CC_BankedCache::calculateBankId(Addr addr)
{
    // Simple modulo-based bank mapping
    unsigned bankID = (addr / blkSize) % numBanks;

    bankFreeList[bankID] = false;

    // Schedule the bank to be free after 20 cycles
    // Schedule the bank to be free after 20 cycles
    schedule(new EventFunctionWrapper(
        [this, bankID]() { freeBank(bankID); }, name() + ".freeBank", true), 
        curTick() + 20 * clockPeriod());

    DPRINTF(CC_BankedCache, "bankFreeList: \n");
    for (size_t i = 0; i < bankFreeList.size(); ++i) {
        DPRINTF(CC_BankedCache, "[%d] = %s ", i, bankFreeList[i] ? "true" : "false");
    }
    DPRINTF(CC_BankedCache, "\n");

    return bankID;
}

bool
CC_BankedCache::access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,
                       PacketList &writebacks)
{
    // Determine the bank ID based on the address
    unsigned bankId = calculateBankId(pkt->getAddr());

    // Print bank ID
    DPRINTF(CC_BankedCache, "Accessing bank ID: %u for address: %#x\n", bankId, pkt->getAddr());

    return Cache::access(pkt, blk, lat, writebacks);
}

Port &
CC_BankedCache::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "cc_cpu_port" && idx < cc_cpu_port.size()) {
        return cc_cpu_port[idx];
    } else {
        // If the port name doesn't match, defer to the base class
        return Cache::getPort(if_name, idx);
    }
}

/////////////////
//
// CC_CPUSidePort Implementation
//
/////////////////

void
CC_BankedCache::CC_CPUSidePort::sendPacket(PacketPtr pkt)
{
    if (!sendTimingResp(pkt)) {
        // If unable to send, store the packet and set needRetry
        blockedPacket = pkt;
        needRetry = true;
    }
}

AddrRangeList
CC_BankedCache::CC_CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void
CC_BankedCache::CC_CPUSidePort::trySendRetry()
{
    if (needRetry) {
        needRetry = false;
        sendRetryReq();
    }
}

void
CC_BankedCache::CC_CPUSidePort::recvFunctional(PacketPtr pkt)
{
    DPRINTF(CC_BankedCache, "Received functional request: %s\n", pkt->print());
    // For simplicity, we're not performing any functional access.
}

bool
CC_BankedCache::CC_CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    // Log receipt of the packet
    Cycles current_cycle = owner->curCycle();

    DPRINTF(CC_BankedCache, "Received timing request at cycle %lu: %s\n",
            current_cycle, pkt->print());

    // Determine the bank ID based on the address
    unsigned bankId = owner->calculateBankId(pkt->getAddr());

    DPRINTF(CC_BankedCache, "Associated bank: %u\n", bankId);

    pkt->senderState = new CC_PacketState(42, "CustomTag");

    // // Prepare variables for cache access
    // CacheBlk *blk = nullptr;         // Pointer to cache block, initially null
    // Cycles lat = Cycles(0);          // Latency initialized to 0 cycles
    // PacketList writebacks;           // List to collect writebacks

    // // Call the `access` function
    // bool hit = owner->access(pkt, blk, lat, writebacks);

    // // Log access results
    // DPRINTF(CC_BankedCache, "Cache access result: %s\n", hit ? "Hit" : "Miss");
    // DPRINTF(CC_BankedCache, "Latency: %u cycles\n", lat);

    // // If there are writebacks, log them
    // if (!writebacks.empty()) {
    //     DPRINTF(CC_BankedCache, "Writebacks during access:\n");
    //     for (const auto& wb_pkt : writebacks) {
    //         DPRINTF(CC_BankedCache, "  Writeback packet: %s\n", wb_pkt->print());
    //     }
    // }

    // Handle writes
    // if (pkt->isWrite()) {
    //     uint8_t* data = pkt->getPtr<uint8_t>(); // Pointer to the data being written
    //     unsigned size = pkt->getSize();        // Size of the data

    //     if (data && size > 0) {
    //         DPRINTF(CC_BankedCache, "WriteReq Data (size: %u): \n", size);
    //         for (unsigned i = 0; i < size; ++i) {
    //             DPRINTF(CC_BankedCache, "%02x \n", data[i]); // Print each byte in hex
    //         }
    //         DPRINTF(CC_BankedCache, "\n");
    //     } else {
    //         DPRINTF(CC_BankedCache, "WriteReq with no data or zero size.\n");
    //     }
    // }

    // Create and send a dummy response if the request needs a response
    if (pkt->needsResponse()) {
        // Create a dummy response
        // owner->createAndSendDummyResponse(pkt);
    }

    // Access the custom state
    CC_PacketState *state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    if (state) {
        DPRINTF(CC_BankedCache, "Custom Info: %d | Tag: %s\n", state->customInfo, state->tag.c_str());
    } else {
        DPRINTF(CC_BankedCache, "No Custom State found for Packet ID: %lu\n", pkt->id);
    }

    // // Drop the packet after processing
    // delete pkt->senderState;
    // delete pkt;

    // Accept the packet without retries
    owner->recvTimingReq(pkt);

    return true;
}


void
CC_BankedCache::CC_CPUSidePort::recvRespRetry()
{
    DPRINTF(CC_BankedCache, "Received response retry\n");

    if (blockedPacket) {
        PacketPtr pkt = blockedPacket;
        blockedPacket = nullptr;
        needRetry = false;

        if (!sendTimingResp(pkt)) {
            // If still blocked, keep the packet and set needRetry
            blockedPacket = pkt;
            needRetry = true;
        }
    }
}

void
CC_BankedCache::CC_CPUSidePort::createAndSendDummyResponse(PacketPtr pkt)
{
    // Create the response packet
    // PacketPtr pkt = pkt->makeResponse();
    // pkt->makeResponse();

    // Set additional fields if needed (e.g., set data for reads)
    // if (pkt->isRead()) {
    //     // // For a dummy response, you might set the data to zero or some default value
    //     // uint8_t *data = new uint8_t[pkt->getSize()];
    //     // std::memset(data, 0, pkt->getSize());
    //     // pkt->setData(data);
    //     // // Remember to delete the data when the packet is deleted
    //     // // Use a custom deleter to free the data when the packet is destroyed
    //     // pkt->setDataDeleter([](uint8_t *data) { delete[] data; });
    //     pkt->setData(nullptr);
    // }

    DPRINTF(CC_BankedCache, "Sending dummy response at cycle %lu: %s\n",
            owner->curCycle(), pkt->print());

    // Send the response packet
    if (!owner->memSidePort.sendTimingReq(pkt)) {
        // If unable to send, handle the blocked packet
        blockedPacket = pkt;
        needRetry = true;
        DPRINTF(CC_BankedCache, "Response blocked, storing packet for retry.\n");
    }
}


} // namespace gem5
