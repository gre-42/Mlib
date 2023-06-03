#pragma once
#include <string>

namespace Mlib {

class RigidBodyVehicle;
class IPlayer;

class ISpawner {
public:
    virtual void notify_vehicle_destroyed(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual IPlayer* player() = 0;
};

}
