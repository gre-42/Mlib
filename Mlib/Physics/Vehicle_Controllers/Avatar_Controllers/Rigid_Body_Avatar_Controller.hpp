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
    void increment_legs_z(const FixedArray<float, 3>& dz);
    void walk(float surface_power, float relaxation);
    void stop();
    void set_target_yaw(float target_yaw);
    void set_target_pitch(float target_pitch);
    void increment_yaw(float dyaw, float relaxation);
    void increment_pitch(float dpitch, float relaxation);
    void reset();
    virtual void apply() = 0;
protected:
    FixedArray<float, 3> legs_z_;
    float target_yaw_;
    float target_pitch_;
    float dyaw_;
    float dyaw_relaxation_;
    float dpitch_;
    float dpitch_relaxation_;
    float surface_power_;
    float drive_relaxation_;
};

}
