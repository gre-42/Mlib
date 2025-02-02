#pragma once
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace Mlib {

struct DdsInfo {
    int width;
    int height;

    static DdsInfo load_from_file(const std::string& filename);
    static DdsInfo load_from_stream(std::istream& istream);
    static DdsInfo load_from_buffer(const std::vector<std::byte>& buffer);
};

}
