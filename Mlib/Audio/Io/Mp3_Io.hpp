#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <vector>

namespace Mlib {

void convert_to_mp3(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    uint32_t bitrate = 0);

void convert_to_mp3(
    std::basic_istream<int16_t>& source,
    std::ostream& destination,
    uint32_t bitrate = 0);

std::vector<std::byte> convert_to_mp3(
    const std::vector<int16_t>& pcm,
    uint32_t bitrate = 0);

}
