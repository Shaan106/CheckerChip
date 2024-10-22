#include "checker_chip/cc_regfile.hh"


CheckerRegfile::CheckerRegfile(
    unsigned long* clk, 
    unsigned int latency, 
    unsigned int bandwidth
) : 
    clk(clk), 
    latency(latency), 
    bandwidth(bandwidth), 
    latencyPointer(0), 
    instsStaged(0),
    instructionsInRegfileArray(latency, 0) 
    {}


void CheckerRegfile::stageInstToRegfile() {
    instsStaged++;
}

// push n update, return how many freed for stats.
unsigned int CheckerRegfile::updateRegfile() {

    //how may freed
    unsigned int numInstsFreed = instructionsInRegfileArray[latencyPointer];

    //put new insts that will be freed in latency cycles
    instructionsInRegfileArray[latencyPointer] = instsStaged;

    // increase latency pointer
    latencyPointer = (latencyPointer + 1) % latency; // could make this much more efficient if power of 2

    // clear instsStaged so that no more can be added
    instsStaged = 0;
    
    return numInstsFreed;
}

bool CheckerRegfile::isBandwidthFull() {
    return (instsStaged >= bandwidth);
}

unsigned int CheckerRegfile::getBandwidth() {
    return bandwidth;
}

unsigned int CheckerRegfile::getLatency() {
    return latency;
}
