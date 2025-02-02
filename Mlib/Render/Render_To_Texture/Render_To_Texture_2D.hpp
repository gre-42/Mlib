#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <functional>

namespace Mlib {

GLuint render_to_texture_2d(
    GLsizei width,
    GLsizei height,
    GLsizei mip_level_count,
    float anisotropic_filtering_level,
    GLenum internalformat,
    const std::function<void(GLsizei width, GLsizei height)>& render);

}
