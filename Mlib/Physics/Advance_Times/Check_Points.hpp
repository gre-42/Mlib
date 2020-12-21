#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class AbsoluteMovable;
class AdvanceTimes;
class SceneNode;
class Players;
class Player;
class SceneNodeResources;
class Scene;

struct CheckPointPose {
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
};

class CheckPoints: public DestructionObserver, public AdvanceTime {
public:
    CheckPoints(
        const std::string& filename,
        AdvanceTimes& advance_times,
        SceneNode* moving_node,
        AbsoluteMovable* moving,
        const std::string& resource_name,
        Players* players,
        Player* player,
        size_t nbeacons,
        size_t nth,
        size_t nahead,
        float radius,
        SceneNodeResources& scene_node_resources,
        Scene& scene);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
private:
    AdvanceTimes& advance_times_;
    TrackReader track_reader_;
    SceneNode* moving_node_;
    AbsoluteMovable* moving_;
    std::vector<SceneNode*> beacon_nodes_;
    std::string resource_name_;
    Players* players_;
    Player* player_;
    float radius_;
    size_t nbeacons_;
    size_t nth_;
    size_t nahead_;
    size_t i01_;
    SceneNodeResources& scene_node_resources_;
    Scene& scene_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    std::list<CheckPointPose> checkpoints_ahead_;
};

}
