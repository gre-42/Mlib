
#include "Gun.hpp"
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

DanglingBaseClassRef<Gun> Mlib::get_gun(
    SceneNode& node,
    SourceLocation loc)
{
    auto ao = node.get_absolute_observer();
    auto gun = dynamic_cast<Gun*>(&ao.get());
    if (gun == nullptr) {
        throw std::runtime_error("Absolute observer is not a gun");
    }
    return {*gun, loc};
}
