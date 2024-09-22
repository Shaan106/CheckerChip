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
    int instDecodeCycle;
    int instExecuteCycle;

    // Constructor
    CheckerInst(int instDecodeCycle,
                int instExecuteCycle,
                const gem5::StaticInstPtr &staticInst);

    // Getter for staticInst
    gem5::StaticInstPtr getStaticInst() const;

    // reducing timers
    void decrementTimers();
};

#endif // cc_inst_HH
