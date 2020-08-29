#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class RigidBodyIntegrator;

class RigidBodyRecorder: public DestructionObserver, public AdvanceTime {
public:
    RigidBodyRecorder(
        const std::string& filename,
        AdvanceTimes& advance_times,
        SceneNode* recorded_node,
        RigidBodyIntegrator* rbi,
        const Focus& focus);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;

private:
    const Focus& focus_;
    AdvanceTimes& advance_times_;
    SceneNode* recorded_node_;
    RigidBodyIntegrator* rbi_;
    std::ofstream ofstr_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
};

}
