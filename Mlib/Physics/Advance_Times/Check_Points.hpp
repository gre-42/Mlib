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
enum class RaceState;

struct CheckPointPose {
    TrackElementExtended track_element;
    size_t lap_index;
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
        size_t nlaps,
        const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
        AdvanceTimes& advance_times,
        SceneNode& moving_node,
        AbsoluteMovable& moving,
        const std::string& resource_name,
        IPlayer& player,
        size_t nbeacons,
        float distance,
        size_t nahead,
        float radius,
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        DeleteNodeMutex& delete_node_mutex,
        const Focuses& focuses,
        bool enable_height_changed_mode = false,
        const FixedArray<float, 3>& selection_emissivity = { -1.f, -1.f, -1.f },
        const FixedArray<float, 3>& deselection_emissivity = { -1.f, -1.f, -1.f },
        const std::function<void()>& on_finish = [](){});
    ~CheckPoints();
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(const Object& destroyed_object) override;
    double meters_to_start() const;
    size_t lap_index() const;
private:
    AdvanceTimes& advance_times_;
    TrackReader track_reader_;
    SceneNode* moving_node_;
    AbsoluteMovable* moving_;
    std::vector<BeaconNode> beacon_nodes_;
    std::string resource_name_;
    IPlayer& player_;
    float radius_;
    size_t nbeacons_;
    float distance_;
    size_t nahead_;
    size_t i01_;
    size_t lap_index_;
    SceneNodeResources& scene_node_resources_;
    Scene& scene_;
    DeleteNodeMutex& delete_node_mutex_;
    const Focuses& focuses_;
    float total_elapsed_seconds_;
    float lap_elapsed_seconds_;
    std::list<float> lap_times_seconds_;
    RaceState race_state_;
    std::list<TrackElement> movable_track_;
    std::list<CheckPointPose> checkpoints_ahead_;
    bool enable_height_changed_mode_;
    FixedArray<float, 3> selection_emissivity_;
    FixedArray<float, 3> deselection_emissivity_;
    std::function<void()> on_finish_;
};

}
