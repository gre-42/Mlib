#pragma once
#include <list>
#include <string>

namespace Mlib {

struct TrackElement;
class Bullet;

class IPlayer {
public:
    virtual const std::string& name() const = 0;
    virtual void notify_lap_time(
        float lap_time,
        const std::list<TrackElement>& track) = 0;
    virtual void notify_vehicle_destroyed() = 0;
    virtual void notify_kill() = 0;
    virtual void notify_bullet_destroyed(Bullet* bullet) = 0;
};

}
