#include "Post_Processing_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Shader_Version.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

/**
 * https://stackoverflow.com/questions/6408851/draw-the-depth-value-in-opengl-using-shaders/6409229#6409229
 */
static GenShaderText fragment_shader_text{[](
    const std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::vector<BlendMapTexture*>& textures,
    bool low_pass,
    bool high_pass,
    bool depth_fog,
    bool soft_light)
{
    if (low_pass && high_pass) {
        THROW_OR_ABORT("Only one of low_pass and high_pass can be specified");
    }
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
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
    if (soft_light) {
        sstr << "uniform sampler2D softLightTexture;" << std::endl;
    }
    sstr << std::endl;
    sstr << "const float offset = 1.0 / 1000.0;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (low_pass || depth_fog) {
        // From: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
        // sstr << "    vec3 col = texture(screenTextureColor, TexCoords).rgb;" << std::endl;
        sstr << "    float z_b = texture(screenTextureDepth, TexCoords).x;" << std::endl;
        sstr << "    float z_n = 2.0 * z_b - 1.0;" << std::endl;
        sstr << "    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));" << std::endl;
        // sstr << "    col.b = (z_e - zNear) / (zFar - zNear);" << std::endl;
        sstr << "    float distance_fac = clamp((z_e - 200.0) / 800.0, 0.0, 0.5);" << std::endl;
    }
    // sstr << "    if (-z_e > 100) col.b = 1;" << std::endl;
    // sstr << "    if (-z_e <= 100) col.b = 0;" << std::endl;
    // sstr << "    col.b = 10 * (z_b - 0.94);" << std::endl;
    // sstr << "    FragColor = vec4(col, 1.0);" << std::endl;
    if (low_pass || high_pass) {
        sstr << "    vec2 offsets[9] = vec2[](" << std::endl;
        sstr << "        vec2(-offset,  offset), // top-left" << std::endl;
        sstr << "        vec2( 0.0,     offset), // top-center" << std::endl;
        sstr << "        vec2( offset,  offset), // top-right" << std::endl;
        sstr << "        vec2(-offset,  0.0),    // center-left" << std::endl;
        sstr << "        vec2( 0.0,     0.0),    // center-center" << std::endl;
        sstr << "        vec2( offset,  0.0),    // center-right" << std::endl;
        sstr << "        vec2(-offset, -offset), // bottom-left" << std::endl;
        sstr << "        vec2( 0.0,    -offset), // bottom-center" << std::endl;
        sstr << "        vec2( offset, -offset)  // bottom-right" << std::endl;
        sstr << "    );" << std::endl;
        sstr << std::endl;
        if (low_pass) {
            sstr << "    float c = 1.0 / 9.0 * distance_fac + 1.0 * (1.0 - distance_fac);" << std::endl;
            sstr << "    float o = 1.0 / 9.0 * distance_fac + 0.0 * (1.0 - distance_fac);" << std::endl;
        }
        if (high_pass) {
            sstr << "    float c =  9.0 * 0.2 + 1.0 / 9.0 * 0.8;" << std::endl;
            sstr << "    float o = -1.0 * 0.2 + 1.0 / 9.0 * 0.8;" << std::endl;
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
        sstr << "    col = 0.5 * distance_fac * backgroundColor + (1.0 - 0.5 * distance_fac) * col;" << std::endl;
    }
    if (soft_light) {
        // From: https://en.wikipedia.org/wiki/Blend_modes#Soft_Light
        sstr << "    vec3 a = col;" << std::endl;
        sstr << "    vec3 b = texture(softLightTexture, TexCoords.st).rgb;" << std::endl;
        sstr << "    col = (1.0 - 2.0 * b) * a * a + 2.0 * b * a;" << std::endl;
    }
    sstr << "    FragColor = vec4(col, 1.0);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

PostProcessingLogic::PostProcessingLogic(
    RenderLogic& child_logic,
    const FixedArray<float, 3>& background_color,
    bool depth_fog,
    bool low_pass,
    bool high_pass)
: child_logic_{child_logic},
  background_color_{background_color},
  rendering_context_{RenderingContextStack::resource_context()},
  initialized_{false},
  depth_fog_{depth_fog},
  low_pass_{low_pass},
  high_pass_{high_pass}
{}

PostProcessingLogic::~PostProcessingLogic() {};

void PostProcessingLogic::ensure_initialized() {
    if (!initialized_) {
        rp_.allocate(simple_vertex_shader_text_, fragment_shader_text({}, {}, low_pass_, high_pass_, depth_fog_, !soft_light_filename_.empty()));

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
        if (!soft_light_filename_.empty()) {
            rp_.soft_light_texture_location = checked_glGetUniformLocation(rp_.program, "softLightTexture");
        } else {
            rp_.soft_light_texture_location = 0;
        }
        initialized_ = true;
    }
}

void PostProcessingLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("PostProcessingLogic::render");
    // TimeGuard time_guard{"PostProcessingLogic::render", "PostProcessingLogic::render"};
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("PostProcessingLogic did not receive standard rendering");
    }
    if (!render_config.vfx || !child_logic_.requires_postprocessing()) {
        child_logic_.render(
            width,
            height,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    } else {
        assert_true(render_config.nsamples_msaa > 0);

        ensure_initialized();

        fbs_.configure({.width = width, .height = height, .with_depth_texture = true, .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{fbs_};
            child_logic_.render(
                width,
                height,
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
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
                CHK(glUniform3fv(rp_.background_color_location, 1, background_color_.flat_begin()));
            }
            if (!soft_light_filename_.empty()) {
                CHK(glUniform1i(rp_.soft_light_texture_location, 2));
            }
            CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
            CHK(glBindTexture(GL_TEXTURE_2D, fbs_.texture_color()));  // use the color attachment texture as the texture of the quad plane

            if (depth_fog_ || low_pass_) {
                CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                CHK(glBindTexture(GL_TEXTURE_2D, fbs_.texture_depth()));
            }
            if (!soft_light_filename_.empty()) {
                CHK(glActiveTexture(GL_TEXTURE0 + 2)); // Texture unit 2
                CHK(glBindTexture(
                    GL_TEXTURE_2D,
                    rendering_context_.rendering_resources->get_texture(
                        "soft_light",
                        TextureDescriptor{
                            .color = soft_light_filename_,
                            .color_mode = ColorMode::RGB,
                            .mipmap_mode = MipmapMode::NO_MIPMAPS})));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            }

            CHK(glBindVertexArray(va().vertex_array));
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

const FixedArray<double, 4, 4>& PostProcessingLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, double, 3>& PostProcessingLogic::iv() const {
    return child_logic_.iv();
}

bool PostProcessingLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void PostProcessingLogic::set_soft_light_filename(const std::string& soft_light_filename) {
    if (!soft_light_filename_.empty()) {
        THROW_OR_ABORT("Soft light filename already set");
    }
    soft_light_filename_ = soft_light_filename;
}

void PostProcessingLogic::set_background_color(const FixedArray<float, 3>& color) {
    background_color_ = color;
}

void PostProcessingLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "PostProcessingLogic\n";
    child_logic_.print(ostr, depth + 1);
}
