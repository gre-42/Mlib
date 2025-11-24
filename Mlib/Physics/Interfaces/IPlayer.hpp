#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <chrono>
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;
template <class T>
class DanglingBaseClassPtr;
class SceneNode;
struct TrackElement;
class RigidBodyVehicle;
enum class RaceState;
template <typename TData, size_t... tshape>
class FixedArray;
class DestructionFunctions;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class VehicleSpawner;
class SceneVehicle;

enum class SelectNextVehicleQuery {
    NONE = 0,
    ENTER_IF_FREE = 1 << 0,
    EXIT = 1 << 1,
    ENTER_BY_FORCE = 1 << 2,
    ANY_ENTER = ENTER_IF_FREE | ENTER_BY_FORCE
};

inline bool any(SelectNextVehicleQuery q) {
    return q != SelectNextVehicleQuery::NONE;
}

inline SelectNextVehicleQuery operator & (SelectNextVehicleQuery a, SelectNextVehicleQuery b) {
    return (SelectNextVehicleQuery)((int)a & (int)b);
}

inline SelectNextVehicleQuery operator | (SelectNextVehicleQuery a, SelectNextVehicleQuery b) {
    return (SelectNextVehicleQuery)((int)a | (int)b);
}

class IPlayer {
public:
    virtual const VariableAndHash<std::string>& id() const = 0;
    virtual std::string title() const = 0;
    virtual std::optional<VariableAndHash<std::string>> target_id() const = 0;
    virtual bool reset_vehicle_requested() = 0;
    virtual bool can_reset_vehicle(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const = 0;
    virtual bool try_reset_vehicle(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) = 0;
    virtual void select_next_vehicle(
        SelectNextVehicleQuery q,
        const std::string& seat) = 0;
    virtual void set_next_vehicle(
        VehicleSpawner& spawner,
        SceneVehicle& vehicle,
        const std::string& seat) = 0;
    virtual void clear_next_vehicle() = 0;
    virtual std::vector<DanglingBaseClassPtr<SceneNode>> moving_nodes() const = 0;
    virtual void notify_race_started() = 0;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const UUVector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) = 0;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual void notify_bullet_generated(std::chrono::steady_clock::time_point time) = 0;
    virtual DestructionFunctions& on_destroy_player() = 0;
    virtual DestructionFunctions& on_clear_vehicle() = 0;
};

}
