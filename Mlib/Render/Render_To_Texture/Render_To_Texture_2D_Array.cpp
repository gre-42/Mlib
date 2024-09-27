#include "Render_To_Texture_2D_Array.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Gl_Extensions.hpp>
#include <Mlib/Render/Instance_Handles/Array_Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>

using namespace Mlib;

GLuint Mlib::render_to_texture_2d_array(
    GLsizei width,
    GLsizei height,
    GLsizei depth,
    GLsizei mip_level_count,
    float anisotropic_filtering_level,
    GLenum internalformat,
    const std::function<void(GLsizei width, GLsizei height, GLsizei layer)>& render)
{
    GLuint texture;
    CHK(glGenTextures(1, &texture));
    CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, texture));
    CHK(glTexStorage3D(
        GL_TEXTURE_2D_ARRAY,
        mip_level_count + 1,
        internalformat,
        width,
        height,
        depth));
    if (anisotropic_filtering_level != 0) {
        CHK(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_filtering_level));
    }
    CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
    for (auto layer = 0; layer < depth; ++layer) {
        auto w = width;
        auto h = height;
        for (auto level = 0; level <= mip_level_count; ++level) {
            auto afbs = std::make_shared<ArrayFrameBufferStorage>(texture, level, integral_cast<int>(layer));
            RenderToFrameBufferGuard rfg{ afbs };
            render(w, h, layer);
            w = std::max(1, w / 2);
            h = std::max(1, h / 2);
            // // Disable the ArrayFrameBufferStorage above for the following code to work.
            // static SaveMovie save_movie;
            // save_movie.save(
            //     "/tmp/atlas_",
            //     "_layer",
            //     integral_cast<size_t>(adesc.width / (1 << level)),
            //     integral_cast<size_t>(adesc.height / (1 << level)));
        }
    }
    return texture;
}
