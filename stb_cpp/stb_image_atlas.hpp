#pragma once
#include "stb_image_load.hpp"
#include <cstdlib>
#include <string>
#include <vector>

struct AtlasTile {
    int left;
    int bottom;
    size_t layer;
    StbInfo<uint8_t> image;
};

void build_image_atlas(
    const std::vector<StbInfo<uint8_t>>& atlas,
    const std::vector<AtlasTile>& tiles);
