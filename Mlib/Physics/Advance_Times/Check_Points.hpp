#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <fstream>
#include <mutex>

namespace Mlib {

class Focuses;
class AbsoluteMovable;
class AdvanceTimes;
class SceneNode;
class IPlayer;
class SceneNodeResources;
class Scene;
class DeleteNodeMutex;
struct BeaconNode;

struct CheckPointPose {
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
    BeaconNode* beacon_node;
};

struct BeaconNode {
    SceneNode* beacon_node;
    CheckPointPose* check_point_pose;
};

class CheckPoints: public DestructionObserver, public AdvanceTime {
public:
    CheckPoints(
        const std::string& filename,
        const TransformationMatrix<double, 3>* inverse_geographic_mapping,
        AdvanceTimes& advance_times,
        SceneNode& moving_node,
        AbsoluteMovable* moving,
        const std::string& resource_name,
        IPlayer* player,
        size_t nbeacons,
        size_t nth,
        size_t nahead,
        float radius,
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        DeleteNodeMutex& delete_node_mutex,
        const Focuses& focuses,
        bool enable_height_changed_mode = false,
        const FixedArray<float, 3>& deselection_ambience = { -1.f, -1.f, -1.f },
        const std::function<void()>& on_finish = [](){});
    ~CheckPoints();
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
private:
    AdvanceTimes& advance_times_;
    TrackReader track_reader_;
    SceneNode* moving_node_;
    AbsoluteMovable* moving_;
    std::vector<BeaconNode> beacon_nodes_;
    std::string resource_name_;
    IPlayer* player_;
    float radius_;
    size_t nbeacons_;
    size_t nth_;
    size_t nahead_;
    size_t i01_;
    SceneNodeResources& scene_node_resources_;
    Scene& scene_;
    DeleteNodeMutex& delete_node_mutex_;
    const Focuses& focuses_;
    float elapsed_seconds_;
    std::list<TrackElement> movable_track_;
    std::list<CheckPointPose> checkpoints_ahead_;
    bool enable_height_changed_mode_;
    const FixedArray<float, 3> deselection_ambience_;
    std::function<void()> on_finish_;
};

}
