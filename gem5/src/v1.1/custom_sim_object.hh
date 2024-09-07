#ifndef __CHECKER_CUSTOM_SIM_OBJECT_HH__
#define __CHECKER_CUSTOM_SIM_OBJECT_HH__

#include "params/CustomSimObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class CustomSimObject : public SimObject
{

  private:
    void processEvent();

    EventFunctionWrapper event;

    const Tick latency;

    int timesLeft;
  
  public:
    CustomSimObject(const CustomSimObjectParams &params);

    void startup() override;
};

} // namespace gem5

#endif // __CHECKER_CUSTOM_SIM_OBJECT_HH__