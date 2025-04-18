// cc_inst.cc

#include "checker_chip/cc_inst.hh"
// #include "cpu/static_inst.hh"

// Constructor
CheckerInst::CheckerInst(unsigned long instDecodeCycle,
                         unsigned long instExecuteCycle,
                         unsigned long instTranslationCycle,
                         uint64_t uniqueInstSeqNum,
                         bool instInFU,
                        const gem5::StaticInstPtr &staticInst)
    : instDecodeCycle(instDecodeCycle),
      instExecuteCycle(instExecuteCycle),
      instTranslationCycle(instTranslationCycle),
      uniqueInstSeqNum(uniqueInstSeqNum),
      instInFU(instInFU),
      iVerify_bit(false),
      execVerify_bit(false),
      memVerify_bit(false),
      isWriteComplete_bit(false),
      staticInst(staticInst),
      functional_unit_index(-1) {}

CheckerInst::~CheckerInst() {
    // if (mem_access_data_ptr) {
    //     delete[] mem_access_data_ptr;
    //     mem_access_data_ptr = nullptr;
    // }
}

// getting staticInst
gem5::StaticInstPtr CheckerInst::getStaticInst() const {
    return staticInst;
}

bool CheckerInst::isReadInst() const {
  gem5::OpClass op_class = staticInst->opClass();
  return (op_class == gem5::MemReadOp || op_class == gem5::FloatMemReadOp);
}

bool CheckerInst::isWriteInst() const {
  gem5::OpClass op_class = staticInst->opClass();
  return (op_class == gem5::MemWriteOp || op_class == gem5::FloatMemWriteOp);
}

bool CheckerInst::isWriteCompleteInst() const {
  return isWriteComplete_bit;
}

void CheckerInst::setMemAddresses(gem5::Addr virt_addr, gem5::Addr phys_addr, unsigned mem_data_size, uint8_t* mem_data_pointer) {
  v_addr = virt_addr;
  p_addr = phys_addr;

  // if (mem_access_data_ptr) {
  //     delete[] mem_access_data_ptr;
  //     mem_access_data_ptr = nullptr;
  //     mem_access_data_size = 0;
  // }

  mem_access_data_size = mem_data_size;
  mem_access_data_ptr = new uint8_t[mem_data_size];
  memcpy(mem_access_data_ptr, mem_data_pointer, mem_data_size);

  // // Allocate memory to store a copy of the data
  //   if (storeData) {
  //       delete[] storeData; // Free previously allocated memory if necessary
  //   }
  //   storeData = new uint8_t[size];
  //   storeDataSize = size;

  //   // Copy the data
  //   memcpy(storeData, data, size);

  // // Allocate new memory and copy data
  // if (mem_data_size > 0 && mem_data_pointer) {
  //     mem_access_data_ptr = new uint8_t[mem_data_size];
  //     mem_access_data_size = mem_data_size;
  //     memcpy(mem_access_data_ptr, mem_data_pointer, mem_data_size);
  // }
}
