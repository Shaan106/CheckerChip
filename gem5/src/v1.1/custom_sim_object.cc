#include "v1.1/custom_sim_object.hh"

#include <iostream>

namespace gem5
{

CustomSimObject::CustomSimObject(const CustomSimObjectParams &params) :
    SimObject(params)
{
    std::cout << "Hello World! From a CustomSimObject!" << std::endl;
}

} // namespace gem5