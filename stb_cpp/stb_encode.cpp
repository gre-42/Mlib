#include "stb_encode.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb/stb_image_write.h>

using namespace Mlib;

void my_stbi_write_func(void* context, void* data, int size) {
    auto* op = reinterpret_cast<std::vector<uint8_t>*>(context);
    auto* ip = reinterpret_cast<uint8_t*>(data);
    op->resize(integral_cast<size_t>(size));
    std::copy(ip, ip + size, op->data());
}

std::vector<std::byte> stb_encode_png(
    const uint8_t* data,
    int width,
    int height,
    int nchannels)
{
    std::vector<std::byte> result;
    if (stbi_write_png_to_func(
        my_stbi_write_func,
        &result,
        width,
        height,
        nchannels,
        data,
        0) == 0)
    {
        THROW_OR_ABORT("Could not convert texture to PNG");
    }
    return result;
}
