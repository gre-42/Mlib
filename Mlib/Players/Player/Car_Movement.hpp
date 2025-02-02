#pragma once
#include <Mlib/Signal/Pid_Controller.hpp>
#include <optional>

namespace Mlib {

class Player;
class SingleWaypoint;

class CarMovement {
    CarMovement(const CarMovement&) = delete;
    CarMovement& operator = (const CarMovement&) = delete;
public:
    explicit CarMovement(Player& player);
    ~CarMovement();

    void set_max_tire_angle(float max_tire_angle);
    void set_tire_angle_pid(const PidController<float, float>& tire_angle_pid);
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
    std::optional<float> max_tire_angle_;
    std::optional<PidController<float, float>> tire_angle_pid_;
};

}
