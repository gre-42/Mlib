#pragma once
#include <ios>

namespace Mlib {

struct StreamSegment {
    std::streamoff offset;
    std::streamsize size;
};

}
