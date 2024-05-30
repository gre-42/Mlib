#pragma once

namespace Mlib {

class RigidBodyVehicle;
class DestructionFunctions;

class ITeam {
public:
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual DestructionFunctions& on_destroy_team() = 0;
};

}
