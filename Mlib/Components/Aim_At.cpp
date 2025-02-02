#include "Aim_At.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

bool Mlib::has_aim_at(DanglingRef<SceneNode> node) {
    return node->has_sticky_absolute_observer();
}

AimAt& Mlib::get_aim_at(DanglingRef<SceneNode> node) {
    auto aim_at = dynamic_cast<AimAt*>(&node->get_sticky_absolute_observer());
    if (aim_at == nullptr) {
        THROW_OR_ABORT("Sticky absolute observer is not an aim_at");
    }
    return *aim_at;
}
