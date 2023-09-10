#include "Iterate_Over_Chunks_Of_String.hpp"

using namespace Mlib;

std::list<std::string_view> Mlib::iterate_over_blocks_of_string(
    std::string_view s,
    size_t block_size)
{
    std::list<std::string_view> result;
    for (size_t i = 0; i < s.length(); i += block_size) {
        result.push_back(s.substr(i, std::min(s.length(), i + block_size)));
    }
    return result;
}
