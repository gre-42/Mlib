#include "Engine_Power.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>

using namespace Mlib;

static float engine_w(float tire_w, float gear_ratio, float w_clutch) {
    float ew = tire_w * gear_ratio;
    if (ew > 0) {
        return NAN;
    } else {
        return std::min(ew, -w_clutch);
    }
}

EnginePower::EnginePower(
    const Interp<float>& w_to_power,
    const std::vector<float>& gear_ratios,
    float w_clutch,
    float max_dw)
: engine_w_{0.f},
  w_clutch_{w_clutch},
  max_dw_{max_dw},
  w_to_power_{w_to_power},
  gear_ratios_{gear_ratios}
{
    if (gear_ratios.empty()) {
        THROW_OR_ABORT("gear_ratios is empty");
    }
    gear_ = gear_ratios.size() - 1;
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
            return get_power(tire_w, r0) < get_power(tire_w, r1);});
    if (it == gear_ratios_.end()) {
        THROW_OR_ABORT("auto_set_gear internal error");
    }
    if (get_power(tire_w, *it) > get_power(tire_w, gear_ratios_[gear_])) {
        gear_ = size_t(it - gear_ratios_.begin());
    }
    float target_engine_w = ::engine_w(tire_w, gear_ratios_[gear_], w_clutch_);
    if (std::isnan(target_engine_w)) {
        THROW_OR_ABORT("Target engine w is NAN");
    }
    engine_w_ += std::clamp(target_engine_w - engine_w_, -max_dw_ * dt, max_dw_ * dt);
}

float EnginePower::engine_w() const {
    if (std::isnan(engine_w_)) {
        THROW_OR_ABORT("engine_w is NAN in engine_w");
    }
    return engine_w_;
}

float EnginePower::get_power() const {
    return w_to_power_(engine_w_);
}

float EnginePower::get_power(float tire_w, float gear_ratio) const {
    float engine_w = ::engine_w(tire_w, gear_ratio, w_clutch_);
    if (std::isnan(engine_w)) {
        return 0.f;
    }
    float res = w_to_power_(engine_w);
    return std::isnan(res) ? 0.f : res;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const EnginePower& engine_power) {
    ostr << "Engine power\n";
    ostr << "   w_to_power " << engine_power.w_to_power_ << '\n';
    ostr << "   gear " << engine_power.gear_ << '\n';
    for (float r : engine_power.gear_ratios_) {
        ostr << "   gear_ratio " << r << '\n';
    }
    ostr << "   w_clutch " << engine_power.w_clutch_ << '\n';
    ostr << "   max_dw " << engine_power.max_dw_ << '\n';
    return ostr;
}
