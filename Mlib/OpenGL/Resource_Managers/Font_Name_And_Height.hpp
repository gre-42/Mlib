#pragma once
#include <Mlib/Hashing/Cached_Hash.hpp>
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

struct FontNameAndHeight {
    VariableAndHash<std::string> charset;
    std::string ttf_filename;
    float height_pixels;
    CachedHash hash;
    FontNameAndHeight& compute_hash();
    inline bool operator == (const FontNameAndHeight& other) const {
        return hash.get() == other.hash.get();
    }
};

}
