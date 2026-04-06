#pragma once
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

namespace Mlib {

inline DanglingBaseClassRef<YawPitchLookAtNodes> get_yaw_pitch_look_at_nodes(SceneNode& node, SourceLocation loc) {
    auto rm = node.get_relative_movable(loc);
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&rm.get());
    if (ypln == nullptr) {
        throw std::runtime_error("Relative movable is not a ypln");
    }
    return {*ypln, loc};
}

}
