#pragma once
#include <list>
#include <mutex>
#include <set>

namespace Mlib {

class CursorMovement;

class CursorStates {
    CursorStates(const CursorStates&) = delete;
    CursorStates& operator = (const CursorStates&) = delete;
public:
    void register_cursor_movement(CursorMovement* cursor_movement);
    void unregister_cursor_movement(CursorMovement* cursor_movement);
    void update_cursor(double x, double y);
private:
    std::list<CursorMovement*> cursor_movements_;
    std::mutex cursor_movements_mutex_;
};

}
