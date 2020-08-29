#pragma once
#include <stb_image/stb_image.h>
#include <memory>
#include <string>

struct StbInfo {
    int width;
    int height;
    int nrChannels;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> data{nullptr, &stbi_image_free};
};

void stb_image_flip_horizontally(const StbInfo& image) {
    for(size_t r = 0; r < (size_t)image.height; ++r) {
        for(size_t c = 0; c < (size_t)image.width / 2; ++c) {
            for(size_t d = 0; d < (size_t)image.nrChannels; ++d) {
                std::swap(
                    image.data.get()[(r * image.width  + c) * image.nrChannels + d],
                    image.data.get()[(r * image.width  + image.width - 1 - c) * image.nrChannels + d]);
            }
        }
    }
}

static StbInfo stb_load(const std::string& filename, bool flip_vertically, bool flip_horizontally) {
    StbInfo result;

    stbi_set_flip_vertically_on_load_thread(flip_vertically);
    result.data.reset(stbi_load(
            filename.c_str(),
            &result.width,
            &result.height,
            &result.nrChannels,
            0));
    if (flip_horizontally) {
        stb_image_flip_horizontally(result);
    }
    return result;
}
