// cc_inst.cc

#include "checker_chip/cc_inst.hh"
// #include "cpu/static_inst.hh"

// Constructor
CheckerInst::CheckerInst(unsigned long instDecodeCycle,
                         unsigned long instExecuteCycle,
                         unsigned long instTranslationCycle,
                         bool instInFU,
                        const gem5::StaticInstPtr &staticInst)
    : instDecodeCycle(instDecodeCycle),
      instExecuteCycle(instExecuteCycle),
      instTranslationCycle(instTranslationCycle),
      instInFU(instInFU),
      iVerify_bit(false),
      execVerify_bit(false),
      staticInst(staticInst),
      functional_unit_index(-1) {}


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

void CheckerInst::setMemAddresses(gem5::Addr virt_addr, gem5::Addr phys_addr, unsigned mem_data_size, uint8_t* mem_data_pointer) {
  v_addr = virt_addr;
  p_addr = phys_addr;

  if (mem_access_data_ptr) {
      delete[] mem_access_data_ptr;
      mem_access_data_ptr = nullptr;
      mem_access_data_size = 0;
  }

  // Allocate new memory and copy data
  if (mem_data_size > 0 && mem_data_pointer) {
      mem_access_data_ptr = new uint8_t[mem_data_size];
      mem_access_data_size = mem_data_size;
      memcpy(mem_access_data_ptr, mem_data_pointer, mem_data_size);
  }
}
