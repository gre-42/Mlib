#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
class ITeam;

using RigidBodyVehicleDestructionObservers = DestructionObservers<const ITeam&>;

}
