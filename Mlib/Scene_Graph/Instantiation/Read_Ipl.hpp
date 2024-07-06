#pragma once
#include <filesystem>
#include <iosfwd>
#include <list>

namespace Mlib {

struct InstanceInformation;

std::list<InstanceInformation> read_ipl(const std::filesystem::path& filename);
std::list<InstanceInformation> read_ipl(std::istream& istr);

}
