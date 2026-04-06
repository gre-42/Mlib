
#include "Incremental_Movement.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Sigmoid/Signed_Min.hpp>
#include <Mlib/OpenGL/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/OpenGL/Ui/Cursor_States.hpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

IncrementalMovement::IncrementalMovement(CursorStates& cursor_states)
: cursor_x_{ 0.f },
  cursor_y_{ 0.f },
  cursor_states_{ cursor_states }
{
    cursor_states.register_cursor_movement(this);
}

IncrementalMovement::~IncrementalMovement() {
    cursor_states_.unregister_cursor_movement(this);
}

void IncrementalMovement::update_cursor(double x, double y) {
    std::scoped_lock lock{ cursor_coordinates_mutex_ };
    cursor_x_ += x;
    cursor_y_ += y;
}

double IncrementalMovement::consume_cursor_unsafe(size_t axis) {
    double result;
    switch (axis) {
    case 0:
        result = cursor_x_;
        cursor_x_ = 0;
        return result;
    case 1:
        result = cursor_y_;
        cursor_y_ = 0;
        return result;
    }
    throw std::runtime_error("Unknown cursor axis");
}

void IncrementalMovement::increment_cursor_unsafe(size_t axis, double value) {
    switch (axis) {
    case 0:
        cursor_x_ += value;
        return;
    case 1:
        cursor_y_ += value;
        return;
    }
    throw std::runtime_error("Unknown cursor axis");
}

float IncrementalMovement::axis_alpha(const BaseCursorAxisBinding& binding, float ncached) {
    if (binding.axis == SIZE_MAX) {
        return NAN;
    }
    if (std::isnan(binding.sign_and_scale)) {
        throw std::runtime_error("Cursor axis sign_and_scale is NAN");
    }
    std::scoped_lock lock{ cursor_coordinates_mutex_ };
    auto v = (float)consume_cursor_unsafe(binding.axis);
    if (sign(v) != sign(binding.sign_and_scale)) {
        return NAN;
    }
    // Read "v" cursor ticks, ignore everything that is above the "ncached" threshold.
    v = signed_min(v, ncached / std::abs(binding.sign_and_scale));
    // Compute the alpha-value from "v".
    auto result = std::min(binding.sign_and_scale * v, 1.f);
    // Compute the unused cursor ticks and add them back to the cursor position.
    // Ticks above "ncached" will be thrown away.
    auto da = std::max(std::abs(v - result / binding.sign_and_scale), 0.f) * sign(binding.sign_and_scale);
    increment_cursor_unsafe(binding.axis, da);
    return result;
}
