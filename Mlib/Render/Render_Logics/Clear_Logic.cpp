#include "Clear_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Shader_Version.hpp>

using namespace Mlib;

static const char* plain_vertex_shader_text =
SHADER_VER
"layout (location = 0) in vec2 aPos;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"}";

static const char* clear_color_only_fragment_shader_text =
SHADER_VER FRAGMENT_PRECISION
"out vec4 fragColor;\n"
"\n"
"uniform vec4 clearColor;\n"
"\n"
"void main() {\n"
"    fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n";

static const char* clear_depth_only_fragment_shader_text =
SHADER_VER FRAGMENT_PRECISION
"\n"
"void main() {\n"
"    gl_FragDepth = 1.0;\n"
"}\n";

static const char* clear_color_and_depth_fragment_shader_text =
SHADER_VER FRAGMENT_PRECISION
"out vec4 fragColor;\n"
"\n"
"uniform vec4 clearColor;\n"
"\n"
"void main() {\n"
"    fragColor = clearColor;\n"
"    gl_FragDepth = 1.0;\n"
"}\n";

ClearLogic::ClearLogic() = default;

ClearLogic::~ClearLogic() = default;

void ClearLogic::ensure_va_initialized() {
    if (va_.vertex_array == (GLuint)-1) {
        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions
            -1.0f,  1.0f,
            -1.0f, -1.0f,
            1.0f, -1.0f,

            -1.0f,  1.0f,
            1.0f, -1.0f,
            1.0f,  1.0f,
        };

        CHK(glGenVertexArrays(1, &va_.vertex_array));
        CHK(glGenBuffers(1, &va_.vertex_buffer));
        CHK(glBindVertexArray(va_.vertex_array));
        CHK(glBindBuffer(GL_ARRAY_BUFFER, va_.vertex_buffer));
        CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW));
        CHK(glEnableVertexAttribArray(0));
        CHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr));
        CHK(glBindVertexArray(0));
    }
}

void ClearLogic::clear_color(const FixedArray<float, 4>& color) {
    std::lock_guard lock{mutex_};
    ensure_va_initialized();
    if (rp_color_only_.program == (GLuint)-1) {
        rp_color_only_.allocate(plain_vertex_shader_text, clear_color_only_fragment_shader_text);
        rp_color_only_.clear_color_location = checked_glGetUniformLocation(rp_color_only_.program, "clearColor");
    }
    CHK(glUseProgram(rp_color_only_.program));
    CHK(glUniform4fv(rp_color_only_.clear_color_location, 1, color.flat_begin()));
    CHK(glBindVertexArray(va_.vertex_array));
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
}

void ClearLogic::clear_depth() {
    std::lock_guard lock{mutex_};
    ensure_va_initialized();
    if (rp_depth_only_.program == (GLuint)-1) {
        rp_depth_only_.allocate(plain_vertex_shader_text, clear_depth_only_fragment_shader_text);
    }
    CHK(glDepthFunc(GL_ALWAYS));
    // From: https://forum.babylonjs.com/t/modifying-the-depth-buffer-during-the-post-processing-pipeline/24832
    // ... only enabling depth writing is not enough for gl_FragDepth to work,
    // you must also enable depth testing ...
    CHK(glEnable(GL_DEPTH_TEST));

    CHK(glUseProgram(rp_depth_only_.program));
    CHK(glBindVertexArray(va_.vertex_array));
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));

    CHK(glDepthFunc(GL_LESS));
    CHK(glDisable(GL_DEPTH_TEST));
}

void ClearLogic::clear_color_and_depth(const FixedArray<float, 4>& color) {
    std::lock_guard lock{mutex_};
    ensure_va_initialized();
    if (rp_color_and_depth_.program == (GLuint)-1) {
        rp_color_and_depth_.allocate(plain_vertex_shader_text, clear_color_and_depth_fragment_shader_text);
        rp_color_and_depth_.clear_color_location = checked_glGetUniformLocation(rp_color_and_depth_.program, "clearColor");
    }
    CHK(glDepthFunc(GL_ALWAYS));
    // From: https://forum.babylonjs.com/t/modifying-the-depth-buffer-during-the-post-processing-pipeline/24832
    // ... only enabling depth writing is not enough for gl_FragDepth to work,
    // you must also enable depth testing ...
    CHK(glEnable(GL_DEPTH_TEST));

    CHK(glUseProgram(rp_color_and_depth_.program));
    CHK(glUniform4fv(rp_color_and_depth_.clear_color_location, 1, color.flat_begin()));
    CHK(glBindVertexArray(va_.vertex_array));
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));

    CHK(glDepthFunc(GL_LESS));
    CHK(glDisable(GL_DEPTH_TEST));
}
