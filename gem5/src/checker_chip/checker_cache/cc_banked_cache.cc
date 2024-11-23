// cc_banked_cache.cc

#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "base/logging.hh"
#include "debug/CC_BankedCache.hh"
#include "sim/system.hh"

#include "checker_chip/checker_cache/checker_packet_state.hh"
#include "mem/cache/prefetch/base.hh"


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

    assert(pkt->isRequest());

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


// ---------------------- CACHE OVERRIDES --------------------------


void
CC_BankedCache::recvTimingReq(PacketPtr pkt)
{
    // DPRINTF(CacheTags, "%s tags:\n%s\n", __func__, tags->print());

    CC_PacketState *state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    if (state) {
        DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq| Custom Info: %d | Tag: %s\n", state->customInfo, state->tag.c_str());
        // Drop the packet after processing
        delete pkt->senderState;
        delete pkt;
        return;
    } else {
        DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq| No Custom State found for Packet ID: %lu\n", pkt->id);
    }

    promoteWholeLineWrites(pkt);

    if (pkt->cacheResponding()) {
        // a cache above us (but not where the packet came from) is
        // responding to the request, in other words it has the line
        // in Modified or Owned state
        DPRINTF(Cache, "Cache above responding to %s: not responding\n",
                pkt->print());

        // if the packet needs the block to be writable, and the cache
        // that has promised to respond (setting the cache responding
        // flag) is not providing writable (it is in Owned rather than
        // the Modified state), we know that there may be other Shared
        // copies in the system; go out and invalidate them all
        assert(pkt->needsWritable() && !pkt->responderHadWritable());

        // an upstream cache that had the line in Owned state
        // (dirty, but not writable), is responding and thus
        // transferring the dirty line from one branch of the
        // cache hierarchy to another

        // send out an express snoop and invalidate all other
        // copies (snooping a packet that needs writable is the
        // same as an invalidation), thus turning the Owned line
        // into a Modified line, note that we don't invalidate the
        // block in the current cache or any other cache on the
        // path to memory

        // create a downstream express snoop with cleared packet
        // flags, there is no need to allocate any data as the
        // packet is merely used to co-ordinate state transitions
        Packet *snoop_pkt = new Packet(pkt, true, false);

        // also reset the bus time that the original packet has
        // not yet paid for
        snoop_pkt->headerDelay = snoop_pkt->payloadDelay = 0;

        // make this an instantaneous express snoop, and let the
        // other caches in the system know that the another cache
        // is responding, because we have found the authorative
        // copy (Modified or Owned) that will supply the right
        // data
        snoop_pkt->setExpressSnoop();
        snoop_pkt->setCacheResponding();

        // this express snoop travels towards the memory, and at
        // every crossbar it is snooped upwards thus reaching
        // every cache in the system
        [[maybe_unused]] bool success = memSidePort.sendTimingReq(snoop_pkt);
        // express snoops always succeed
        assert(success);

        // main memory will delete the snoop packet

        // queue for deletion, as opposed to immediate deletion, as
        // the sending cache is still relying on the packet
        pendingDelete.reset(pkt);

        // no need to take any further action in this particular cache
        // as an upstram cache has already committed to responding,
        // and we have already sent out any express snoops in the
        // section above to ensure all other copies in the system are
        // invalidated
        return;
    }

    // anything that is merely forwarded pays for the forward latency and
    // the delay provided by the crossbar
    Tick forward_time = clockEdge(forwardLatency) + pkt->headerDelay;

    if (pkt->cmd == MemCmd::LockedRMWWriteReq) {
        // For LockedRMW accesses, we mark the block inaccessible after the
        // read (see below), to make sure no one gets in before the write.
        // Now that the write is here, mark it accessible again, so the
        // write will succeed.  LockedRMWReadReq brings the block in in
        // exclusive mode, so we know it was previously writable.
        CacheBlk *blk = tags->findBlock(pkt->getAddr(), pkt->isSecure());
        assert(blk && blk->isValid());
        assert(!blk->isSet(CacheBlk::WritableBit) &&
               !blk->isSet(CacheBlk::ReadableBit));
        blk->setCoherenceBits(CacheBlk::ReadableBit);
        blk->setCoherenceBits(CacheBlk::WritableBit);
    }

    Cycles lat;
    CacheBlk *blk = nullptr;
    bool satisfied = false;
    {
        PacketList writebacks;
        // Note that lat is passed by reference here. The function
        // access() will set the lat value.
        satisfied = access(pkt, blk, lat, writebacks);

        // After the evicted blocks are selected, they must be forwarded
        // to the write buffer to ensure they logically precede anything
        // happening below
        doWritebacks(writebacks, clockEdge(lat + forwardLatency));
    }

    // Here we charge the headerDelay that takes into account the latencies
    // of the bus, if the packet comes from it.
    // The latency charged is just the value set by the access() function.
    // In case of a hit we are neglecting response latency.
    // In case of a miss we are neglecting forward latency.
    Tick request_time = clockEdge(lat);
    // Here we reset the timing of the packet.
    pkt->headerDelay = pkt->payloadDelay = 0;

    if (satisfied) {
        // notify before anything else as later handleTimingReqHit might turn
        // the packet in a response
        ppHit->notify(CacheAccessProbeArg(pkt,accessor));

        if (prefetcher && blk && blk->wasPrefetched()) {
            DPRINTF(Cache, "Hit on prefetch for addr %#x (%s)\n",
                    pkt->getAddr(), pkt->isSecure() ? "s" : "ns");
            blk->clearPrefetched();
        }

        handleTimingReqHit(pkt, blk, request_time);
    } else {
        handleTimingReqMiss(pkt, blk, forward_time, request_time);

        ppMiss->notify(CacheAccessProbeArg(pkt,accessor));
    }

    if (prefetcher) {
        // track time of availability of next prefetch, if any
        Tick next_pf_time = std::max(
                            prefetcher->nextPrefetchReadyTime(), clockEdge());
        if (next_pf_time != MaxTick) {
            schedMemSideSendEvent(next_pf_time);
        }
    }
    
}

} // namespace gem5