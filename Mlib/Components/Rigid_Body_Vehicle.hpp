#pragma once
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

inline RigidBodyVehicle& get_rigid_body_vehicle(DanglingRef<SceneNode> node) {
    auto rb = dynamic_cast<RigidBodyVehicle*>(&node->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    return *rb;
}

inline bool has_rigid_body_vehicle(DanglingRef<SceneNode> node) {
    return node->has_absolute_movable();
}

}
