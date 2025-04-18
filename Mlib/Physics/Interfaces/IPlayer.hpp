#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class DanglingPtr;
class SceneNode;
struct TrackElement;
class RigidBodyVehicle;
enum class RaceState;
template <typename TData, size_t... tshape>
class FixedArray;
class DestructionFunctions;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IPlayer {
public:
    virtual std::string id() const = 0;
    virtual std::string title() const = 0;
    virtual std::optional<std::string> target_id() const = 0;
    virtual bool reset_vehicle_requested() = 0;
    virtual bool can_reset_vehicle(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const = 0;
    virtual bool try_reset_vehicle(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) = 0;
    virtual std::vector<DanglingPtr<SceneNode>> moving_nodes() const = 0;
    virtual void notify_race_started() = 0;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const UUVector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) = 0;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual DestructionFunctions& on_destroy_player() = 0;
    virtual DestructionFunctions& on_clear_vehicle() = 0;
};

}
