#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Motion_Interpolation_Logic.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Log.hpp>
// #include <Mlib/Render/Save_Movie.hpp>

using namespace Mlib;

// static SaveMovie save_movie;

/**
 * From: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
 *
 * https://stackoverflow.com/questions/6408851/draw-the-depth-value-in-opengl-using-shaders/6409229#6409229
 */
static GenShaderText fragment_shader_text{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices,
    bool interpolate)
{
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screenTextureColor0;" << std::endl;
    if (interpolate) {
        sstr << "uniform sampler2D screenTextureColor1;" << std::endl;
    }
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    vec3 col = vec3(texture(screenTextureColor0, TexCoords.st));" << std::endl;
    if (interpolate) {
        sstr << "    col = 0.5 * col + 0.5 * vec3(texture(screenTextureColor1, TexCoords.st));" << std::endl;
    }
    sstr << "    FragColor = vec4(col, 1.0);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

static GenShaderText optical_flow_diff_fragment_shader_text{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices)
{
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screenTextureColor0;" << std::endl;
    sstr << "uniform sampler2D screenTextureColor1;" << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    float dx = 0.001;" << std::endl;
    sstr << "    float dy = 0.001;" << std::endl;

    sstr << "    float ItP = dot(texture(screenTextureColor1, TexCoords.st).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float ItM = dot(texture(screenTextureColor0, TexCoords.st).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;

    sstr << "    float Itf = ItP - ItM;" << std::endl;

    sstr << "    float IxM0 = dot(texture(screenTextureColor0, TexCoords.st - vec2(dx, 0)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float IxP0 = dot(texture(screenTextureColor0, TexCoords.st + vec2(dx, 0)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float IyM0 = dot(texture(screenTextureColor0, TexCoords.st - vec2(0, dy)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float IyP0 = dot(texture(screenTextureColor0, TexCoords.st + vec2(0, dy)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;

    sstr << "    float IxM1 = dot(texture(screenTextureColor1, TexCoords.st - vec2(dx, 0)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float IxP1 = dot(texture(screenTextureColor1, TexCoords.st + vec2(dx, 0)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float IyM1 = dot(texture(screenTextureColor1, TexCoords.st - vec2(0, dy)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float IyP1 = dot(texture(screenTextureColor1, TexCoords.st + vec2(0, dy)).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;

    sstr << "    float Ix = 0.5 * ((IxP0 - IxM0) + (IxP1 - IxM1)) / (2 * dx);" << std::endl;
    sstr << "    float Iy = 0.5 * ((IyP0 - IyM0) + (IyP1 - IyM1)) / (2 * dy);" << std::endl;

    // sstr << "    float Ix = (IxP0 - IxM0) / (2 * dx);" << std::endl;
    // sstr << "    float Iy = (IyP0 - IyM0) / (2 * dy);" << std::endl;

    // sstr << "    float Ix = (IxP1 - IxM1) / (2 * dx);" << std::endl;
    // sstr << "    float Iy = (IyP1 - IyM1) / (2 * dy);" << std::endl;

    sstr << "    FragColor = vec4(pow(Ix, 2), pow(Iy, 2), Ix * Itf, Iy * Itf);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

static GenShaderText optical_flow_diff1_fragment_shader_text{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices)
{
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform float width;" << std::endl;
    sstr << "uniform float height;" << std::endl;
    sstr << "uniform sampler2D screenTextureColor0;" << std::endl;
    sstr << "uniform sampler2D screenTextureColor1;" << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    float c0 = dot(texture(screenTextureColor0, TexCoords.st).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;
    sstr << "    float c1 = dot(texture(screenTextureColor1, TexCoords.st).rgb, vec3(0.299, 0.587, 0.114));" << std::endl;

    sstr << "    float Itf = c1 - c0;" << std::endl;

    sstr << "    float Ix0 = dFdx(c0) / (width - 1);" << std::endl;
    sstr << "    float Ix1 = dFdx(c1) / (width - 1);" << std::endl;

    sstr << "    float Iy0 = dFdy(c0) / (height - 1);" << std::endl;
    sstr << "    float Iy1 = dFdy(c1) / (height - 1);" << std::endl;

    sstr << "    float Ix = 0.5 * (Ix0 + Ix1);" << std::endl;
    sstr << "    float Iy = 0.5 * (Iy0 + Iy1);" << std::endl;

    // sstr << "    float Ix = Ix0;" << std::endl;
    // sstr << "    float Iy = Iy0;" << std::endl;

    sstr << "    FragColor = vec4(pow(Ix, 2), pow(Iy, 2), Ix * Itf, Iy * Itf);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

static GenShaderText optical_flow_fragment_shader_text{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices)
{
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screenTextureDiff;" << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    float dx = 0.001;" << std::endl;
    sstr << "    float dy = 0.001;" << std::endl;
    sstr << "    vec4 s = vec4(0, 0, 0, 0);" << std::endl;
    sstr << "    float s_xy = 0;" << std::endl;
    sstr << "    int n = 5;" << std::endl;
    sstr << "    for(int i = -n; i <= n; ++i) {" << std::endl;
    sstr << "        for(int j = -n; j <= n; ++j) {" << std::endl;
    sstr << "            s += texture(screenTextureDiff, TexCoords.st + vec2(i * dx, j * dy));" << std::endl;
    sstr << "            s_xy += 0;" << std::endl;
    sstr << "        }" << std::endl;
    sstr << "    }" << std::endl;
    sstr << "    s /= (n + 1) * (n + 1);" << std::endl;
    sstr << "    s_xy /= (n + 1) * (n + 1);" << std::endl;
    sstr << "    float rdetM = 1.f / (s.x * s.y - s_xy * s_xy);" << std::endl;
    sstr << "    if (abs(rdetM) > 1e12) { rdetM = 0; }" << std::endl;
    sstr << "    vec2 flow = -rdetM * vec2(" << std::endl;
    sstr << "        s.y * s.z - s_xy * s.w," << std::endl;
    sstr << "        s.x * s.w - s_xy * s.z);" << std::endl;
    sstr << "    FragColor = vec4(flow, 0, 1);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

static GenShaderText optical_flow_apply_fragment_shader_text{[](
    const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights,
    const std::vector<size_t>& light_noshadow_indices,
    const std::vector<size_t>& light_shadow_indices,
    const std::vector<size_t>& black_shadow_indices)
{
    std::stringstream sstr;
    sstr << "#version 330 core" << std::endl;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screenTextureColor0;" << std::endl;
    sstr << "uniform sampler2D screenTextureColor1;" << std::endl;
    sstr << "uniform sampler2D screenTextureOpticalFlow;" << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    vec2 flow = texture(screenTextureOpticalFlow, TexCoords.st).rg;" << std::endl;
    // Sampling takes place in the target-domain, so the signs are inverted.
    sstr << "    vec3 color0 = texture(screenTextureColor0, TexCoords.st - 0.5 * flow).rgb;" << std::endl;
    sstr << "    vec3 color1 = texture(screenTextureColor1, TexCoords.st + 0.5 * flow).rgb;" << std::endl;
    sstr << "    FragColor = vec4(0.5 * color0 + 0.5 * color1, 1.0);" << std::endl;
    //sstr << "    FragColor = vec4(flow.x, flow.y, 0.0, 1.0);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

MotionInterpolationLogic::MotionInterpolationLogic(RenderLogic& child_logic, InterpolationType interpolation_type)
: child_logic_{child_logic},
  interpolation_type_{interpolation_type}
{
    // shader configuration
    // --------------------
    rp_no_interpolate_.generate(vertex_shader_text, fragment_shader_text({}, {}, {}, {}, false));
    // https://www.khronos.org/opengl/wiki/Example/Texture_Shader_Binding
    rp_no_interpolate_.screen_texture_color0_location = checked_glGetUniformLocation(rp_no_interpolate_.program, "screenTextureColor0");
    rp_no_interpolate_.screen_texture_color1_location = 0;
    rp_no_interpolate_.screen_texture_of_diff_location = 0;
    rp_no_interpolate_.screen_texture_of_location = 0;
    rp_no_interpolate_.width_location = 0;
    rp_no_interpolate_.height_location = 0;
    if (interpolation_type_ == InterpolationType::MEAN) {
        rp_interpolate_mean_.generate(vertex_shader_text, fragment_shader_text({}, {}, {}, {}, true));
        rp_interpolate_mean_.screen_texture_color0_location = checked_glGetUniformLocation(rp_interpolate_mean_.program, "screenTextureColor0");
        rp_interpolate_mean_.screen_texture_color1_location = checked_glGetUniformLocation(rp_interpolate_mean_.program, "screenTextureColor1");
        rp_interpolate_mean_.screen_texture_of_diff_location = 0;
        rp_interpolate_mean_.screen_texture_of_location = 0;
        rp_interpolate_mean_.width_location = 0;
        rp_interpolate_mean_.height_location = 0;
    }
    if (interpolation_type_ == InterpolationType::OPTICAL_FLOW) {
        rp_interpolate_of_diff_.generate(vertex_shader_text, optical_flow_diff1_fragment_shader_text({}, {}, {}, {}));
        rp_interpolate_of_diff_.screen_texture_color0_location = checked_glGetUniformLocation(rp_interpolate_of_diff_.program, "screenTextureColor0");
        rp_interpolate_of_diff_.screen_texture_color1_location = checked_glGetUniformLocation(rp_interpolate_of_diff_.program, "screenTextureColor1");
        rp_interpolate_of_diff_.screen_texture_of_diff_location = 0;
        rp_interpolate_of_diff_.screen_texture_of_location = 0;
        rp_interpolate_of_diff_.width_location = checked_glGetUniformLocation(rp_interpolate_of_diff_.program, "width");
        rp_interpolate_of_diff_.height_location = checked_glGetUniformLocation(rp_interpolate_of_diff_.program, "height");

        rp_interpolate_of_finalize_.generate(vertex_shader_text, optical_flow_fragment_shader_text({}, {}, {}, {}));
        rp_interpolate_of_finalize_.screen_texture_color0_location = 0;
        rp_interpolate_of_finalize_.screen_texture_color1_location = 0;
        rp_interpolate_of_finalize_.screen_texture_of_diff_location = checked_glGetUniformLocation(rp_interpolate_of_finalize_.program, "screenTextureDiff");
        rp_interpolate_of_finalize_.screen_texture_of_location = 0;
        rp_interpolate_of_finalize_.width_location = 0;
        rp_interpolate_of_finalize_.height_location = 0;

        rp_interpolate_of_apply_.generate(vertex_shader_text, optical_flow_apply_fragment_shader_text({}, {}, {}, {}));
        rp_interpolate_of_apply_.screen_texture_color0_location = checked_glGetUniformLocation(rp_interpolate_of_apply_.program, "screenTextureColor0");
        rp_interpolate_of_apply_.screen_texture_color1_location = checked_glGetUniformLocation(rp_interpolate_of_apply_.program, "screenTextureColor1");
        rp_interpolate_of_apply_.screen_texture_of_diff_location = 0;
        rp_interpolate_of_apply_.screen_texture_of_location = checked_glGetUniformLocation(rp_interpolate_of_apply_.program, "screenTextureOpticalFlow");
        rp_interpolate_of_apply_.width_location = 0;
        rp_interpolate_of_apply_.height_location = 0;
    }
}

MotionInterpolationLogic::~MotionInterpolationLogic() = default;

void MotionInterpolationLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("MotionInterpolationLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPass::UNDEFINED) {
        throw std::runtime_error("MotionInterpolationLogic did not receive undefined rendering");
    }
    if (!child_logic_.requires_postprocessing()) {
        // std::cerr << "n " << (int)frame_id.external_render_pass << " " << frame_id.time_id << std::endl;
        child_logic_.render(
            width,
            height,
            render_config,
            scene_graph_config,
            render_results,
            RenderedSceneDescriptor{.external_render_pass = {ExternalRenderPass::STANDARD_WO_POSTPROCESSING, ""}, .time_id = frame_id.time_id, .light_resource_id =""});
    } else {
        bool interpolate = ((frame_id.time_id % 2) == 1);
        bool render_texture = ((frame_id.time_id % 2) == 0);
        if (render_texture) {
            RenderedSceneDescriptor rsd{.external_render_pass = {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = frame_id.time_id, .light_resource_id = ""};
            frame_buffers_[rsd].configure({width: width, height: height});
            CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffers_[rsd].frame_buffer));
            child_logic_.render(
                width,
                height,
                render_config,
                scene_graph_config,
                render_results,
                rsd);
            CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        }

        if (!interpolate) {
            RenderedSceneDescriptor rsd_r{.external_render_pass = {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = (frame_id.time_id + 2) % 4, .light_resource_id = ""};
            auto it = frame_buffers_.find(rsd_r);
            if (it != frame_buffers_.end()) {
                CHK(glUseProgram(rp_no_interpolate_.program));

                CHK(glUniform1i(rp_no_interpolate_.screen_texture_color0_location, 0));
                CHK(glBindTexture(GL_TEXTURE_2D, it->second.texture_color_buffer));

                CHK(glBindVertexArray(va_.vertex_buffer));
                CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                // save_movie.save("/tmp/mov-", "-n", width, height);
            }
        } else {
            RenderedSceneDescriptor rsd_r0{.external_render_pass = {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = (frame_id.time_id + 1) % 4, .light_resource_id = ""};
            RenderedSceneDescriptor rsd_r1{.external_render_pass = {ExternalRenderPass::STANDARD_WITH_POSTPROCESSING, ""}, .time_id = (frame_id.time_id + 3) % 4, .light_resource_id = ""};
            auto it0 = frame_buffers_.find(rsd_r0);
            auto it1 = frame_buffers_.find(rsd_r1);
            if ((it0 != frame_buffers_.end()) && (it1 != frame_buffers_.end())) {
                if (interpolation_type_ == InterpolationType::MEAN) {
                    CHK(glUseProgram(rp_interpolate_mean_.program));

                    CHK(glUniform1i(rp_interpolate_mean_.screen_texture_color0_location, 0));
                    CHK(glUniform1i(rp_interpolate_mean_.screen_texture_color1_location, 1));

                    CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                    CHK(glBindTexture(GL_TEXTURE_2D, it0->second.texture_color_buffer));

                    CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                    CHK(glBindTexture(GL_TEXTURE_2D, it1->second.texture_color_buffer));

                    CHK(glBindVertexArray(va_.vertex_buffer));
                    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

                    // Reset to defaults
                    CHK(glActiveTexture(GL_TEXTURE0));
                } else if (interpolation_type_ == InterpolationType::OPTICAL_FLOW) {
                    GLint of_width = 640;
                    GLint of_height = 480;
                    glViewport(0, 0, of_width, of_height);
                    FrameBuffer fb_diff;
                    // https://community.khronos.org/t/texture-can-not-keep-negative-value/66018/3
                    fb_diff.configure(FrameBufferConfig{width: of_width, height: of_height, color_internal_format: GL_RGBA32F, color_type: GL_FLOAT});
                    {
                        CHK(glBindFramebuffer(GL_FRAMEBUFFER, fb_diff.frame_buffer));

                        CHK(glUseProgram(rp_interpolate_of_diff_.program));

                        CHK(glUniform1i(rp_interpolate_of_diff_.screen_texture_color0_location, 0));
                        CHK(glUniform1i(rp_interpolate_of_diff_.screen_texture_color1_location, 1));
                        CHK(glUniform1f(rp_interpolate_of_diff_.width_location, of_width));
                        CHK(glUniform1f(rp_interpolate_of_diff_.height_location, of_height));

                        CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                        CHK(glBindTexture(GL_TEXTURE_2D, it0->second.texture_color_buffer));

                        CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                        CHK(glBindTexture(GL_TEXTURE_2D, it1->second.texture_color_buffer));

                        CHK(glBindVertexArray(va_.vertex_buffer));
                        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

                        // Reset to defaults
                        CHK(glActiveTexture(GL_TEXTURE0));

                        CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                    }
                    FrameBuffer fb_flow;
                    // https://community.khronos.org/t/texture-can-not-keep-negative-value/66018/3
                    fb_flow.configure(FrameBufferConfig{width: of_width, height: of_height, color_internal_format: GL_RGBA32F, color_type: GL_FLOAT});
                    {
                        CHK(glBindFramebuffer(GL_FRAMEBUFFER, fb_flow.frame_buffer));

                        CHK(glUseProgram(rp_interpolate_of_finalize_.program));

                        CHK(glUniform1i(rp_interpolate_of_finalize_.screen_texture_of_diff_location, 0));

                        CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                        CHK(glBindTexture(GL_TEXTURE_2D, fb_diff.texture_color_buffer));

                        CHK(glBindVertexArray(va_.vertex_buffer));
                        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

                        // save_movie.save("/tmp/mov-", "-f", of_width, of_height);

                        // Reset to defaults
                        CHK(glActiveTexture(GL_TEXTURE0));

                        CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                    }
                    fb_diff.deallocate();
                    glViewport(0, 0, width, height);
                    {
                        CHK(glUseProgram(rp_interpolate_of_apply_.program));

                        CHK(glUniform1i(rp_interpolate_of_apply_.screen_texture_color0_location, 0));
                        CHK(glUniform1i(rp_interpolate_of_apply_.screen_texture_color1_location, 1));
                        CHK(glUniform1i(rp_interpolate_of_apply_.screen_texture_of_location, 2));

                        CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                        CHK(glBindTexture(GL_TEXTURE_2D, it0->second.texture_color_buffer));

                        CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                        CHK(glBindTexture(GL_TEXTURE_2D, it1->second.texture_color_buffer));

                        CHK(glActiveTexture(GL_TEXTURE0 + 2)); // Texture unit 2
                        CHK(glBindTexture(GL_TEXTURE_2D, fb_flow.texture_color_buffer));

                        CHK(glBindVertexArray(va_.vertex_buffer));
                        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));

                        // save_movie.save("/tmp/mov-", "-i", of_width, of_height);

                        // Reset to defaults
                        CHK(glActiveTexture(GL_TEXTURE0));
                    }
                }
            }
        }
    }
}

float MotionInterpolationLogic::near_plane() const {
    return child_logic_.near_plane();
}

float MotionInterpolationLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<float, 4, 4>& MotionInterpolationLogic::vp() const {
    return child_logic_.vp();
}

const FixedArray<float, 4, 4>& MotionInterpolationLogic::iv() const {
    return child_logic_.iv();
}

bool MotionInterpolationLogic::requires_postprocessing() const {
    return false;
}
