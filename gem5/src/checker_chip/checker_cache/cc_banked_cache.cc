// cc_banked_cache.cc

#include "checker_chip/checker_cache/cc_banked_cache.hh"
#include "base/logging.hh"
#include "debug/CC_BankedCache.hh"
#include "sim/system.hh"

namespace gem5
{

CC_BankedCache::CC_BankedCache(const CC_BankedCacheParams &p)
    : Cache(p),
      numBanks(p.num_banks)//,
    //   cc_cpu_port(name() + ".cc_cpu_port", /*id*/ 0, this)
{
    // Since the CPU side ports are a vector of ports, create an instance of
    // the CPUSidePort for each connection. This member of params is
    // automatically created depending on the name of the vector port and
    // holds the number of connections to this port name
    for (int i = 0; i < 8; ++i) {
        cc_cpu_port.emplace_back(name() + csprintf(".cc_cpu_port[%d]", i), i, this);
    }
}

unsigned
CC_BankedCache::calculateBankId(Addr addr) const
{
    // Simple modulo-based bank mapping
    return (addr / blkSize) % numBanks;
}

bool
CC_BankedCache::access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,
                       PacketList &writebacks)
{
    // Determine the bank ID based on the address
    unsigned bankId = calculateBankId(pkt->getAddr());

    // Print bank ID
    // DPRINTF(CC_BankedCache, "Accessing bank ID: %u for address: %#x\n", bankId, pkt->getAddr());

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
    DPRINTF(CC_BankedCache, "Received timing request: %s\n", pkt->print());

    // Drop the packet after logging
    delete pkt;

    // Always accept the packet without the need for retries
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

} // namespace gem5
