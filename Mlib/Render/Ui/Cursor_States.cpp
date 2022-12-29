#include "Cursor_States.hpp"
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>

using namespace Mlib;

CursorStates::CursorStates()
{}

void CursorStates::register_cursor_movement(CursorMovement* cursor_movement) {
    std::lock_guard lock{cursor_movements_mutex_};
    cursor_movements_.push_back(cursor_movement);
}

void CursorStates::unregister_cursor_movement(CursorMovement* cursor_movement) {
    std::lock_guard lock{cursor_movements_mutex_};
    auto it = std::find(cursor_movements_.begin(), cursor_movements_.end(), cursor_movement);
    if (it == cursor_movements_.end()) {
        THROW_OR_ABORT("Could not find cursor_movement to be removed");
    }
    cursor_movements_.erase(it);
}

void CursorStates::update_cursor(double x, double y) {
    std::lock_guard lock{cursor_movements_mutex_};
    for (auto& c : cursor_movements_) {
        c->update_cursor(x, y);
    }
}
