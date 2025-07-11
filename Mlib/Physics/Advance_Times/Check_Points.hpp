#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Advance_Times/Respawn_Config.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <fstream>
#include <mutex>

namespace Mlib {

class Focuses;
class IAbsoluteMovable;
class AdvanceTimes;
class SceneNode;
class IPlayer;
class SceneNodeResources;
class Scene;
class DeleteNodeMutex;
struct BeaconNode;
enum class RaceState;
class RenderingResources;

struct CheckPointPose {
    TrackElementExtended track_element;
    std::list<TrackElementExtended> history;
    double progress;
    size_t lap_index;
    BeaconNode* beacon_node;
};

struct BeaconNode {
    VariableAndHash<std::string> beacon_node_name;
    DanglingPtr<SceneNode> beacon_node;
    CheckPointPose* check_point_pose;
};

struct TrafoAndMetersToStart {
    TransformationMatrix<SceneDir, ScenePos, 3> trafo;
    double meters_to_start;
};

class CheckPoints: public DestructionObserver<SceneNode&>, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    CheckPoints(
        std::unique_ptr<ITrackElementSequence>&& sequence,
        size_t nframes,
        size_t nlaps,
        const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
        std::string asset_id,
        VariableAndHash<std::string> resource_name,
        uint32_t user_id,
        const DanglingBaseClassRef<IPlayer>& player,
        size_t nbeacons,
        float distance,
        size_t nahead,
        float radius,
        RenderingResources* rendering_resources,
        SceneNodeResources& scene_node_resources,
        Scene& scene,
        DeleteNodeMutex& delete_node_mutex,
        const Focuses& focuses,
        bool enable_height_changed_mode = false,
        const FixedArray<float, 3>& selection_emissive = { -1.f, -1.f, -1.f },
        const FixedArray<float, 3>& deselection_emissive = { -1.f, -1.f, -1.f },
        const RespawnConfig& respawn_config = RespawnConfig{},
        const std::function<void()>& on_finish = [](){});
    ~CheckPoints();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    bool has_meters_to_start() const;
    double meters_to_start() const;
    size_t lap_index() const;
private:
    void advance_time(float dt);
    void reset_player();
    TrackReader track_reader_;
    size_t nlaps_;
    std::string asset_id_;
    std::vector<DanglingPtr<SceneNode>> moving_nodes_;
    std::vector<IAbsoluteMovable*> movings_;
    std::vector<BeaconNode> beacon_nodes_;
    VariableAndHash<std::string> resource_name_;
    uint32_t user_id_;
    DanglingBaseClassRef<IPlayer> player_;
    float radius_;
    size_t nbeacons_;
    float distance_;
    size_t nahead_;
    size_t i01_;
    size_t lap_index_;
    double progress_;
    double straight_progress_;
    RenderingResources* rendering_resources_;
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
    std::optional<FixedArray<ScenePos, 3>> last_direction_;
    std::optional<FixedArray<ScenePos, 3>> last_reached_checkpoint_;
    std::list<TrafoAndMetersToStart> straight_checkpoints_;
    std::list<TrackElementExtended> history_;
    bool enable_height_changed_mode_;
    FixedArray<float, 3> selection_emissive_;
    FixedArray<float, 3> deselection_emissive_;
    RespawnConfig respawn_config_;
    std::function<void()> on_finish_;
};

}
