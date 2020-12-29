#pragma once
#include "stb_image.h"
#include <memory>
#include <string>

struct StbInfo {
    int width;
    int height;
    int nrChannels;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> data{nullptr, &stbi_image_free};
};

void stb_image_flip_horizontally(const StbInfo& image);

StbInfo stb_load(const std::string& filename, bool flip_vertically, bool flip_horizontally);
