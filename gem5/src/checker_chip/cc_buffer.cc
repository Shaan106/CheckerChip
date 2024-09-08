
#include "checker_chip/cc_buffer.hh"

#include "base/trace.hh"
#include "debug/CC_Buffer_Flag.hh"
#include "sim/sim_exit.hh"

#include <iostream>

namespace gem5
{

/*
Constructor for the GoodbyeObject. 
*/
CC_Buffer::CC_Buffer(const CC_BufferParams &params) :
    SimObject(params), 
    event([this]{ processEvent(); }, name() + ".event") //,
    // bandwidth(params.write_bandwidth), 
    // bufferSize(params.buffer_size),
    // buffer(nullptr), 
    // bufferUsed(0)
{
    // buffer = new char[bufferSize]();
    DPRINTF(CC_Buffer_Flag, "Created the goodbye object\n");
}

CC_Buffer::~CC_Buffer()
{
    DPRINTF(CC_Buffer_Flag, "Destructor called\n");
}

void
CC_Buffer::processEvent()
{
    DPRINTF(CC_Buffer_Flag, "Processing (schedule event called)!\n");

    // Actually do the "work" of the event
    // fillBuffer();
}

void
CC_Buffer::pushCommit()
{
    DPRINTF(CC_Buffer_Flag, "Commit pushed to cc_buffer\n");

    // message = "Goodbye " + other_name + "!! ";
    std::cout << "hi" << std::endl;

    // Kick off the the first buffer fill. If it can't fill the whole buffer
    // because of a limited bandwidth, then this function will schedule another
    // event to finish the fill
    // fillBuffer();
}

// void
// CC_Buffer::fillBuffer()
// {
//     // There better be a message
//     assert(message.length() > 0);

//     // Copy from the message to the buffer per byte.
//     int bytes_copied = 0;
//     for (auto it = message.begin();
//          it < message.end() && bufferUsed < bufferSize - 1;
//          it++, bufferUsed++, bytes_copied++) {
//         // Copy the character into the buffer
//         buffer[bufferUsed] = *it;
//     }

//     if (bufferUsed < bufferSize - 1) {
//         // Wait for the next copy for as long as it would have taken
//         DPRINTF(CC_Buffer_Flag, "Scheduling another fillBuffer in %d ticks\n",
//                 bandwidth * bytes_copied);
//         schedule(event, curTick() + bandwidth * bytes_copied); //this calls the processEvent function
//     } else {
//         DPRINTF(CC_Buffer_Flag, "Goodbye done copying!\n");
//         // Be sure to take into account the time for the last bytes
//         exitSimLoop(buffer, 0, curTick() + bandwidth * bytes_copied);
//     }
// }

} // namespace gem5
