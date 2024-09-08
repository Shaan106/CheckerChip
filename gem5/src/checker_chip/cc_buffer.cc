
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "base/logging.hh"
#include "debug/CC_Buffer_Flag.hh"
// #include "debug/Commit.hh"

#include "sim/sim_exit.hh"

#include <iostream>

namespace gem5
{

/*
Constructor for the GoodbyeObject. 
*/
CC_Buffer::CC_Buffer(const CC_BufferParams &params) :
    SimObject(params), 
    event([this]{ processEvent(); }, name() + ".event")
{
    std::cout << "created the buffer object" << std::endl;
}

CC_Buffer::~CC_Buffer()
{
    // DPRINTF(CC_Buffer_Flag, "Destructor called\n");
}

void
CC_Buffer::processEvent()
{
    std::cout << "hi again..." << std::endl;

}

void
CC_Buffer::pushCommit(const std::string &instName)
{
    // std::cout << "hi" << std::endl;


    DPRINTF(CC_Buffer_Flag, "Debug statement??\n");

    DPRINTF(CC_Buffer_Flag, "instName: %s\n", instName.c_str());


    // schedule(event, curTick() + 1000);


    // message = "Goodbye " + other_name + "!! ";

    linkedFunc();
}

void 
CC_Buffer::linkedFunc()
{
    // std::cout << "linkedFunc called" << std::endl;
    DPRINTF(CC_Buffer_Flag, "debug statement from linked func\n");
}


} // namespace gem5
