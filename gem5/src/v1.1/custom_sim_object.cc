#include "v1.1/custom_sim_object.hh"
#include "base/trace.hh"
#include "debug/CustomSimObjectFlag.hh"

#include <iostream>

namespace gem5
{

CustomSimObject::CustomSimObject(const CustomSimObjectParams &params) :
    // SimObject(params) - this is the base class constructor
    // event([this]{processEvent();}, name()) - [this] is a lambda capture list, {processEvent();} is the lambda body, name() is the name of the SimObject
    SimObject(params), 
    event([this]{processEvent();}, name()),
    latency(100), // custom defined parameter, passing in a value here
    timesLeft(10)
{
    DPRINTF(CustomSimObjectFlag, "Created the customSimObject\n");
}

// for the event to be processed, we first have to schedule the event. 
// The startup() function is where SimObjects are allowed to schedule internal events.
// does not get executed until the simulation begins for the first time (i.e. the simulate() function is called from a Python config file).
void
CustomSimObject::startup()
{
    // we simply schedule the event to execute at tick 100. Normally, you would use some offset from curTick(), but since we know the startup() function is called when the time is currently 0, we can use an explicit tick value.
    schedule(event, 100);

}

void
CustomSimObject::processEvent()
{
    timesLeft--;
    DPRINTF(CustomSimObjectFlag, "Firing event from within custom sim object. Event counter: %d\n", timesLeft);

    if (timesLeft <= 0) {
        DPRINTF(CustomSimObjectFlag, "Done firing!\n");
    } else {
        schedule(event, curTick() + latency);
    }
}

} // namespace gem5