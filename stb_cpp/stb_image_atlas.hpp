#pragma once
#include "stb_image_load.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

struct AtlasTileTarget {
    int left;
    int bottom;
    size_t layer;
};

struct AtlasTileSource {
    int left;
    int bottom;
    int width;
    int height;
    const StbInfo<uint8_t>& image;
};

struct AtlasTile {
    AtlasTileSource source;
    AtlasTileTarget target;
};

void build_image_atlas(
    const std::vector<std::shared_ptr<StbInfo<uint8_t>>>& atlas,
    const std::vector<AtlasTile>& tiles);
