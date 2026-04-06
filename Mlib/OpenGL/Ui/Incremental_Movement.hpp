#pragma once
#include <Mlib/Threads/Fast_Mutex.hpp>

namespace Mlib {

class CursorStates;
struct BaseCursorAxisBinding;

class IncrementalMovement {
    IncrementalMovement& operator = (const IncrementalMovement&) = delete;
public:
    explicit IncrementalMovement(CursorStates& cursor_states);
    ~IncrementalMovement();
    void update_cursor(double x, double y);
    float axis_alpha(const BaseCursorAxisBinding& binding, float nsubsteps);
private:
    double consume_cursor_unsafe(size_t axis);
    void increment_cursor_unsafe(size_t axis, double value);
    double cursor_x_;
    double cursor_y_;
    CursorStates& cursor_states_;
    FastMutex cursor_coordinates_mutex_;
};

}
