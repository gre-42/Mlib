#include "Download_As_Stb_Image.hpp"
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/CHK.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

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
    auto result = stb_create<uint8_t>(
        integral_cast<int>(width),
        integral_cast<int>(height),
        nchannels);
    CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer));
    DestructionGuard g{[](){CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));}};
    CHK(glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, result.data.get()));
    // Unbinding is done by the "DestructionGuard" above.
    return result;
}
