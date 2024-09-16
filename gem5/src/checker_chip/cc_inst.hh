// cc_inst.hh

#ifndef cc_inst_HH
#define cc_inst_HH

#include <string>

#include "cpu/static_inst.hh"

class CheckerInst {
private:
    // Private member variables
    int number;

    gem5::StaticInstPtr staticInst;

public:
    // Constructor
    CheckerInst(int num, const gem5::StaticInstPtr &staticInst);

    // Getter for number
    int getNumber() const;

    // Setter for number
    void setNumber(int num);

    // Getter for staticInst
    gem5::StaticInstPtr getStaticInst() const;
};

#endif // cc_inst_HH
