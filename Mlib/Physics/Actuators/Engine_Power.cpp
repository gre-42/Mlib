#include "Engine_Power.hpp"
#include <ostream>

using namespace Mlib;

static float engine_w(float tire_w, float gear_ratio) {
    return tire_w * gear_ratio;
}

EnginePower::EnginePower(
    const Interp<float>& w_to_power,
    const std::vector<float>& gear_ratios)
: w_to_power_{w_to_power},
  gear_{SIZE_MAX},
  gear_ratios_{gear_ratios}
{
    // if (w_to_power(0) == 0.f) {
    //     throw std::runtime_error("The power at angular velocity \"zero\" cannot be \"zero\". Please specify a small, positive value.");
    // }
}

void EnginePower::auto_set_gear(float tire_w) {
    if (std::isnan(tire_w)) {
        throw std::runtime_error("tire_w is NAN in auto_set_gear");
    }
    if (gear_ratios_.empty()) {
        throw std::runtime_error("Gear ratio array is empty");
    }
    auto it = std::max_element(
        gear_ratios_.begin(),
        gear_ratios_.end(),
        [this, &tire_w](float r0, float r1){
            return w_to_power_(::engine_w(r0, tire_w)) < w_to_power_(::engine_w(r1, tire_w));});
    if (it == gear_ratios_.end()) {
        throw std::runtime_error("auto_set_gear internal error");
    }
    gear_ = (it - gear_ratios_.begin());
}

float EnginePower::engine_w(float tire_w) const {
    if (std::isnan(tire_w)) {
        throw std::runtime_error("tire_w is NAN in engine_w");
    }
    if (gear_ == SIZE_MAX) {
        throw std::runtime_error("Gear uninitialized");
    }
    if (gear_ >= gear_ratios_.size()) {
        throw std::runtime_error("Gear too large");
    }
    return ::engine_w(gear_ratios_.at(gear_), tire_w);
}

float EnginePower::get_power(float tire_w) const {
    return w_to_power_(engine_w(tire_w));
}

std::ostream& Mlib::operator << (std::ostream& ostr, const EnginePower& engine_power) {
    ostr << "Engine power\n";
    ostr << "   w_to_power " << engine_power.w_to_power_ << '\n';
    ostr << "   gear " << engine_power.gear_ << '\n';
    for (float r : engine_power.gear_ratios_) {
        ostr << "   gear_ratio " << r << '\n';
    }
    return ostr;
}
