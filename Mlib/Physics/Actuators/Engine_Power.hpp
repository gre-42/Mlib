#pragma once
#include <Mlib/Math/Interp.hpp>
#include <iosfwd>

namespace Mlib {

class EnginePower {
    friend std::ostream& operator << (std::ostream& ostr, const EnginePower& engine_power);
public:
    EnginePower(
        const Interp<float>& w_to_power,
        const std::vector<float>& gear_ratios);
    void auto_set_gear(float tire_w);
    float engine_w(float tire_w) const;
    float get_power(float tire_w) const;
private:
    Interp<float> w_to_power_;
    size_t gear_;
    std::vector<float> gear_ratios_;
};

std::ostream& operator << (std::ostream& ostr, const EnginePower& engine_power);

}
