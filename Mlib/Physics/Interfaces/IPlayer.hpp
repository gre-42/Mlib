#pragma once
#include <list>
#include <string>
#include <vector>

namespace Mlib {

struct TrackElement;
class Bullet;
class RigidBodyVehicle;
enum class RaceState;

class IPlayer {
public:
    virtual const std::string& name() const = 0;
    virtual void notify_race_started() = 0;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const std::vector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) = 0;
    virtual void notify_vehicle_destroyed() = 0;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual void notify_bullet_destroyed(Bullet& bullet) = 0;
};

}
