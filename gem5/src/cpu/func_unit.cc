/*
 * Copyright (c) 2002-2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cpu/func_unit.hh"

#include <sstream>

#include "base/logging.hh"

// #include <iostream> // for debugging

namespace gem5
{

////////////////////////////////////////////////////////////////////////////
//
//  The funciton unit
//
FuncUnit::FuncUnit()
{
    opLatencies.fill(0);
    pipelined.fill(false);
    capabilityList.reset();
}


//  Copy constructor
FuncUnit::FuncUnit(const FuncUnit &fu)
{

    for (int i = 0; i < Num_OpClasses; ++i) {
        opLatencies[i] = fu.opLatencies[i];
        pipelined[i] = fu.pipelined[i];
    }

    capabilityList = fu.capabilityList;
}


void
FuncUnit::addCapability(OpClass cap, unsigned oplat, bool pipeline)
{
    if (oplat == 0)
        panic("FuncUnit:  you don't really want a zero-cycle latency do you?");

    capabilityList.set(cap);

    opLatencies[cap] = oplat;
    pipelined[cap] = pipeline;
}

bool
FuncUnit::provides(OpClass capability)
{
    return capabilityList[capability];
}

std::bitset<Num_OpClasses>
FuncUnit::capabilities()
{
    return capabilityList;
}

unsigned &
FuncUnit::opLatency(OpClass cap)
{
    return opLatencies[cap];
}

bool
FuncUnit::isPipelined(OpClass capability)
{
    return pipelined[capability];
}

// TAG getting a latency for the corresponding operation
int FuncUnit::getLatencyForOp(OpClass op_class) const {
    // Assuming op_class is the index corresponding to the operation in the opLatencies array
    for (int i = 0; i < opLatencies.size(); ++i) {
        // std::cout << "Latency for operation " << i << " is " << opLatencies[i] << std::endl;
    }
    return opLatencies[op_class];
}

} // namespace gem5
