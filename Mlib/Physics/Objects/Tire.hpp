#pragma once
#include <Mlib/Physics/Objects/Shock_Absorber.hpp>
#include <string>

namespace Mlib {

struct Tire {
    Tire(const std::string& engine, float break_force, float Ks, float Ka, float angle);
    void advance_time(float dt);
    ShockAbsorber shock_absorber;
    float angle;
    std::string engine;
    float break_force;
};

}
