#pragma once
#include "Binary_Bitwise_Words_Reader.hpp"
#include <Mlib/Os/Io/Serialize/Serialize.hpp>

namespace Mlib {

void ReadingArchive::operator () (auto& element) {
    using Element = std::remove_reference_t<decltype(element)>;
    element = Mlib::load<Element>(reader_, ctx_, message_);
}

}
