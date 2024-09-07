#ifndef __CHECKER_CUSTOM_SIM_OBJECT_HH__
#define __CHECKER_CUSTOM_SIM_OBJECT_HH__

#include <string>

#include "params/CustomSimObject.hh"
#include "sim/sim_object.hh"
#include "v1.1/goodbye_object.hh"

namespace gem5
{

class CustomSimObject : public SimObject
{

  private:
    void processEvent();

    EventFunctionWrapper event;

    /// Pointer to the corresponding RecieverSimObject. Set via Python
    GoodbyeObject* goodbye;

    const std::string myName;

    const Tick latency;

    int timesLeft;
  
  public:
    CustomSimObject(const CustomSimObjectParams &p);

    void startup() override;
};

} // namespace gem5

#endif // __CHECKER_CUSTOM_SIM_OBJECT_HH__