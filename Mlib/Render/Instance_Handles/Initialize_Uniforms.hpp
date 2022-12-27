#pragma once
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

void initialize_uniforms(GLuint shader_program);
void initialize_uniform(GLuint shader_program, GLuint uniform_id);

}
