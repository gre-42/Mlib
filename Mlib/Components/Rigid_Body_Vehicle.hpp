#pragma once
#include <Mlib/Misc/Source_Location.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

namespace Mlib {

inline DanglingBaseClassRef<RigidBodyVehicle> get_rigid_body_vehicle(
    const SceneNode& node,
    SourceLocation loc)
{
    auto am = node.get_absolute_movable(loc);
    auto rb = dynamic_cast<RigidBodyVehicle*>(&am.get());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    return {*rb, loc};
}

inline bool has_rigid_body_vehicle(const DanglingBaseClassRef<SceneNode>& node) {
    return node->has_absolute_movable();
}

}
