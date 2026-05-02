#pragma once
#include <compare>
#include <string>

namespace Mlib {

std::strong_ordering operator <=> (const std::string& a, const std::string& b);

}
