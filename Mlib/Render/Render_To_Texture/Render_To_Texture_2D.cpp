#include "Render_To_Texture_2D.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Gl_Extensions.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_2D.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>

using namespace Mlib;

GLuint Mlib::render_to_texture_2d(
    GLsizei width,
    GLsizei height,
    GLsizei mip_level_count,
    float anisotropic_filtering_level,
    GLenum internalformat,
    const std::function<void(GLsizei width, GLsizei height)>& render)
{
    GLuint texture;
    CHK(glGenTextures(1, &texture));
    CHK(glBindTexture(GL_TEXTURE_2D, texture));
    CHK(glTexStorage2D(
        GL_TEXTURE_2D,
        mip_level_count + 1,
        internalformat,
        width,
        height));
    if (anisotropic_filtering_level != 0) {
        CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_filtering_level));
    }
    CHK(glBindTexture(GL_TEXTURE_2D, 0));
    for (auto level = 0; level <= mip_level_count; ++level) {
        auto afbs = std::make_shared<FrameBufferStorage2D>(texture, level);
        RenderToFrameBufferGuard rfg{ afbs };
        render(width, height);
        width = std::max(1, width / 2);
        height = std::max(1, height / 2);
        // // Disable the ArrayFrameBufferStorage above for the following code to work.
        // static SaveMovie save_movie;
        // save_movie.save(
        //     "/tmp/atlas_",
        //     "_layer",
        //     integral_cast<size_t>(adesc.width / (1 << level)),
        //     integral_cast<size_t>(adesc.height / (1 << level)));
    }
    return texture;
}
