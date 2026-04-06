
#include "Weapon_Cycle.hpp"
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

DanglingBaseClassRef<const WeaponCycle> Mlib::get_weapon_cycle(
    const SceneNode& node,
    SourceLocation loc)
{
    return get_weapon_cycle(const_cast<SceneNode&>(node), loc);
}

DanglingBaseClassRef<WeaponCycle> Mlib::get_weapon_cycle(
    SceneNode& node,
    SourceLocation loc)
{
    auto nm = node.get_node_modifier(CURRENT_SOURCE_LOCATION);
    auto wc = dynamic_cast<WeaponCycle*>(&nm.get());
    if (wc == nullptr) {
        throw std::runtime_error("Node modifier is not a list of weapon cycles");
    }
    return {*wc, CURRENT_SOURCE_LOCATION};
}

bool Mlib::has_weapon_cycle(const DanglingBaseClassRef<const SceneNode>& node) {
    return node->has_node_modifier();
}
