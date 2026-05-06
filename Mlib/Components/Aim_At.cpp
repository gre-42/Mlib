#include "Aim_At.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

bool Mlib::has_aim_at(const DanglingBaseClassRef<SceneNode>& node) {
    return node->has_sticky_absolute_observer();
}

DanglingBaseClassRef<AimAt> Mlib::get_aim_at(const SceneNode& node, SourceLocation loc) {
    auto sao = node.get_sticky_absolute_observer();
    auto aim_at = dynamic_cast<AimAt*>(&sao.get());
    if (aim_at == nullptr) {
        throw std::runtime_error("Sticky absolute observer is not an aim_at");
    }
    return {*aim_at, loc};
}
