#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

struct ImageInfo {
    static ImageInfo load(const std::string& filename, const std::vector<uint8_t>* data);
    FixedArray<size_t, 2> size;
};

};
