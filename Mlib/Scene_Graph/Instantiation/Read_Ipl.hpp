#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <filesystem>
#include <iosfwd>
#include <list>

namespace Mlib {

template <class TPosition>
struct InstanceInformation;
enum class RenderingDynamics;

std::list<InstanceInformation<ScenePos>> read_ipl(
    const std::filesystem::path& filename,
    RenderingDynamics rendering_dynamics);

std::list<InstanceInformation<ScenePos>> read_ipl(
    std::istream& istr,
    RenderingDynamics rendering_dynamics);

}
