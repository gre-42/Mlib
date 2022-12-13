#include "stb_image_load.hpp"
#include <Mlib/Features.hpp>
#include <cstdlib>
#include <stdexcept>

#ifdef __ANDROID__
#include <Mlib/Os/Os.hpp>
#endif

#ifdef WITHOUT_THREAD_LOCAL
#include <mutex>
#endif

void stb_image_flip_horizontally(const StbInfo& image) {
    for (size_t r = 0; r < (size_t)image.height; ++r) {
        for (size_t c = 0; c < (size_t)image.width / 2; ++c) {
            for (size_t d = 0; d < (size_t)image.nrChannels; ++d) {
                std::swap(
                    image.data.get()[(r * image.width  + c) * image.nrChannels + d],
                    image.data.get()[(r * image.width  + image.width - 1 - c) * image.nrChannels + d]);
            }
        }
    }
}

StbInfo stb_load(const std::string& filename, bool flip_vertically, bool flip_horizontally) {
    StbInfo result;

#ifdef WITHOUT_THREAD_LOCAL
    static std::mutex mutex;
    std::lock_guard lock{mutex};
    stbi_set_flip_vertically_on_load(flip_vertically);
#else
    stbi_set_flip_vertically_on_load_thread(flip_vertically);
#endif
#ifdef __ANDROID__
    {
        std::vector<uint8_t> buffer = Mlib::read_file_bytes(filename);
        if (buffer.size() > INT_MAX) {
            throw std::runtime_error("File too large");
        }
        result.data.reset(stbi_load_from_memory(
            buffer.data(),
            (int)buffer.size(),
            &result.width,
            &result.height,
            &result.nrChannels,
            0));
    }
#else
    result.data.reset(stbi_load(
            filename.c_str(),
            &result.width,
            &result.height,
            &result.nrChannels,
            0));
#endif
    if (result.data == nullptr) {
        throw std::runtime_error("Could not load \"" + filename + '"');
    }
    if (flip_horizontally) {
        stb_image_flip_horizontally(result);
    }
    return result;
}

StbInfo stb_create(int width, int height, int nrChannels) {
    StbInfo result{
        .width = width,
        .height = height,
        .nrChannels = nrChannels
    };
    result.data.reset((unsigned char*)malloc(width * height * nrChannels));
    return result;
}
