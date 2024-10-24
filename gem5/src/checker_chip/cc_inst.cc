// cc_inst.cc

#include "checker_chip/cc_inst.hh"
// #include "cpu/static_inst.hh"

// Constructor
CheckerInst::CheckerInst(int instDecodeCycle,
                         int instExecuteCycle,
                         bool instInFU,
                        const gem5::StaticInstPtr &staticInst)
    : instDecodeCycle(instDecodeCycle),
      instExecuteCycle(instExecuteCycle),
      instInFU(instInFU),
      iVerify_bit(false),
      execVerify_bit(false),
      staticInst(staticInst),
      functional_unit_index(-1) {}


// getting staticInst
gem5::StaticInstPtr CheckerInst::getStaticInst() const {
    return staticInst;
}
