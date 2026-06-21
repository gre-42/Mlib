#pragma once
#include "Binary_Bitwise_Words_Writer.hpp"
#include <Mlib/Os/Io/Serialize/Serialize.hpp>

namespace Mlib {

void WritingArchive::operator () (const auto& element) {
    save(writer_, ctx_, element, message_);
}

}
