#include "Engine_Power.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>

using namespace Mlib;

static float engine_w(float tire_w, float gear_ratio) {
    return tire_w * gear_ratio;
}

EnginePower::EnginePower(
    const Interp<float>& w_to_power,
    const std::vector<float>& gear_ratios,
    float max_dw)
: engine_w_{0.f},
  max_dw_{max_dw},
  w_to_power_{w_to_power},
  gear_{SIZE_MAX},
  gear_ratios_{gear_ratios}
{
    // if (w_to_power(0) == 0.f) {
    //     THROW_OR_ABORT("The power at angular velocity \"zero\" cannot be \"zero\". Please specify a small, positive value.");
    // }
}

void EnginePower::auto_set_gear(float dt, float tire_w) {
    if (std::isnan(tire_w)) {
        THROW_OR_ABORT("tire_w is NAN in auto_set_gear");
    }
    if (gear_ratios_.empty()) {
        THROW_OR_ABORT("Gear ratio array is empty");
    }
    auto it = std::max_element(
        gear_ratios_.begin(),
        gear_ratios_.end(),
        [this, &tire_w](float r0, float r1){
            return w_to_power_(::engine_w(tire_w, r0)) < w_to_power_(::engine_w(tire_w, r1));});
    if (it == gear_ratios_.end()) {
        THROW_OR_ABORT("auto_set_gear internal error");
    }
    gear_ = size_t(it - gear_ratios_.begin());
    engine_w_ += std::clamp(::engine_w(tire_w, *it) - engine_w_, -max_dw_ * dt, max_dw_ * dt);
}

float EnginePower::engine_w() const {
    if (std::isnan(engine_w_)) {
        THROW_OR_ABORT("engine_w is NAN in engine_w");
    }
    if (gear_ == SIZE_MAX) {
        THROW_OR_ABORT("Gear uninitialized");
    }
    if (gear_ >= gear_ratios_.size()) {
        THROW_OR_ABORT("Gear too large");
    }
    return engine_w_;
}

float EnginePower::get_power() const {
    return w_to_power_(engine_w_);
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
