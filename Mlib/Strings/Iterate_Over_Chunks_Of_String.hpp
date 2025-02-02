#pragma once
#include <cstddef>
#include <list>
#include <string_view>

namespace Mlib {

std::list<std::string_view> iterate_over_blocks_of_string(
    std::string_view s,
    size_t block_size);

}
