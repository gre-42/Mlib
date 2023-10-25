#include "Download_As_Stb_Image.hpp"
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/CHK.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

void my_stbi_write_func(void* context, void* data, int size) {
    auto* op = reinterpret_cast<uint8_t*>(context);
    auto* ip = reinterpret_cast<uint8_t*>(data);
    std::copy(ip, ip + size, op);
}

StbInfo<uint8_t> Mlib::download_as_stb_image(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    int nchannels)
{
    GLenum format;
    switch (nchannels) {
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        THROW_OR_ABORT("download_as_stb_image: Unsupported number of channels (" + std::to_string(nchannels) + ')');
    };
    // From: http://www.songho.ca/opengl/gl_pbo.html
    std::vector<uint8_t> data(width * height * nchannels);
    CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer));
    DestructionGuard g{[](){CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));}};
    CHK(glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, data.data()));
    // Unbinding is done by the "DestructionGuard" above.
    auto png = stb_create<uint8_t>(width, height, nchannels);
    if (stbi_write_png_to_func(
        my_stbi_write_func,
        png.data.get(),
        integral_cast<int>(width),
        integral_cast<int>(height),
        integral_cast<int>(nchannels),
        data.data(),
        0) == 0)
    {
        THROW_OR_ABORT("Could not convert texture to PNG");
    }
    return png;
}
