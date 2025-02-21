#include "Charsets.hpp"
#include <Mlib/Iterator/Enumerate.hpp>

using namespace Mlib;

std::wstring Mlib::ascii_chars() {
    std::wstring result(95, '?');
    for (auto&& [i, r] : enumerate(result)) {
        r = i + 32;
    }
    return result;
}
