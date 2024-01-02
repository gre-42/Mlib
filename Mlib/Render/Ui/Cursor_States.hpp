#pragma once
#include <list>
#include <mutex>
#include <set>

namespace Mlib {

class IncrementalMovement;

class CursorStates {
    CursorStates(const CursorStates&) = delete;
    CursorStates& operator = (const CursorStates&) = delete;
public:
    CursorStates();
    void register_cursor_movement(IncrementalMovement* cursor_movement);
    void unregister_cursor_movement(IncrementalMovement* cursor_movement);
    void update_cursor(double x, double y);
private:
    std::list<IncrementalMovement*> cursor_movements_;
    std::mutex cursor_movements_mutex_;
};

}
