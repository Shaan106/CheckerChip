// cc_inst.hh

#ifndef cc_inst_HH
#define cc_inst_HH

#include <string>

class CheckerInst {
private:
    // Private member variables
    int number;

public:
    // Constructor
    CheckerInst(int num);

    // Getter for number
    int getNumber() const;

    // Setter for number
    void setNumber(int num);
};

#endif // cc_inst_HH
