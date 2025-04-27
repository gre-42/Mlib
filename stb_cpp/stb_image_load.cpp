#include "stb_image_load.hpp"
#include <Mlib/Features.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdlib>
#include <stb/stb_image_bpc.h>

#ifdef __ANDROID__
#include <Mlib/Os/Os.hpp>
#endif

#ifdef WITHOUT_THREAD_LOCAL
#include <mutex>
#endif

using namespace Mlib;

template <class TData>
void stb_image_flip_horizontally(StbInfo<TData>& image) {
    for (size_t r = 0; r < (size_t)image.height; ++r) {
        for (size_t c = 0; c < (size_t)image.width / 2; ++c) {
            for (size_t d = 0; d < (size_t)image.nrChannels; ++d) {
                std::swap(
                    image.data()[(r * (size_t)image.width + c) * (size_t)image.nrChannels + d],
                    image.data()[(r * (size_t)image.width + (size_t)image.width - 1 - c) * (size_t)image.nrChannels + d]);
            }
        }
    }
}

template <class TData>
static StbInfo<TData> stb_wrap_and_postprocess(TData* data, int width, int height, int nrChannels, bool flip_horizontally) {
    auto result = StbInfo<TData>(
        width,
        height,
        nrChannels,
        (TData*)data);
    if (flip_horizontally) {
        stb_image_flip_horizontally(result);
    }
    return result;
}

std::variant<StbInfo<uint8_t>, StbInfo<uint16_t>> stb_load(
    const std::string& filename,
    FlipMode flip_mode,
    const std::vector<std::byte>* data)
{
    void* image;
    int width;
    int height;
    int nrChannels;
    int bytes_per_pixel;

#ifdef WITHOUT_THREAD_LOCAL
    static std::mutex mutex;
    std::scoped_lock lock{mutex};
    stbi_set_flip_vertically_on_load(any(flip_mode & FlipMode::VERTICAL));
#else
    stbi_set_flip_vertically_on_load_thread(any(flip_mode & FlipMode::VERTICAL));
#endif
    if (data != nullptr) {
        if (data->size() > INT_MAX) {
            THROW_OR_ABORT("File too large");
        }
        image = stbi_load_from_memory_bpc(
            (const uint8_t*)data->data(),
            (int)data->size(),
            &width,
            &height,
            &nrChannels,
            0,
            &bytes_per_pixel);
    } else {
#ifdef __ANDROID__
        std::vector<uint8_t> buffer = Mlib::read_file_bytes(filename);
        if (buffer.size() > INT_MAX) {
            THROW_OR_ABORT("File too large");
        }
        image = stbi_load_from_memory_bpc(
            buffer.data(),
            (int)buffer.size(),
            &width,
            &height,
            &nrChannels,
            0,
            &bytes_per_pixel);
#else
        image = stbi_load_bpc(
                filename.c_str(),
                &width,
                &height,
                &nrChannels,
                0,
                &bytes_per_pixel);
#endif
    }
    if (image == nullptr) {
        THROW_OR_ABORT("Could not load \"" + filename + '"');
    }
    if (bytes_per_pixel == 8) {
        return stb_wrap_and_postprocess((uint8_t*)image, width, height, nrChannels, any(flip_mode & FlipMode::HORIZONTAL));
    } else if (bytes_per_pixel == 16) {
        return stb_wrap_and_postprocess((uint16_t*)image, width, height, nrChannels, any(flip_mode & FlipMode::HORIZONTAL));
    } else {
        stbi_image_free(image);
        THROW_OR_ABORT("Unsupported image data size");
    }
}

StbInfo<uint8_t> stb_load8(
    const std::string& filename,
    FlipMode flip_mode,
    const std::vector<std::byte>* data,
    IncorrectDatasizeBehavior datasize_behavior)
{
    auto res = stb_load(filename, flip_mode, data);
    auto* res8 = std::get_if<StbInfo<uint8_t>>(&res);
    if (res8 == nullptr) {
        if (datasize_behavior == IncorrectDatasizeBehavior::THROW) {
            THROW_OR_ABORT("Image \"" + filename + "\" does not have 8 bits");
        } else if (datasize_behavior == IncorrectDatasizeBehavior::CONVERT) {
            auto* res16 = std::get_if<StbInfo<uint16_t>>(&res);
            if (res16 == nullptr) {
                THROW_OR_ABORT("Image has neither 8 nor 16 bits");
            }
            auto conv8 = StbInfo<uint8_t>(res16->width, res16->height, res16->nrChannels);
            auto* d16 = res16->data();
            auto* d8 = conv8.data();
            for (; d8 != conv8.data() + conv8.width * conv8.height * conv8.nrChannels; ++d8, ++d16) {
                *d8 = (*d16 >> 8);
            }
            return conv8;
        } else {
            THROW_OR_ABORT("Unknown data-size behavior");
        }
    }
    return std::move(*res8);
}
