#pragma once
#include <list>
#include <string>

namespace Mlib {

struct TrackElement;

class IPlayer {
public:
    virtual const std::string& name() const = 0;
    virtual void notify_lap_time(
        float lap_time,
        const std::list<TrackElement>& track) = 0;
};

}
