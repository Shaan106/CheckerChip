#include "v1.1/custom_sim_object.hh"
#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CustomSimObjectFlag.hh"

// including connected sim object
// #include "v1.1/reciever_sim_object.hh"

// #include <iostream>

namespace gem5
{

CustomSimObject::CustomSimObject(const CustomSimObjectParams &params) :
    // SimObject(params) - this is the base class constructor
    // event([this]{processEvent();}, name()) - [this] is a lambda capture list, {processEvent();} is the lambda body, name() is the name of the SimObject
    SimObject(params), 
    event([this]{processEvent();}, name()),
    goodbye(params.goodbye_object), 
    myName(params.name),
    latency(params.time_to_wait), // this is a value passed in from the python config file
    timesLeft(params.number_of_fires)
{
    DPRINTF(CustomSimObjectFlag, "Created the CustomSimObject with the name %s\n", myName);
    panic_if(!goodbye, "CustomSimObject must have a non-null GoodbyeObject");
}

// for the event to be processed, we first have to schedule the event. 
// The startup() function is where SimObjects are allowed to schedule internal events.
// does not get executed until the simulation begins for the first time (i.e. the simulate() function is called from a Python config file).
void
CustomSimObject::startup()
{
    // we simply schedule the event to execute at tick 100. Normally, you would use some offset from curTick(), but since we know the startup() function is called when the time is currently 0, we can use an explicit tick value.
    schedule(event, latency);

}

void
CustomSimObject::processEvent()
{
    timesLeft--;
    DPRINTF(CustomSimObjectFlag, "Firing event from within custom sim object. Event counter: %d\n", timesLeft);

    if (timesLeft <= 0) {
        DPRINTF(CustomSimObjectFlag, "Done firing!\n");
        
        goodbye->sayGoodbye(myName); // calling the sayGoodbye function from the recieverSimObject (connected object)
    } else {
        schedule(event, curTick() + latency);
    }
}

} // namespace gem5