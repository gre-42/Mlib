#include "Countdown_Physics.hpp"
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Physics/Units.hpp>
#include <mutex>

using namespace Mlib;

CountdownPhysics::CountdownPhysics()
    : elapsed_{ NAN }
    , duration_{ NAN }
{}

CountdownPhysics::~CountdownPhysics() {
    on_destroy.clear();
}

void CountdownPhysics::reset(float duration) {
    std::scoped_lock lock{ mutex_ };
    duration_ = duration;
    elapsed_ = 0.f;
}

uint32_t CountdownPhysics::seconds_remaining() const {
    std::scoped_lock lock{ mutex_ };
    if (std::isnan(duration_)) {
        return 0;
    }
    return float_to_integral<uint32_t>(std::ceil((duration_ - elapsed_) / seconds));
}

bool CountdownPhysics::counting() const {
    std::scoped_lock lock{ mutex_ };
    if (std::isnan(duration_)) {
        return false;
    }
    return elapsed_ < duration_;
}

bool CountdownPhysics::finished() const {
    return !counting();
}

void CountdownPhysics::advance_time(float dt, const StaticWorld& world) {
    std::scoped_lock lock{ mutex_ };
    if (!std::isnan(duration_)) {
        elapsed_ = std::min(duration_, elapsed_ + dt);
    }
}
