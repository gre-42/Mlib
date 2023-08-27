#include "Scene_Vehicle_Destruction_Observers.hpp"
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle_Destruction_Observers.hpp>
#include <Mlib/Memory/Destruction_Observers.impl.hpp>

namespace Mlib {

template class DestructionObservers<const SceneVehicle&>;

}
