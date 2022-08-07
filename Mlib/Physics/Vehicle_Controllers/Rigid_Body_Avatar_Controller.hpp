#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBodyVehicle;
template <typename TData, size_t... tshape>
class FixedArray;

class RigidBodyAvatarController {
public:
    RigidBodyAvatarController();
    virtual ~RigidBodyAvatarController();
    void increment_tires_z(const FixedArray<float, 3>& dz);
    void walk(float surface_power);
    void stop();
    void set_target_yaw(float target_yaw);
    void set_target_pitch(float target_pitch);
    void increment_yaw(float dyaw);
    void increment_pitch(float dpitch);
    void reset();
    virtual void apply() = 0;
protected:
    FixedArray<float, 3> legs_z_;
    float target_yaw_;
    float target_pitch_;
    float dyaw_;
    float dpitch_;
    float surface_power_;
};

}
