#pragma once
#include <string>

namespace Mlib {

struct AnimationFrame {
    std::string name;
    float loop_begin;
    float loop_end;
    float loop_time;
};

}
