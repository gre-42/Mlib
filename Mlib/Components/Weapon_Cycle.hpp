#pragma once
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

inline const WeaponCycle& get_weapon_cycle(DanglingRef<const SceneNode> node) {
    auto wc = dynamic_cast<WeaponCycle*>(&node->get_node_modifier());
    if (wc == nullptr) {
        THROW_OR_ABORT("Node modifier is not a weapon inventory");
    }
    return *wc;
}

inline WeaponCycle& get_weapon_cycle(DanglingRef<SceneNode> node) {
    return const_cast<WeaponCycle&>(get_weapon_cycle((DanglingRef<const SceneNode>)node));
}

inline bool has_weapon_cycle(DanglingRef<const SceneNode> node) {
    return node->has_node_modifier();
}

}
