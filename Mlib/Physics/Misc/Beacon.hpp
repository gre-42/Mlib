#pragma once
#include <Mlib/Array/Fixed_Array.hpp>


namespace Mlib {

struct Beacon {
    FixedArray<float, 3> position;
    std::string resource_name = "beacon";
};

}
