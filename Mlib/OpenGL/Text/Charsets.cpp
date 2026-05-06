#include "Charsets.hpp"
#include <Mlib/Iterator/Enumerate.hpp>

using namespace Mlib;

std::u32string Mlib::ascii_chars() {
    std::u32string result(95, '?');
    for (auto&& [i, r] : enumerate(result)) {
        r = (uint32_t)i + 32;
    }
    return result;
}
