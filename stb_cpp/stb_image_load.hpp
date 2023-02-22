#pragma once
#include <cstdint>
#include <memory>
#include <stb/stb_image.h>
#include <string>
#include <variant>

#if defined(STBI_MALLOC) || defined(STBI_FREE) || defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED)
#error Do not define STBI_MALLOC etc.
#endif

template <class TData>
struct StbInfo {
    int width;
    int height;
    int nrChannels;
    std::unique_ptr<TData, decltype(&stbi_image_free)> data{nullptr, &stbi_image_free};
};

template <class TData>
void stb_image_flip_horizontally(const StbInfo<TData>& image);

std::variant<StbInfo<uint8_t>, StbInfo<uint16_t>> stb_load(const std::string& filename, bool flip_vertically, bool flip_horizontally);
StbInfo<uint8_t> stb_load8(const std::string& filename, bool flip_vertically, bool flip_horizontally);
template <class TData>
StbInfo<TData> stb_create(int width, int height, int nrChannels);
