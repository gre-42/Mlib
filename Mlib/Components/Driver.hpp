#pragma once

namespace Mlib {

class Player;
class RigidBodyVehicle;
template <class T>
class DanglingBaseClassRef;

DanglingBaseClassRef<Player> get_driver(RigidBodyVehicle& rb);

}
