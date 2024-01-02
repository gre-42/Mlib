#pragma once
#include <mutex>

namespace Mlib {

class CursorStates;
struct BaseCursorAxisBinding;

class IncrementalMovement {
    IncrementalMovement& operator = (const IncrementalMovement&) = delete;
public:
    explicit IncrementalMovement(CursorStates& cursor_states);
    ~IncrementalMovement();
    void update_cursor(double x, double y);
    double consume_cursor(size_t axis);
    float axis_alpha(const BaseCursorAxisBinding& binding);
private:
    double cursor_x_;
    double cursor_y_;
    CursorStates& cursor_states_;
    std::mutex cursor_coordinates_mutex_;
};

}
