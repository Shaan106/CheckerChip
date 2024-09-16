// cc_inst.cc

#include "cc_inst.hh"

// Constructor
CheckerInst::CheckerInst(int num) : number(num) {}

// Getter for number
int CheckerInst::getNumber() const {
    return number;
}

// Setter for number
void CheckerInst::setNumber(int num) {
    number = num;
}
