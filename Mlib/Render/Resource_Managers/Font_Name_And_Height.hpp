#pragma once
#include <Mlib/Cached_Hash.hpp>
#include <string>

namespace Mlib {

struct FontNameAndHeight {
    std::string ttf_filename;
    float height_pixels;
    CachedHash hash;
    FontNameAndHeight& compute_hash();
    inline bool operator == (const FontNameAndHeight& other) const {
        return hash.get() == other.hash.get();
    }
};

}
