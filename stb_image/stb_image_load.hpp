#pragma once
#include "stb_image.h"
#include <memory>
#include <string>

#if defined(STBI_MALLOC) || defined(STBI_FREE) || defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED)
#error Do not define STBI_MALLOC etc.
#endif

struct StbInfo {
    int width;
    int height;
    int nrChannels;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> data{nullptr, &stbi_image_free};
};

void stb_image_flip_horizontally(const StbInfo& image);

StbInfo stb_load(const std::string& filename, bool flip_vertically, bool flip_horizontally);
StbInfo stb_create(int width, int height, int nrChannels);
