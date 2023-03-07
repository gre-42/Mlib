#include "Cursor_Movement.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CursorMovement::CursorMovement(CursorStates& cursor_states)
: cursor_x_{ 0.f },
  cursor_y_{ 0.f },
  cursor_states_{ cursor_states }
{
    cursor_states.register_cursor_movement(this);
}

CursorMovement::~CursorMovement() {
    cursor_states_.unregister_cursor_movement(this);
}

void CursorMovement::update_cursor(double x, double y) {
    std::scoped_lock lock{cursor_coordinates_mutex_};
    cursor_x_ += x;
    cursor_y_ += y;
}

double CursorMovement::consume_cursor(size_t axis) {
    std::scoped_lock lock{cursor_coordinates_mutex_};
    double result;
    if (axis == 0) {
        result = cursor_x_;
        cursor_x_ = 0;
    } else if (axis == 1) {
        result = cursor_y_;
        cursor_y_ = 0;
    } else {
        THROW_OR_ABORT("Unknown cursor axis");
    }
    return result;
}

float CursorMovement::axis_alpha(const BaseCursorAxisBinding& binding) {
    if (binding.axis == SIZE_MAX) {
        return NAN;
    }
    if (std::isnan(binding.sign_and_scale)) {
        THROW_OR_ABORT("Cursor axis sign_and_scale is NAN");
    }
    float v = (float)consume_cursor(binding.axis);
    if (sign(v) != sign(binding.sign_and_scale)) {
        return NAN;
    }
    return std::min(binding.sign_and_scale * v, 1.f);
}
