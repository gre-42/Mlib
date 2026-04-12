#pragma once
#include <Mlib/Os/Utf8_Path.hpp>
#include <iosfwd>
#include <sstream>
#include <string>

namespace Mlib {

std::istringstream uncompress_stream(
    std::istream& istr,
    const std::string& filename,
    std::streamoff nbytes,
    size_t chunk_size = 512);

void decompress_file(
    const Utf8Path& source,
    const Utf8Path& destination);

}
