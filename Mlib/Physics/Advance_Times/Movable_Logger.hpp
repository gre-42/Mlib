#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Loggable.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;

class MovableLogger: public DestructionObserver, public AdvanceTime {
public:
    MovableLogger(
        SceneNode& scene_node,
        AdvanceTimes& advance_times,
        Loggable* logged,
        unsigned int log_components);
    virtual void notify_destroyed(void* destroyed_object) override;
    virtual void advance_time(float dt) override;
private:
    SceneNode& scene_node_;
    AdvanceTimes& advance_times_;
    Loggable* logged_;
    unsigned int log_components_;
};

}
