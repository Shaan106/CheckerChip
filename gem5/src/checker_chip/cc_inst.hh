// cc_inst.hh

#ifndef cc_inst_HH
#define cc_inst_HH

#include <string>

#include "cpu/static_inst.hh"

#include "base/types.hh"

#include "cpu/op_class.hh"

#include <cstdint>

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

    //storing v_addr and p_addr if inst is a ld/st
    gem5::Addr v_addr;
    gem5::Addr p_addr;
    unsigned mem_access_data_size;
    uint8_t* mem_access_data_ptr;

    // unique identifier for inst
    uint64_t uniqueInstSeqNum;

    //bool to check whether inst has been assigned a functional unit
    bool instInFU;

    //bool to check if Iword was verified
    bool iVerify_bit;

    //bool to check if exec was verified
    bool execVerify_bit;

    //bool to check if mem was verified
    bool memVerify_bit;

    //bool to check if write complete inst
    bool isWriteComplete_bit;

    //which functional unit is executing this instruction, used to free fu when done
    int functional_unit_index; 

    // Constructor
    CheckerInst(unsigned long instDecodeCycle,
                unsigned long instExecuteCycle,
                unsigned long instTranslationCycle,
                uint64_t uniqueInstSeqNum,
                bool instInFU,
                const gem5::StaticInstPtr &staticInst);

    // Destructor
    ~CheckerInst();

    // Getter for staticInst
    gem5::StaticInstPtr getStaticInst() const;

    // return true if load inst
    bool isReadInst() const;

    // return true if store inst
    bool isWriteInst() const;

    // return true if write complete inst
    bool isWriteCompleteInst() const;

    // function to set v_addr and p_addr if inst is mem_inst
    void setMemAddresses(gem5::Addr virt_addr, 
                         gem5::Addr phys_addr,
                         unsigned mem_data_size,
                         uint8_t* mem_data_pointer);
};

#endif // cc_inst_HH
