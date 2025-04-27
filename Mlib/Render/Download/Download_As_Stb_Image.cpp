#include "Download_As_Stb_Image.hpp"
#include <Mlib/Images/Flip_Mode.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/CHK.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

StbInfo<uint8_t> Mlib::download_as_stb_image(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    int nchannels,
    FlipMode flip_mode)
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
    Array<uint8_t> res{ ArrayShape{
        integral_cast<size_t>(height),
        integral_cast<size_t>(width),
        integral_cast<size_t>(nchannels)} };
    // From: http://www.songho.ca/opengl/gl_pbo.html
    CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer));
    DestructionGuard g{[](){CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));}};
    CHK(glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, res.flat_begin()));
    // Unbinding is done by the "DestructionGuard" above.
    if (any(flip_mode & FlipMode::HORIZONTAL)) {
        res.ref() = reverted_axis(res, 1);
    }
    if (any(flip_mode & FlipMode::VERTICAL)) {
        res.ref() = reverted_axis(res, 0);
    }
    auto result = StbInfo<uint8_t>(
        integral_cast<int>(width),
        integral_cast<int>(height),
        nchannels);
    std::copy(res.flat_begin(), res.flat_end(), result.data());
    return result;
}

Array<float> Mlib::color_to_array(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    int nchannels,
    FlipMode flip_mode)
{
    Array<float> res{ ArrayShape{
        integral_cast<size_t>(height),
        integral_cast<size_t>(width),
        integral_cast<size_t>(nchannels)} };
    CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer));
    DestructionGuard g{[](){CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));}};
    CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, res.flat_begin()));
    // Unbinding is done by the "DestructionGuard" above.
    if (any(flip_mode & FlipMode::HORIZONTAL)) {
        res.ref() = reverted_axis(res, 1);
    }
    if (any(flip_mode & FlipMode::VERTICAL)) {
        res.ref() = reverted_axis(res, 0);
    }
    return res;
}

Array<float> Mlib::depth_to_array(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    FlipMode flip_mode)
{
    Array<float> res{ ArrayShape{ size_t(height), size_t(width) } };
    CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer));
    DestructionGuard g{[](){CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));}};
    CHK(glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, res->flat_begin()));
    if (any(flip_mode & FlipMode::HORIZONTAL)) {
        res.ref() = reverted_axis(res, 1);
    }
    if (any(flip_mode & FlipMode::VERTICAL)) {
        res.ref() = reverted_axis(res, 0);
    }
    return res;
}
