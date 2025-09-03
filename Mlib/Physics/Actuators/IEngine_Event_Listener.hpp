#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
struct RotatingFrame;
struct EnginePowerIntent;
struct StaticWorld;

class IEngineEventListener {
public:
    virtual ~IEngineEventListener() = default;
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power,
        const StaticWorld& static_world) = 0;
    virtual void set_location(
        const RotatingFrame<SceneDir, ScenePos, 3>& frame) = 0;
    virtual void advance_time(float dt) = 0;
};

}
