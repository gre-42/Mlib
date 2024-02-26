#include "Gun.hpp"
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Gun& Mlib::get_gun(DanglingRef<SceneNode> node) {
    auto gun = dynamic_cast<Gun*>(&node->get_absolute_observer());
    if (gun == nullptr) {
        THROW_OR_ABORT("Absolute observer is not a gun");
    }
    return *gun;
}
