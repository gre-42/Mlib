#include "Idl_Mod.hpp"
#include <cmath>

using namespace Mlib;

double Mlib::idl_mod(double a, double b) {
    auto m = std::fmod(a, b);
    if (a < 0) {
        m -= b;
    }
    return m;
}
