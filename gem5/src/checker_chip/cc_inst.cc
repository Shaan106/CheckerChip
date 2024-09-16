// cc_inst.cc

#include "checker_chip/cc_inst.hh"
// #include "cpu/static_inst.hh"

// Constructor
CheckerInst::CheckerInst(int num, const gem5::StaticInstPtr &staticInst)
    : number(num), staticInst(staticInst) {}

// Getter for number
int CheckerInst::getNumber() const {
    return number;
}

// Setter for number
void CheckerInst::setNumber(int num) {
    number = num;
}

// getting staticInst
gem5::StaticInstPtr CheckerInst::getStaticInst() const {
    return staticInst;
}
