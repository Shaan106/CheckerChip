// cc_inst.cc

#include "checker_chip/cc_inst.hh"
// #include "cpu/static_inst.hh"

// Constructor
CheckerInst::CheckerInst(int instDecodeCycle,
                         int instExecuteCycle,
                        const gem5::StaticInstPtr &staticInst)
    : instDecodeCycle(instDecodeCycle),
      instExecuteCycle(instExecuteCycle),
      staticInst(staticInst) {}


// getting staticInst
gem5::StaticInstPtr CheckerInst::getStaticInst() const {
    return staticInst;
}

// decrement timers
void CheckerInst::decrementTimers() {
    instDecodeCycle = instDecodeCycle - 1;
    
    instExecuteCycle = instExecuteCycle - 1;
}
