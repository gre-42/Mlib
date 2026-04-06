#include "stb_rotate.hpp"
#include <stb_cpp/stb_image_load.hpp>

StbInfo<unsigned char> stb_rotate_90(const StbInfo<unsigned char>& data)
{
    auto result = StbInfo<unsigned char>{data.height, data.width, data.nrChannels};
    for (int r = 0; r < data.height; ++r) {
        for (int c = 0; c < data.width; ++c) {
            for (int d = 0; d < data.nrChannels; ++d) {
                result(data.height - r - 1, c, d) = data(c, r, d);
            }
        }
    }
    return result;
}

StbInfo<unsigned char> stb_rotate_180(const StbInfo<unsigned char>& data)
{
    auto result = StbInfo<unsigned char>{data.width, data.height, data.nrChannels};
    for (int r = 0; r < data.height; ++r) {
        for (int c = 0; c < data.width; ++c) {
            for (int d = 0; d < data.nrChannels; ++d) {
                result(data.width - c - 1, data.height - r - 1, d) = data(c, r, d);
            }
        }
    }
    return result;
}

StbInfo<unsigned char> stb_rotate_270(const StbInfo<unsigned char>& data)
{
    auto result = StbInfo<unsigned char>{data.height, data.width, data.nrChannels};
    for (int r = 0; r < data.height; ++r) {
        for (int c = 0; c < data.width; ++c) {
            for (int d = 0; d < data.nrChannels; ++d) {
                result(r, data.width - c - 1, d) = data(c, r, d);
            }
        }
    }
    return result;
}

StbInfo<unsigned char> stb_rotate(
    const StbInfo<unsigned char>& data,
    int degrees)
{
    switch (degrees) {
        case 90: return stb_rotate_90(data);
        case 180: return stb_rotate_180(data);
        case 270: return stb_rotate_270(data);
    }
    throw std::runtime_error("Can only rotate by 90, 180 or 270 degrees. Requested: " + std::to_string(degrees));
}
