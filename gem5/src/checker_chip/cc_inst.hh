// cc_inst.hh

#ifndef cc_inst_HH
#define cc_inst_HH

#include <string>

#include "cpu/static_inst.hh"

class CheckerInst {
private:
    // Private member variables
    // unsigned long clockPeriodTicks;

    gem5::StaticInstPtr staticInst;

public:

    // cycle counters to see when the instructions can be executed.
    unsigned long instDecodeCycle;
    unsigned long instExecuteCycle;
    unsigned long instTranslationCycle;

    //bool to check whether inst has been assigned a functional unit
    bool instInFU;

    //bool to check if Iword was verified
    bool iVerify_bit;

    //bool to check if exec was verified
    bool execVerify_bit;

    //which functional unit is executing this instruction, used to free fu when done
    int functional_unit_index; 

    // Constructor
    CheckerInst(unsigned long instDecodeCycle,
                unsigned long instExecuteCycle,
                unsigned long instTranslationCycle,
                bool instInFU,
                const gem5::StaticInstPtr &staticInst);

    // Getter for staticInst
    gem5::StaticInstPtr getStaticInst() const;
};

#endif // cc_inst_HH
