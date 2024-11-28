// cc_banked_cache.cc

#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "base/logging.hh"
#include "debug/CC_BankedCache.hh"
#include "sim/system.hh"
#include "base/statistics.hh"
// #include <sstream>  // For std::stringstream

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

    // Initialize bank units
    bankUnits.resize(numBanks);

    bank_queue_occupancy_histogram_arr.resize(numBanks);

    // Initialize bankFreeList with all entries set to true
    bankFreeList = std::vector<bool>(numBanks, true);

    // bank_queue_occupancy_histogram_arr.reserve(numBanks);
}

void CC_BankedCache::regStats()
{
    Cache::regStats(); // Call base class regStats()

    for (size_t i = 0; i < numBanks; ++i) {
        bank_queue_occupancy_histogram_arr[i] = new statistics::Distribution;
        bank_queue_occupancy_histogram_arr[i]
            ->init(0, bankUnits[i].getMaxQueueSize(), 1)  // Initialize with min, max, and step
            .name(name() + ".bank" + std::to_string(i) + ".cc_buffer_test_bank_test")
            .desc("Distribution of bank queue for bank " + std::to_string(i))
            .flags(statistics::pdf | statistics::display);
    }
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

    // DPRINTF(CC_BankedCache, "bankFreeList: \n");
    // for (size_t i = 0; i < bankFreeList.size(); ++i) {
    //     DPRINTF(CC_BankedCache, "[%d] = %s \n", i, bankFreeList[i] ? "true" : "false");
    // }
    // DPRINTF(CC_BankedCache, "\n");

    return bankID;
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

unsigned CC_BankedCache::CC_CPUSidePort::getCacheBlockSize()
{
    return owner->blkSize;
}

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

    // Access the custom state
    CC_PacketState *state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    if (state) {
        DPRINTF(CC_BankedCache, "Custom Info: %d | Tag: %s\n", state->customInfo, state->tag.c_str());
    } else {
        DPRINTF(CC_BankedCache, "No Custom State found for Packet ID: %lu\n", pkt->id);
    }

    assert(pkt->isRequest());

    return owner->cc_cacheController(pkt);

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


// ---------------------- CUSTOM CACHE METHODS --------------------------

bool 
CC_BankedCache::cc_cacheController(PacketPtr pkt)
{

    unsigned bankId = calculateBankId(pkt->getAddr());

    bool bankUnitSuccess = bankUnits[bankId].addPacket(pkt);

    if (bankUnitSuccess) {
        DPRINTF(CC_BankedCache, "Packet added to bank %d\n", bankId);
    } else {
        DPRINTF(CC_BankedCache, "Failed to add packet to bank %d: Queue is full\n", bankId);
        // Handle the case where the packet could not be added
        // For example, you might need to stall or retry later
        // PacketPtr tmp1 = bankUnits[bankId].removePacket();
        // PacketPtr tmp2 = bankUnits[bankId].removePacket();
        // PacketPtr tmp3 = bankUnits[bankId].removePacket();
        // bool tmpSuccess = bankUnits[bankId].addPacket(pkt);
    }

    DPRINTF(CC_BankedCache, "Queue Size: %u\n", static_cast<unsigned>(bankUnits[bankId].getQueueSize()));

    // next clock cycle, cycle through queues to send packets to banks
    schedule(new EventFunctionWrapper(
    [this]() { cc_dispatchEvent(); }, name() + ".cc_dispatchEvent", true), 
    clockEdge(Cycles(10))); // TODO: latency from entry to dispatch to mem

    return bankUnitSuccess; // TODO: need to change this to only return true if we can add to a queue (leads to stalling in buffer)
}

void 
CC_BankedCache::cc_dispatchEvent() 
{
    // go over every bank and if not empty, send to bank to deal with req
    for (int i = 0; i < numBanks; ++i) {
        bank_queue_occupancy_histogram_arr[i]->sample(bankUnits[i].getQueueSize());
        if (!bankUnits[i].isEmpty()) {
            PacketPtr pkt = bankUnits[i].removePacket();
            recvTimingReq(pkt);
        }
        DPRINTF(CC_BankedCache, "cc_dispatchEvent queue %d Size: %u\n", i,  static_cast<unsigned>(bankUnits[i].getQueueSize()));
    }
    
}


// ---------------------- CACHE OVERRIDES --------------------------


void
CC_BankedCache::recvTimingReq(PacketPtr pkt)
{
    // DPRINTF(CacheTags, "%s tags:\n%s\n", __func__, tags->print());


    // bankUnits[bankId].printUnit(); // Print the bank's queue

    DPRINTF(CC_BankedCache, "[[[1]]] CC_BankedCache::recvTimingReq | Packet ID: %lu\n", pkt->id);

    DPRINTF(CC_BankedCache, "satisfyRequest: Condition: getOffset %d + getSize %d <= blkSize %d is %s\n",
        pkt->getOffset(blkSize), pkt->getSize(), blkSize,
        (pkt->getOffset(blkSize) + pkt->getSize() <= blkSize) ? "true" : "false");


    // Get the requestor ID (this identifies which core sent the request)
    int requestor_id = pkt->req->requestorId();

    CC_PacketState *cc_packet_state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    if (cc_packet_state) {
        DPRINTF(CC_BankedCache, "Received from queue dispatch\n");
        DPRINTF(CC_BankedCache, "Received request from CC core, global requestor ID: %d, core ID: %d\n", requestor_id, cc_packet_state->senderCoreID);
    } else {
        DPRINTF(CC_BankedCache, "Received request from normal core, global requestor ID: %d\n", requestor_id);
    }

    promoteWholeLineWrites(pkt);

    if (pkt->cacheResponding()) {
        // a cache above us (but not where the packet came from) is
        // responding to the request, in other words it has the line
        // in Modified or Owned state
        // DPRINTF(Cache, "Cache above responding to %s: not responding\n", pkt->print());

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

    // DPRINTF(CC_BankedCache, "[[[2]]] CC_BankedCache::recvTimingReq Cache | Packet ID: %lu\n", pkt->id);

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

    // CC_PacketState *state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    // if (state) {
    //     DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq| Custom Info: %d | Tag: %s\n", state->customInfo, state->tag.c_str());
    //     // Drop the packet after processing
    //     delete pkt->senderState;
    //     delete pkt;
    //     return;
    // } else {
    //     DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq| No Custom State found for Packet ID: %lu\n", pkt->id);
    // }    

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

    // CC_PacketState *state = dynamic_cast<CC_PacketState *>(pkt->senderState);
    // DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq| Custom Info: %d | Tag: %s\n", state->customInfo, state->tag.c_str());

    if (cc_packet_state) {
        DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq|\n");
    }
    DPRINTF(CC_BankedCache, "%s for %s\n", __func__, pkt->print());
    // Print the values after the block
    DPRINTF(CC_BankedCache, "Access Result: lat = %d, satisfied = %s, blk = %s\n",
        lat, satisfied ? "true" : "false", blk ? "valid" : "nullptr");

    DPRINTF(CC_BankedCache, "pkt->needsResponse(): %s\n", pkt->needsResponse() ? "true" : "false");
    // if (cc_packet_state) {
    //     // Drop the packet after processing
    //     delete pkt->senderState;
    //     delete pkt;
    //     return;
    // } else {
    //     DPRINTF(CC_BankedCache, "|CC_BankedCache::recvTimingReq| No Custom State found for Packet ID: %lu\n", pkt->id);
    // }    

    // Here we charge the headerDelay that takes into account the latencies
    // of the bus, if the packet comes from it.
    // The latency charged is just the value set by the access() function.
    // In case of a hit we are neglecting response latency.
    // In case of a miss we are neglecting forward latency.
    Tick request_time = clockEdge(lat);
    // Here we reset the timing of the packet.
    pkt->headerDelay = pkt->payloadDelay = 0;
    
    if (cc_packet_state) {
        // if it is a checker packet then

        if (satisfied) {
            // cache hit
            pkt->makeTimingResponse();
            cc_cpu_port[cc_packet_state->senderCoreID].sendPacket(pkt);
        } else {
            // cache miss
            pkt->makeTimingResponse();
            cc_cpu_port[cc_packet_state->senderCoreID].sendPacket(pkt);
        }

    } else {
    if (satisfied) {
        // notify before anything else as later handleTimingReqHit might turn
        // the packet in a response
        ppHit->notify(CacheAccessProbeArg(pkt,accessor));

        if (prefetcher && blk && blk->wasPrefetched()) {
            // DPRINTF(Cache, "Hit on prefetch for addr %#x (%s)\n", pkt->getAddr(), pkt->isSecure() ? "s" : "ns");
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
    
}

bool
CC_BankedCache::access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,
                       PacketList &writebacks)
{
    // Determine the bank ID based on the address
    unsigned bankId = calculateBankId(pkt->getAddr());

    // Print bank ID
    DPRINTF(CC_BankedCache, "Accessing bank ID: %u for address: %#x\n", bankId, pkt->getAddr());

    if (pkt->req->isUncacheable()) {
        assert(pkt->isRequest());

        gem5_assert(!(isReadOnly && pkt->isWrite()),
                    "Should never see a write in a read-only cache %s\n",
                    name());

        // DPRINTF(CC_BankedCache, "%s for %s\n", __func__, pkt->print());

        // flush and invalidate any existing block
        CacheBlk *old_blk(tags->findBlock(pkt->getAddr(), pkt->isSecure()));
        if (old_blk && old_blk->isValid()) {
            BaseCache::evictBlock(old_blk, writebacks);
        }

        blk = nullptr;
        // lookupLatency is the latency in case the request is uncacheable.
        lat = lookupLatency;
        return false;
    }

    // sanity check
    assert(pkt->isRequest());

    gem5_assert(!(isReadOnly && pkt->isWrite()),
                "Should never see a write in a read-only cache %s\n",
                name());

    // Access block in the tags
    Cycles tag_latency(0);
    blk = tags->accessBlock(pkt, tag_latency);

    DPRINTF(CC_BankedCache, "%s for %s %s\n", __func__, pkt->print(), blk ? "hit " + blk->print() : "miss");

    if (pkt->req->isCacheMaintenance()) {
        // A cache maintenance operation is always forwarded to the
        // memory below even if the block is found in dirty state.

        // We defer any changes to the state of the block until we
        // create and mark as in service the mshr for the downstream
        // packet.

        // Calculate access latency on top of when the packet arrives. This
        // takes into account the bus delay.
        lat = calculateTagOnlyLatency(pkt->headerDelay, tag_latency);

        return false;
    }

    if (pkt->isEviction()) {
        // We check for presence of block in above caches before issuing
        // Writeback or CleanEvict to write buffer. Therefore the only
        // possible cases can be of a CleanEvict packet coming from above
        // encountering a Writeback generated in this cache peer cache and
        // waiting in the write buffer. Cases of upper level peer caches
        // generating CleanEvict and Writeback or simply CleanEvict and
        // CleanEvict almost simultaneously will be caught by snoops sent out
        // by crossbar.
        WriteQueueEntry *wb_entry = writeBuffer.findMatch(pkt->getAddr(),
                                                          pkt->isSecure());
        if (wb_entry) {
            assert(wb_entry->getNumTargets() == 1);
            PacketPtr wbPkt = wb_entry->getTarget()->pkt;
            assert(wbPkt->isWriteback());

            if (pkt->isCleanEviction()) {
                // The CleanEvict and WritebackClean snoops into other
                // peer caches of the same level while traversing the
                // crossbar. If a copy of the block is found, the
                // packet is deleted in the crossbar. Hence, none of
                // the other upper level caches connected to this
                // cache have the block, so we can clear the
                // BLOCK_CACHED flag in the Writeback if set and
                // discard the CleanEvict by returning true.
                wbPkt->clearBlockCached();

                // A clean evict does not need to access the data array
                lat = calculateTagOnlyLatency(pkt->headerDelay, tag_latency);

                return true;
            } else {
                assert(pkt->cmd == MemCmd::WritebackDirty);
                // Dirty writeback from above trumps our clean
                // writeback... discard here
                // Note: markInService will remove entry from writeback buffer.
                markInService(wb_entry);
                delete wbPkt;
            }
        }
    }

    // The critical latency part of a write depends only on the tag access
    if (pkt->isWrite()) {
        lat = calculateTagOnlyLatency(pkt->headerDelay, tag_latency);
    }

    // Writeback handling is special case.  We can write the block into
    // the cache without having a writeable copy (or any copy at all).
    if (pkt->isWriteback()) {
        assert(blkSize == pkt->getSize());

        // we could get a clean writeback while we are having
        // outstanding accesses to a block, do the simple thing for
        // now and drop the clean writeback so that we do not upset
        // any ordering/decisions about ownership already taken
        if (pkt->cmd == MemCmd::WritebackClean &&
            mshrQueue.findMatch(pkt->getAddr(), pkt->isSecure())) {
            // DPRINTF(Cache, "Clean writeback %#llx to block with MSHR, dropping\n", pkt->getAddr());

            // A writeback searches for the block, then writes the data.
            // As the writeback is being dropped, the data is not touched,
            // and we just had to wait for the time to find a match in the
            // MSHR. As of now assume a mshr queue search takes as long as
            // a tag lookup for simplicity.
            return true;
        }

        const bool has_old_data = blk && blk->isValid();
        if (!blk) {
            // need to do a replacement
            blk = allocateBlock(pkt, writebacks);
            if (!blk) {
                // no replaceable block available: give up, fwd to next level.
                incMissCount(pkt);
                return false;
            }

            blk->setCoherenceBits(CacheBlk::ReadableBit);
        } else if (compressor) {
            // This is an overwrite to an existing block, therefore we need
            // to check for data expansion (i.e., block was compressed with
            // a smaller size, and now it doesn't fit the entry anymore).
            // If that is the case we might need to evict blocks.
            if (!updateCompressionData(blk, pkt->getConstPtr<uint64_t>(),
                writebacks)) {
                invalidateBlock(blk);
                return false;
            }
        }

        // only mark the block dirty if we got a writeback command,
        // and leave it as is for a clean writeback
        if (pkt->cmd == MemCmd::WritebackDirty) {
            // TODO: the coherent cache can assert that the dirty bit is set
            blk->setCoherenceBits(CacheBlk::DirtyBit);
        }
        // if the packet does not have sharers, it is passing
        // writable, and we got the writeback in Modified or Exclusive
        // state, if not we are in the Owned or Shared state
        if (!pkt->hasSharers()) {
            blk->setCoherenceBits(CacheBlk::WritableBit);
        }
        // nothing else to do; writeback doesn't expect response
        assert(!pkt->needsResponse());

        updateBlockData(blk, pkt, has_old_data);
        // DPRINTF(Cache, "%s new state is %s\n", __func__, blk->print());
        incHitCount(pkt);

        // When the packet metadata arrives, the tag lookup will be done while
        // the payload is arriving. Then the block will be ready to access as
        // soon as the fill is done
        blk->setWhenReady(clockEdge(fillLatency) + pkt->headerDelay +
            std::max(cyclesToTicks(tag_latency), (uint64_t)pkt->payloadDelay));

        return true;
    } else if (pkt->cmd == MemCmd::CleanEvict) {
        // A CleanEvict does not need to access the data array
        lat = calculateTagOnlyLatency(pkt->headerDelay, tag_latency);

        if (blk) {
            // Found the block in the tags, need to stop CleanEvict from
            // propagating further down the hierarchy. Returning true will
            // treat the CleanEvict like a satisfied write request and delete
            // it.
            return true;
        }
        // We didn't find the block here, propagate the CleanEvict further
        // down the memory hierarchy. Returning false will treat the CleanEvict
        // like a Writeback which could not find a replaceable block so has to
        // go to next level.
        return false;
    } else if (pkt->cmd == MemCmd::WriteClean) {
        // WriteClean handling is a special case. We can allocate a
        // block directly if it doesn't exist and we can update the
        // block immediately. The WriteClean transfers the ownership
        // of the block as well.
        assert(blkSize == pkt->getSize());

        const bool has_old_data = blk && blk->isValid();
        if (!blk) {
            if (pkt->writeThrough()) {
                // if this is a write through packet, we don't try to
                // allocate if the block is not present
                return false;
            } else {
                // a writeback that misses needs to allocate a new block
                blk = allocateBlock(pkt, writebacks);
                if (!blk) {
                    // no replaceable block available: give up, fwd to
                    // next level.
                    incMissCount(pkt);
                    return false;
                }

                blk->setCoherenceBits(CacheBlk::ReadableBit);
            }
        } else if (compressor) {
            // This is an overwrite to an existing block, therefore we need
            // to check for data expansion (i.e., block was compressed with
            // a smaller size, and now it doesn't fit the entry anymore).
            // If that is the case we might need to evict blocks.
            if (!updateCompressionData(blk, pkt->getConstPtr<uint64_t>(),
                writebacks)) {
                invalidateBlock(blk);
                return false;
            }
        }

        // at this point either this is a writeback or a write-through
        // write clean operation and the block is already in this
        // cache, we need to update the data and the block flags
        assert(blk);
        // TODO: the coherent cache can assert that the dirty bit is set
        if (!pkt->writeThrough()) {
            blk->setCoherenceBits(CacheBlk::DirtyBit);
        }
        // nothing else to do; writeback doesn't expect response
        assert(!pkt->needsResponse());

        updateBlockData(blk, pkt, has_old_data);
        // DPRINTF(Cache, "%s new state is %s\n", __func__, blk->print());

        incHitCount(pkt);

        // When the packet metadata arrives, the tag lookup will be done while
        // the payload is arriving. Then the block will be ready to access as
        // soon as the fill is done
        blk->setWhenReady(clockEdge(fillLatency) + pkt->headerDelay +
            std::max(cyclesToTicks(tag_latency), (uint64_t)pkt->payloadDelay));

        // If this a write-through packet it will be sent to cache below
        return !pkt->writeThrough();
    } else if (blk && (pkt->needsWritable() ?
            blk->isSet(CacheBlk::WritableBit) :
            blk->isSet(CacheBlk::ReadableBit))) {
        // OK to satisfy access
        incHitCount(pkt);

        // Calculate access latency based on the need to access the data array
        if (pkt->isRead()) {
            lat = calculateAccessLatency(blk, pkt->headerDelay, tag_latency);

            // When a block is compressed, it must first be decompressed
            // before being read. This adds to the access latency.
            if (compressor) {
                lat += compressor->getDecompressionLatency(blk);
            }
        } else {
            lat = calculateTagOnlyLatency(pkt->headerDelay, tag_latency);
        }

        satisfyRequest(pkt, blk);
        maintainClusivity(pkt->fromCache(), blk);

        return true;
    }

    // Can't satisfy access normally... either no block (blk == nullptr)
    // or have block but need writable

    incMissCount(pkt);

    lat = calculateAccessLatency(blk, pkt->headerDelay, tag_latency);

    if (!blk && pkt->isLLSC() && pkt->isWrite()) {
        // complete miss on store conditional... just give up now
        pkt->req->setExtraData(0);
        return true;
    }

    return false;
}


void
CC_BankedCache::handleTimingReqHit(PacketPtr pkt, CacheBlk *blk, Tick request_time)
{
    // should never be satisfying an uncacheable access as we
    // flush and invalidate any existing block as part of the
    // lookup
    assert(!pkt->req->isUncacheable());

    // handle special cases for LockedRMW transactions
    if (pkt->isLockedRMW()) {
        Addr blk_addr = pkt->getBlockAddr(blkSize);

        if (pkt->isRead()) {
            // Read hit for LockedRMW.  Since it requires exclusive
            // permissions, there should be no outstanding access.
            assert(!mshrQueue.findMatch(blk_addr, pkt->isSecure()));
            // The keys to LockedRMW are that (1) we always have an MSHR
            // allocated during the RMW interval to catch snoops and
            // defer them until after the RMW completes, and (2) we
            // clear permissions on the block to turn any upstream
            // access other than the matching write into a miss, causing
            // it to append to the MSHR as well.

            // Because we hit in the cache, we have to fake an MSHR to
            // achieve part (1).  If the read had missed, this MSHR
            // would get allocated as part of normal miss processing.
            // Basically we need to get the MSHR in the same state as if
            // we had missed and just received the response.
            // Request *req2 = new Request(*(pkt->req));
            RequestPtr req2 = std::make_shared<Request>(*(pkt->req));
            PacketPtr pkt2 = new Packet(req2, pkt->cmd);
            MSHR *mshr = allocateMissBuffer(pkt2, curTick(), true);
            // Mark the MSHR "in service" (even though it's not) to prevent
            // the cache from sending out a request.
            mshrQueue.markInService(mshr, false);
            // Part (2): mark block inaccessible
            assert(blk);
            blk->clearCoherenceBits(CacheBlk::ReadableBit);
            blk->clearCoherenceBits(CacheBlk::WritableBit);
        } else {
            assert(pkt->isWrite());
            // All LockedRMW writes come here, as they cannot miss.
            // Need to undo the two things described above.  Block
            // permissions were already restored earlier in this
            // function, prior to the access() call.  Now we just need
            // to clear out the MSHR.

            // Read should have already allocated MSHR.
            MSHR *mshr = mshrQueue.findMatch(blk_addr, pkt->isSecure());
            assert(mshr);
            // Fake up a packet and "respond" to the still-pending
            // LockedRMWRead, to process any pending targets and clear
            // out the MSHR
            PacketPtr resp_pkt =
                new Packet(pkt->req, MemCmd::LockedRMWWriteResp);
            resp_pkt->senderState = mshr;
            recvTimingResp(resp_pkt);
        }
    }

    if (pkt->needsResponse()) {
        // These delays should have been consumed by now
        assert(pkt->headerDelay == 0);
        assert(pkt->payloadDelay == 0);

        pkt->makeTimingResponse();

        // In this case we are considering request_time that takes
        // into account the delay of the xbar, if any, and just
        // lat, neglecting responseLatency, modelling hit latency
        // just as the value of lat overriden by access(), which calls
        // the calculateAccessLatency() function.
        cpuSidePort.schedTimingResp(pkt, request_time);
        // cc_cpu_port[0].schedTimingResp(pkt, request_time);
        
    } else {
        DPRINTF(CC_BankedCache, "%s satisfied %s, no response needed\n", __func__,
                pkt->print());

        // queue the packet for deletion, as the sending cache is
        // still relying on it; if the block is found in access(),
        // CleanEvict and Writeback messages will be deleted
        // here as well
        pendingDelete.reset(pkt);
    }
}

void
CC_BankedCache::handleTimingReqMiss(PacketPtr pkt, CacheBlk *blk, Tick forward_time,
                           Tick request_time)
{
    DPRINTF(CC_BankedCache, "In CC_BankedCache::handleTimingReqMiss\n");
    // These should always hit due to the earlier Locked Read
    assert(pkt->cmd != MemCmd::LockedRMWWriteReq);
    if (pkt->req->isUncacheable()) {
        // ignore any existing MSHR if we are dealing with an
        // uncacheable request

        // should have flushed and have no valid block
        assert(!blk || !blk->isValid());

        stats.cmdStats(pkt).mshrUncacheable[pkt->req->requestorId()]++;

        if (pkt->isWrite()) {
            allocateWriteBuffer(pkt, forward_time);
        } else {
            // uncacheable accesses always allocate a new MSHR

            // Here we are using forward_time, modelling the latency of
            // a miss (outbound) just as forwardLatency, neglecting the
            // lookupLatency component.

            // Here we allow allocating miss buffer for read requests
            // and x86's clflush requests. A clflush request should be
            // propagate through all levels of the cache system.

            // Doing clflush in uncacheable regions might sound contradictory;
            // however, it is entirely possible due to how the Linux kernel
            // handle page property changes. When a linux kernel wants to
            // change a page property, it flushes the related cache lines. The
            // kernel might change the page property before flushing the cache
            // lines. This results in the clflush might occur in an uncacheable
            // region, where the kernel marks a region uncacheable before
            // flushing. clflush results in a CleanInvalidReq.
            assert(pkt->isRead() || pkt->isCleanInvalidateRequest());
            allocateMissBuffer(pkt, forward_time);
        }

        return;
    }

    Addr blk_addr = pkt->getBlockAddr(blkSize);

    MSHR *mshr = mshrQueue.findMatch(blk_addr, pkt->isSecure());

    // Software prefetch handling:
    // To keep the core from waiting on data it won't look at
    // anyway, send back a response with dummy data. Miss handling
    // will continue asynchronously. Unfortunately, the core will
    // insist upon freeing original Packet/Request, so we have to
    // create a new pair with a different lifecycle. Note that this
    // processing happens before any MSHR munging on the behalf of
    // this request because this new Request will be the one stored
    // into the MSHRs, not the original.
    if (pkt->cmd.isSWPrefetch()) {
        assert(pkt->needsResponse());
        assert(pkt->req->hasPaddr());
        assert(!pkt->req->isUncacheable());

        // There's no reason to add a prefetch as an additional target
        // to an existing MSHR. If an outstanding request is already
        // in progress, there is nothing for the prefetch to do.
        // If this is the case, we don't even create a request at all.
        PacketPtr pf = nullptr;

        if (!mshr) {
            // copy the request and create a new SoftPFReq packet
            RequestPtr req = std::make_shared<Request>(pkt->req->getPaddr(),
                                                    pkt->req->getSize(),
                                                    pkt->req->getFlags(),
                                                    pkt->req->requestorId());
            pf = new Packet(req, pkt->cmd);
            pf->allocate();
            assert(pf->matchAddr(pkt));
            assert(pf->getSize() == pkt->getSize());
        }

        pkt->makeTimingResponse();

        // request_time is used here, taking into account lat and the delay
        // charged if the packet comes from the xbar.
        cpuSidePort.schedTimingResp(pkt, request_time);

        // If an outstanding request is in progress (we found an
        // MSHR) this is set to null
        pkt = pf;
    }

    if (writeAllocator &&
        pkt && pkt->isWrite() && !pkt->req->isUncacheable()) {
        writeAllocator->updateMode(pkt->getAddr(), pkt->getSize(),
                                   pkt->getBlockAddr(blkSize));
    }

    if (mshr) {
        /// MSHR hit
        /// @note writebacks will be checked in getNextMSHR()
        /// for any conflicting requests to the same block

        //@todo remove hw_pf here

        // Coalesce unless it was a software prefetch (see above).
        if (pkt) {
            assert(!pkt->isWriteback());
            // CleanEvicts corresponding to blocks which have
            // outstanding requests in MSHRs are simply sunk here
            if (pkt->cmd == MemCmd::CleanEvict) {
                pendingDelete.reset(pkt);
            } else if (pkt->cmd == MemCmd::WriteClean) {
                // A WriteClean should never coalesce with any
                // outstanding cache maintenance requests.

                // We use forward_time here because there is an
                // uncached memory write, forwarded to WriteBuffer.
                allocateWriteBuffer(pkt, forward_time);
            } else {
                DPRINTF(CC_BankedCache, "%s coalescing MSHR for %s\n", __func__,
                        pkt->print());

                assert(pkt->req->requestorId() < system->maxRequestors());
                stats.cmdStats(pkt).mshrHits[pkt->req->requestorId()]++;

                // We use forward_time here because it is the same
                // considering new targets. We have multiple
                // requests for the same address here. It
                // specifies the latency to allocate an internal
                // buffer and to schedule an event to the queued
                // port and also takes into account the additional
                // delay of the xbar.
                mshr->allocateTarget(pkt, forward_time, order++,
                                     allocOnFill(pkt->cmd));
                if (mshr->getNumTargets() >= numTarget) {
                    noTargetMSHR = mshr;
                    setBlocked(Blocked_NoTargets);
                    // need to be careful with this... if this mshr isn't
                    // ready yet (i.e. time > curTick()), we don't want to
                    // move it ahead of mshrs that are ready
                    // mshrQueue.moveToFront(mshr);
                }
            }
        }
    } else {
        // no MSHR
        assert(pkt->req->requestorId() < system->maxRequestors());
        stats.cmdStats(pkt).mshrMisses[pkt->req->requestorId()]++;
        if (prefetcher && pkt->isDemand())
            prefetcher->incrDemandMhsrMisses();

        if (pkt->isEviction() || pkt->cmd == MemCmd::WriteClean) {
            // We use forward_time here because there is an
            // writeback or writeclean, forwarded to WriteBuffer.
            allocateWriteBuffer(pkt, forward_time);
        } else {
            if (blk && blk->isValid()) {
                // If we have a write miss to a valid block, we
                // need to mark the block non-readable.  Otherwise
                // if we allow reads while there's an outstanding
                // write miss, the read could return stale data
                // out of the cache block... a more aggressive
                // system could detect the overlap (if any) and
                // forward data out of the MSHRs, but we don't do
                // that yet.  Note that we do need to leave the
                // block valid so that it stays in the cache, in
                // case we get an upgrade response (and hence no
                // new data) when the write miss completes.
                // As long as CPUs do proper store/load forwarding
                // internally, and have a sufficiently weak memory
                // model, this is probably unnecessary, but at some
                // point it must have seemed like we needed it...
                assert((pkt->needsWritable() &&
                    !blk->isSet(CacheBlk::WritableBit)) ||
                    pkt->req->isCacheMaintenance());
                blk->clearCoherenceBits(CacheBlk::ReadableBit);
            }
            // Here we are using forward_time, modelling the latency of
            // a miss (outbound) just as forwardLatency, neglecting the
            // lookupLatency component.
            allocateMissBuffer(pkt, forward_time);
        }
    }
}

} // namespace gem5