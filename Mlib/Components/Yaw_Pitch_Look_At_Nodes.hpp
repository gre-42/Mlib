#pragma once
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

inline YawPitchLookAtNodes& get_yaw_pitch_look_at_nodes(DanglingBaseClassRef<SceneNode> node) {
    auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&node->get_relative_movable());
    if (ypln == nullptr) {
        THROW_OR_ABORT("Relative movable is not a ypln");
    }
    return *ypln;
}

}
