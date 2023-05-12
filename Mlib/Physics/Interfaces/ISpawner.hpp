#pragma once
#include <string>

namespace Mlib {

class IPlayer;

class ISpawner {
public:
    virtual void notify_vehicle_destroyed() = 0;
    virtual IPlayer* player() = 0;
};

}
