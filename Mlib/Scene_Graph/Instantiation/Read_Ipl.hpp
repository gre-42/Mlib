#pragma once
#include <filesystem>
#include <iosfwd>
#include <list>

namespace Mlib {

struct InstanceInformation;
enum class RenderingDynamics;

std::list<InstanceInformation> read_ipl(
    const std::filesystem::path& filename,
    RenderingDynamics rendering_dynamics);

std::list<InstanceInformation> read_ipl(
    std::istream& istr,
    RenderingDynamics rendering_dynamics);

}
