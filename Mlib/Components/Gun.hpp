#pragma once
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

inline Gun& get_gun(DanglingRef<SceneNode> node) {
    auto gun = dynamic_cast<Gun*>(&node->get_absolute_observer());
    if (gun == nullptr) {
        THROW_OR_ABORT("Absolute observer is not a gun");
    }
    return *gun;
}

}
