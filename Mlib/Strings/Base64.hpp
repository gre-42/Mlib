#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

std::vector<std::byte> decode_base64(const std::string& base64_str);

}
