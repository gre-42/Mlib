#include "Rigid_Body_Vehicle_Destruction_Observers.hpp"
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Memory/Destruction_Observers.impl.hpp>

namespace Mlib {

template class DestructionObservers<const RigidBodyVehicle&>;

}
