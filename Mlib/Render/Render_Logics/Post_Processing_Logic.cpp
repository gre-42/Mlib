#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Post_Processing_Logic.hpp"
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/RenderGuards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Log.hpp>

using namespace Mlib;

/**
 * From: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
 *
 * https://stackoverflow.com/questions/6408851/draw-the-depth-value-in-opengl-using-shaders/6409229#6409229
 */
static GenShaderText fragment_shader_text{[](
    const std::vector<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices,
    bool low_pass,
    bool high_pass,
    bool depth_fog)
{
    if (low_pass && high_pass) {
        throw std::runtime_error("Only one of low_pass and high_pass can be specified");
    }
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screenTextureColor;" << std::endl;
    if (low_pass || depth_fog) {
        sstr << "uniform float zNear;" << std::endl;
        sstr << "uniform float zFar;" << std::endl;
        sstr << "uniform sampler2D screenTextureDepth;" << std::endl;
    }
    if (depth_fog) {
        sstr << "uniform vec3 backgroundColor;" << std::endl;
    }
    sstr << std::endl;
    sstr << "const float offset = 1.0 / 1000.0;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (low_pass || depth_fog) {
        // sstr << "    vec3 col = texture(screenTextureColor, TexCoords).rgb;" << std::endl;
        sstr << "    float z_b = texture(screenTextureDepth, TexCoords).x;" << std::endl;
        sstr << "    float z_n = 2.0 * z_b - 1.0;" << std::endl;
        sstr << "    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));" << std::endl;
        // sstr << "    col.b = (z_e - zNear) / (zFar - zNear);" << std::endl;
        sstr << "    float distance_fac = clamp((z_e - 200) / 800, 0, 0.5);" << std::endl;
    }
    // sstr << "    if (-z_e > 100) col.b = 1;" << std::endl;
    // sstr << "    if (-z_e <= 100) col.b = 0;" << std::endl;
    // sstr << "    col.b = 10 * (z_b - 0.94);" << std::endl;
    // sstr << "    FragColor = vec4(col, 1.0);" << std::endl;
    if (low_pass || high_pass) {
        sstr << "    vec2 offsets[9] = vec2[](" << std::endl;
        sstr << "        vec2(-offset,  offset), // top-left" << std::endl;
        sstr << "        vec2( 0.0f,    offset), // top-center" << std::endl;
        sstr << "        vec2( offset,  offset), // top-right" << std::endl;
        sstr << "        vec2(-offset,  0.0f),   // center-left" << std::endl;
        sstr << "        vec2( 0.0f,    0.0f),   // center-center" << std::endl;
        sstr << "        vec2( offset,  0.0f),   // center-right" << std::endl;
        sstr << "        vec2(-offset, -offset), // bottom-left" << std::endl;
        sstr << "        vec2( 0.0f,   -offset), // bottom-center" << std::endl;
        sstr << "        vec2( offset, -offset)  // bottom-right" << std::endl;
        sstr << "    );" << std::endl;
        sstr << std::endl;
        if (low_pass) {
            sstr << "    float c = 1.f / 9 * distance_fac + 1 * (1 - distance_fac);" << std::endl;
            sstr << "    float o = 1.f / 9 * distance_fac + 0 * (1 - distance_fac);" << std::endl;
        }
        if (high_pass) {
            sstr << "    float c =  9 * 0.2 + 1. / 9 * 0.8;" << std::endl;
            sstr << "    float o = -1 * 0.2 + 1. / 9 * 0.8;" << std::endl;
        }
        sstr << "    float kernel[9] = float[](" << std::endl;
        sstr << "        o, o, o," << std::endl;
        sstr << "        o, c, o," << std::endl;
        sstr << "        o, o, o" << std::endl;
        sstr << "    );" << std::endl;
        sstr << std::endl;
        sstr << "    vec3 sampleTex[9];" << std::endl;
        sstr << "    for (int i = 0; i < 9; i++)" << std::endl;
        sstr << "    {" << std::endl;
        sstr << "        sampleTex[i] = vec3(texture(screenTextureColor, TexCoords.st + offsets[i]));" << std::endl;
        sstr << "    }" << std::endl;
        sstr << "    vec3 col = vec3(0);" << std::endl;
        sstr << "    for (int i = 0; i < 9; i++)" << std::endl;
        sstr << "    {" << std::endl;
        sstr << "        col += sampleTex[i] * kernel[i];" << std::endl;
        sstr << "    }" << std::endl;
        sstr << std::endl;
    } else {
        sstr << "    vec3 col = vec3(texture(screenTextureColor, TexCoords.st));" << std::endl;
    }
    if (depth_fog) {
        sstr << "    col = 0.5 * distance_fac * backgroundColor + (1 - 0.5 * distance_fac) * col;" << std::endl;
    }
    sstr << "    FragColor = vec4(col, 1);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

PostProcessingLogic::PostProcessingLogic(RenderLogic& child_logic, bool depth_fog, bool low_pass, bool high_pass)
: child_logic_{child_logic},
  depth_fog_{depth_fog},
  low_pass_{low_pass}
{
    rp_.generate(vertex_shader_text, fragment_shader_text({}, {}, {}, {}, low_pass_, high_pass, depth_fog_));

    // https://www.khronos.org/opengl/wiki/Example/Texture_Shader_Binding
    rp_.screen_texture_color_location = checked_glGetUniformLocation(rp_.program, "screenTextureColor");
    if (low_pass_ || depth_fog_) {
        rp_.screen_texture_depth_location = checked_glGetUniformLocation(rp_.program, "screenTextureDepth");
        rp_.z_near_location = checked_glGetUniformLocation(rp_.program, "zNear");
        rp_.z_far_location = checked_glGetUniformLocation(rp_.program, "zFar");
    } else {
        rp_.screen_texture_depth_location = 0;
        rp_.z_near_location = 0;
        rp_.z_far_location = 0;
    }
    if (depth_fog_) {
        rp_.background_color_location = checked_glGetUniformLocation(rp_.program, "backgroundColor");
    } else {
        rp_.background_color_location = 0;
    }
}

PostProcessingLogic::~PostProcessingLogic() = default;

void PostProcessingLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    // TimeGuard time_guard{"PostProcessingLogic::render", "PostProcessingLogic::render"};
    LOG_FUNCTION("PostProcessingLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::UNDEFINED) {
        throw std::runtime_error("PostProcessingLogic did not receive undefined rendering");
    }
    if (!child_logic_.requires_postprocessing()) {
        child_logic_.render(
            width,
            height,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    } else {
        assert_true(render_config.nsamples_msaa > 0);

        fbs_.configure({.width = width, .height = height, .with_depth_texture = true, .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{fbs_};
            RenderedSceneDescriptor fid{.external_render_pass = {ExternalRenderPassType::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = 0, .light_node_name = ""};
            child_logic_.render(
                width,
                height,
                render_config,
                scene_graph_config,
                render_results,
                fid);
        }

        // Now draw a quad plane with the attached framebuffer color texture
        // Depth test should already be disabled
        // CHK(glDisable(GL_DEPTH_TEST)); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        // CHK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f)); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
        // CHK(glClear(GL_COLOR_BUFFER_BIT));

        {
            RenderToScreenGuard rgb;
            CHK(glUseProgram(rp_.program));

            CHK(glUniform1i(rp_.screen_texture_color_location, 0));
            if (depth_fog_ || low_pass_) {
                CHK(glUniform1i(rp_.screen_texture_depth_location, 1));
                CHK(glUniform1f(rp_.z_near_location, near_plane()));
                CHK(glUniform1f(rp_.z_far_location, far_plane()));
            }
            if (depth_fog_) {
                CHK(glUniform3fv(rp_.background_color_location, 1, (GLfloat*)&render_config.background_color));
            }
            CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
            CHK(glBindTexture(GL_TEXTURE_2D, fbs_.fb.texture_color));  // use the color attachment texture as the texture of the quad plane

            if (depth_fog_ || low_pass_) {
                CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                CHK(glBindTexture(GL_TEXTURE_2D, fbs_.fb.texture_depth));
            }

            CHK(glBindVertexArray(va_.vertex_array));
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            // Reset to defaults
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
}

float PostProcessingLogic::near_plane() const {
    return child_logic_.near_plane();
}

float PostProcessingLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<float, 4, 4>& PostProcessingLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, 3>& PostProcessingLogic::iv() const {
    return child_logic_.iv();
}

bool PostProcessingLogic::requires_postprocessing() const {
    return false;
}
