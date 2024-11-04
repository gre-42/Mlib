#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

std::vector<std::byte> stb_encode_png(
    const uint8_t* data,
    int width,
    int height,
    int nchannels);
