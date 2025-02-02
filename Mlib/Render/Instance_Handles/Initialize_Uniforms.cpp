#include "Initialize_Uniforms.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const size_t MAX_UNIFORM_NAME_LENGTH = 64;

void Mlib::initialize_uniforms(GLuint shader_program) {
    GLint uniform_count;
    CHK(glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &uniform_count));

    for (GLint uniform_id = 0; uniform_id < uniform_count; ++uniform_id) {
        initialize_uniform(shader_program, (GLuint)uniform_id);
    }
}

static void initialize_uniform_no_array(const char* uniform_name, GLuint uniform_type, GLint uniform_id) {
    try {
        switch (uniform_type) {
            case GL_FLOAT:
                CHK(glUniform1f(uniform_id, -1234.f));
                break;
            case GL_SAMPLER_2D:
                CHK(glUniform1i(uniform_id, 5));
                break;
            case GL_SAMPLER_CUBE:
                CHK(glUniform1i(uniform_id, 7));
                break;
            case GL_FLOAT_VEC2:
                {
                    float v[2] = {-34234.f};
                    CHK(glUniform2fv(uniform_id, 1, v));
                }
                break;
            case GL_FLOAT_VEC3:
                {
                    float v[3] = {-5433.f};
                    CHK(glUniform3fv(uniform_id, 1, v));
                }
                break;
            case GL_FLOAT_VEC4:
                {
                    float v[4] = {89243.f};
                    CHK(glUniform4fv(uniform_id, 1, v));
                }
                break;
            case GL_FLOAT_MAT3:
                {
                    float m[9] = {23475.f};
                    CHK(glUniformMatrix3fv(uniform_id, 1, GL_TRUE, m));
                }
                break;
            case GL_FLOAT_MAT4:
                {
                    float m[16] = {47334.f};
                    CHK(glUniformMatrix4fv(uniform_id, 1, GL_TRUE, m));
                }
                break;
            default:
                THROW_OR_ABORT("Uniform has unknown type: " + std::to_string(uniform_type));
        }
    } catch (const std::runtime_error& e) {
        THROW_OR_ABORT("Error initializing uniform \"" + std::string(uniform_name) + "\": " + e.what());
    }
}

void Mlib::initialize_uniform(GLuint shader_program, GLuint uniform_id) {
    GLsizei uniform_name_length;
    GLint uniform_size;
    GLenum uniform_type;
    GLchar uniform_name[MAX_UNIFORM_NAME_LENGTH];

    CHK(glGetActiveUniform(
        shader_program,
        uniform_id,
        MAX_UNIFORM_NAME_LENGTH,
        &uniform_name_length,
        &uniform_size,
        &uniform_type,
        uniform_name));
    // lerr() << "Initializing uniform id " << uniform_id << " with name " << uniform_name;

    std::string uniform_name_str{uniform_name};
    static const DECLARE_REGEX(regex, "^(\\w+)\\[0\\]$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(uniform_name_str, match, regex)) {
        for (GLint i = 0; i < uniform_size; ++i) {
            std::string uniform_name_str = (match[1].str() + '[' + std::to_string(i) + ']');
            GLint location = glGetUniformLocation(shader_program, uniform_name_str.c_str());
            if (location != -1) {
                initialize_uniform_no_array(uniform_name_str.c_str(), uniform_type, location);
            }
        }
    } else {
        initialize_uniform_no_array(
            uniform_name,
            uniform_type,
            checked_glGetUniformLocation(shader_program, uniform_name));
    }
}
