#pragma once
#include "stb_image_load.hpp"
#include <cstdlib>
#include <string>
#include <vector>

struct AtlasTile {
    int left;
    int bottom;
    StbInfo image;
};

void build_image_atlas(StbInfo& atlas, const std::vector<AtlasTile>& tiles);
