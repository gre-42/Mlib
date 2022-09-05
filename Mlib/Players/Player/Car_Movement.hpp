#pragma once
#include <Mlib/Signal/Pid_Controller.hpp>

namespace Mlib {

class Player;
class SingleWaypoint;

class CarMovement {
    friend SingleWaypoint;
public:
    CarMovement(Player& player);
    ~CarMovement();

    void set_control_parameters(
        float max_tire_angle,
        const PidController<float, float>& tire_angle_pid);
    void reset_node();

    void step_on_brakes();
    void drive_forward();
    void drive_backwards();
    void roll_tires();
    void steer(float angle);
    void steer_left_full();
    void steer_right_full();
    void steer_left_partial(float angle);
    void steer_right_partial(float angle);
private:
    Player& player_;
    float max_tire_angle_;
    PidController<float, float> tire_angle_pid_;
};

}
