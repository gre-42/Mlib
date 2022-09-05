#pragma once
#include <Mlib/Signal/Pid_Controller.hpp>

namespace Mlib {

class Player;
class SingleWaypoint;

class PlayerMovement {
    friend SingleWaypoint;
public:
    PlayerMovement(Player& player);
    ~PlayerMovement();

    void set_vehicle_control_parameters(
        float surface_power_forward,
        float surface_power_backward,
        float max_tire_angle,
        const PidController<float, float>& tire_angle_pid);
    void reset_node();

    void run_move(
        float yaw,
        float pitch,
        float forwardmove,
        float sidemove);

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
    float surface_power_forward_;
    float surface_power_backward_;
    float max_tire_angle_;
    PidController<float, float> tire_angle_pid_;
};

}
