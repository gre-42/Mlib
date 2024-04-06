#pragma once
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

struct TrackElement;
class RigidBodyVehicle;
enum class RaceState;
template <typename TData, size_t... tshape>
class FixedArray;
template <class T>
class DestructionObservers;

class IPlayer {
public:
    virtual std::optional<std::string> target_name() const = 0;
    virtual const std::string& name() const = 0;
    virtual void notify_race_started() = 0;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const std::vector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) = 0;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual DestructionObservers<const IPlayer&>& destruction_observers() = 0;
};

}
