// cc_inst.cc

#include "checker_chip/cc_inst.hh"
// #include "cpu/static_inst.hh"

// Constructor
CheckerInst::CheckerInst(unsigned long clockPeriodTicks, 
                         int timeUntilDecode,
                         int timeUntilExecute,
                        const gem5::StaticInstPtr &staticInst)
    : clockPeriodTicks(clockPeriodTicks), 
      timeUntilDecode(timeUntilDecode),
      timeUntilExecute(timeUntilExecute),
      staticInst(staticInst) {}

// Getter for number
unsigned long CheckerInst::getClockPeriodTicks() const {
    return clockPeriodTicks;
}

// getting staticInst
gem5::StaticInstPtr CheckerInst::getStaticInst() const {
    return staticInst;
}

// decrement timers
void CheckerInst::decrementTimers() {
    timeUntilDecode = timeUntilDecode - 1;
    
    timeUntilExecute = timeUntilExecute - 1;
}
