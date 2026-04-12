#pragma once
#include <Mlib/Os/Utf8_Path.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <vector>

namespace Mlib {

void convert_to_mp3(
    const Utf8Path& source,
    const Utf8Path& destination,
    uint32_t bitrate = 0);

void convert_to_mp3(
    std::basic_istream<int16_t>& source,
    std::ostream& destination,
    uint32_t bitrate = 0);

std::vector<std::byte> convert_to_mp3(
    const std::vector<int16_t>& pcm,
    uint32_t bitrate = 0);

}
