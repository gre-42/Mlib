#pragma once

namespace Mlib {

class RigidBodyVehicle;
template <class T>
class DestructionObservers;

class ITeam {
public:
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual DestructionObservers<const ITeam&>& destruction_observers() = 0;
};

}
