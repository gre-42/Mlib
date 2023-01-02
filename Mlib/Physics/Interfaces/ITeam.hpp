#pragma once

namespace Mlib {

class Bullet;
class RigidBodyVehicle;

class ITeam {
public:
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) = 0;
    virtual void notify_bullet_destroyed(Bullet& bullet) = 0;
};

}
