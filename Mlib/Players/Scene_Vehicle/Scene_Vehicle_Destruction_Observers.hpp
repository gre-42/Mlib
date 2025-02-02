#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
class SceneVehicle;

using SceneVehicleDestructionObservers = DestructionObservers<const SceneVehicle&>;

}
