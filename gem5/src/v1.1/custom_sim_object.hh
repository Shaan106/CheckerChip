#ifndef __CHECKER_CUSTOM_SIM_OBJECT_HH__
#define __CHECKER_CUSTOM_SIM_OBJECT_HH__

#include "params/CustomSimObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class CustomSimObject : public SimObject
{
  public:
    CustomSimObject(const CustomSimObjectParams &params);
};

} // namespace gem5

#endif // __CHECKER_CUSTOM_SIM_OBJECT_HH__