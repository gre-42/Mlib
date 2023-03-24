#pragma once
#include <Mlib/Math/Interp.hpp>
#include <iosfwd>

namespace Mlib {

class EnginePower {
    friend std::ostream& operator << (std::ostream& ostr, const EnginePower& engine_power);
public:
    EnginePower(
        const Interp<float>& w_to_power,
        const std::vector<float>& gear_ratios,
        float max_dw);
    void auto_set_gear(float dt, float tire_w);
    float engine_w() const;
    float get_power() const;
private:
    float engine_w_;
    float max_dw_;
    Interp<float> w_to_power_;
    size_t gear_;
    std::vector<float> gear_ratios_;
};

std::ostream& operator << (std::ostream& ostr, const EnginePower& engine_power);

}
