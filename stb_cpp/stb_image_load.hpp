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

enum class FlipMode {
    NONE = 0,
    HORIZONTAL = 1 << 0,
    VERTICAL = 1 << 1
};

inline bool any(FlipMode m) {
    return m != FlipMode::NONE;
}

inline FlipMode operator & (FlipMode a, FlipMode b) {
    return (FlipMode)((int)a & (int)b);
}

inline FlipMode operator | (FlipMode a, FlipMode b) {
    return (FlipMode)((int)a | (int)b);
}

template <class TData>
void stb_image_flip_horizontally(const StbInfo<TData>& image);

std::variant<StbInfo<uint8_t>, StbInfo<uint16_t>> stb_load(const std::string& filename, FlipMode flip_mode);
StbInfo<uint8_t> stb_load8(const std::string& filename, FlipMode flip_mode);
template <class TData>
StbInfo<TData> stb_create(int width, int height, int nrChannels);
