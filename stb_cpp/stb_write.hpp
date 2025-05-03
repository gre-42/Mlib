#pragma once
#include <cstdint>
#include <string>

void stb_write_png(
    const std::string& filename,
    int width,
    int height,
    int nrChannels,
    const uint8_t* data);
