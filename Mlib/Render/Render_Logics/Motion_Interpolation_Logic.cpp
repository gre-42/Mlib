#include "Motion_Interpolation_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>

using namespace Mlib;

// static SaveMovie save_movie;

/**
 * https://stackoverflow.com/questions/6408851/draw-the-depth-value-in-opengl-using-shaders/6409229#6409229
 */
static GenShaderText fragment_shader_text{[](bool interpolate)
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
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

static GenShaderText optical_flow_diff_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
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

static GenShaderText optical_flow_diff1_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
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

static GenShaderText optical_flow_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
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
    sstr << "    for (int i = -n; i <= n; ++i) {" << std::endl;
    sstr << "        for (int j = -n; j <= n; ++j) {" << std::endl;
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

static GenShaderText optical_flow_apply_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
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
    : initialized_{ false }
    , child_logic_{ child_logic }
    , interpolation_type_{ interpolation_type }
{}

MotionInterpolationLogic::~MotionInterpolationLogic() {
    on_destroy.clear();
}

void MotionInterpolationLogic::ensure_initialized() {
    if (!initialized_) {
        // shader configuration
        // --------------------
        rp_no_interpolate_.allocate(simple_vertex_shader_text_, fragment_shader_text(/* interpolate = */false));
        // https://www.khronos.org/opengl/wiki/Example/Texture_Shader_Binding
        rp_no_interpolate_.screen_texture_color0_location = rp_no_interpolate_.get_uniform_location("screenTextureColor0");
        rp_no_interpolate_.screen_texture_color1_location = 0;
        rp_no_interpolate_.screen_texture_of_diff_location = 0;
        rp_no_interpolate_.screen_texture_of_location = 0;
        rp_no_interpolate_.width_location = 0;
        rp_no_interpolate_.height_location = 0;
        if (interpolation_type_ == InterpolationType::MEAN) {
            rp_interpolate_mean_.allocate(simple_vertex_shader_text_, fragment_shader_text(/* interpolate = */true));
            rp_interpolate_mean_.screen_texture_color0_location = rp_interpolate_mean_.get_uniform_location("screenTextureColor0");
            rp_interpolate_mean_.screen_texture_color1_location = rp_interpolate_mean_.get_uniform_location("screenTextureColor1");
            rp_interpolate_mean_.screen_texture_of_diff_location = 0;
            rp_interpolate_mean_.screen_texture_of_location = 0;
            rp_interpolate_mean_.width_location = 0;
            rp_interpolate_mean_.height_location = 0;
        }
        if (interpolation_type_ == InterpolationType::OPTICAL_FLOW) {
            rp_interpolate_of_diff_.allocate(simple_vertex_shader_text_, optical_flow_diff1_fragment_shader_text());
            rp_interpolate_of_diff_.screen_texture_color0_location = rp_interpolate_of_diff_.get_uniform_location("screenTextureColor0");
            rp_interpolate_of_diff_.screen_texture_color1_location = rp_interpolate_of_diff_.get_uniform_location("screenTextureColor1");
            rp_interpolate_of_diff_.screen_texture_of_diff_location = 0;
            rp_interpolate_of_diff_.screen_texture_of_location = 0;
            rp_interpolate_of_diff_.width_location = rp_interpolate_of_diff_.get_uniform_location("width");
            rp_interpolate_of_diff_.height_location = rp_interpolate_of_diff_.get_uniform_location("height");

            rp_interpolate_of_finalize_.allocate(simple_vertex_shader_text_, optical_flow_fragment_shader_text());
            rp_interpolate_of_finalize_.screen_texture_color0_location = 0;
            rp_interpolate_of_finalize_.screen_texture_color1_location = 0;
            rp_interpolate_of_finalize_.screen_texture_of_diff_location = rp_interpolate_of_finalize_.get_uniform_location("screenTextureDiff");
            rp_interpolate_of_finalize_.screen_texture_of_location = 0;
            rp_interpolate_of_finalize_.width_location = 0;
            rp_interpolate_of_finalize_.height_location = 0;

            rp_interpolate_of_apply_.allocate(simple_vertex_shader_text_, optical_flow_apply_fragment_shader_text());
            rp_interpolate_of_apply_.screen_texture_color0_location = rp_interpolate_of_apply_.get_uniform_location("screenTextureColor0");
            rp_interpolate_of_apply_.screen_texture_color1_location = rp_interpolate_of_apply_.get_uniform_location("screenTextureColor1");
            rp_interpolate_of_apply_.screen_texture_of_diff_location = 0;
            rp_interpolate_of_apply_.screen_texture_of_location = rp_interpolate_of_apply_.get_uniform_location("screenTextureOpticalFlow");
            rp_interpolate_of_apply_.width_location = 0;
            rp_interpolate_of_apply_.height_location = 0;
        }
        initialized_ = true;
    }
}

std::optional<RenderSetup> MotionInterpolationLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.render_setup(lx, ly, frame_id);
}

bool MotionInterpolationLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("MotionInterpolationLogic::render");
    // TimeGuard time_guard{"MotionInterpolationLogic::render", "MotionInterpolationLogic::render"};
    if (!render_config.motion_interpolation) {
        // lerr() << "n " << (int)frame_id.external_render_pass << " " << frame_id.time_id;
        child_logic_.render_auto_setup(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id,
            setup);
    } else {
        ensure_initialized();

        bool interpolate = ((frame_id.time_id % 2) == 1);
        bool render_texture = ((frame_id.time_id % 2) == 0);
        if (render_texture) {
            RenderedSceneDescriptor rsd{.external_render_pass = {ExternalRenderPassType::STANDARD, frame_id.external_render_pass.time}, .time_id = frame_id.time_id };
            auto it = frame_buffers_.find(rsd);
            if (it == frame_buffers_.end()) {
                it = frame_buffers_.try_emplace(rsd, std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION)).first;
            }
            it->second->configure({.width = lx.ilength(), .height = ly.ilength()});
            RenderToFrameBufferGuard rfg{ it->second };
            child_logic_.render_auto_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                rsd,
                setup);
        }

        if (!interpolate) {
            RenderedSceneDescriptor rsd_r{.external_render_pass = {ExternalRenderPassType::STANDARD, frame_id.external_render_pass.time}, .time_id = (frame_id.time_id + 2) % 4 };
            auto it = frame_buffers_.find(rsd_r);
            if (it != frame_buffers_.end()) {
                rp_no_interpolate_.use();

                CHK(glUniform1i(rp_no_interpolate_.screen_texture_color0_location, 0));
                CHK(glBindTexture(GL_TEXTURE_2D, it->second->texture_color()->handle<GLuint>()));

                va().bind();
                CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                CHK(glBindVertexArray(0));
                // save_movie.save("/tmp/mov-", "-n", width, height);
            }
        } else {
            RenderedSceneDescriptor rsd_r0{.external_render_pass = {ExternalRenderPassType::STANDARD, frame_id.external_render_pass.time}, .time_id = (frame_id.time_id + 1) % 4 };
            RenderedSceneDescriptor rsd_r1{.external_render_pass = {ExternalRenderPassType::STANDARD, frame_id.external_render_pass.time}, .time_id = (frame_id.time_id + 3) % 4 };
            auto it0 = frame_buffers_.find(rsd_r0);
            auto it1 = frame_buffers_.find(rsd_r1);
            if ((it0 != frame_buffers_.end()) && (it1 != frame_buffers_.end())) {
                if (interpolation_type_ == InterpolationType::MEAN) {
                    rp_interpolate_mean_.use();

                    CHK(glUniform1i(rp_interpolate_mean_.screen_texture_color0_location, 0));
                    CHK(glUniform1i(rp_interpolate_mean_.screen_texture_color1_location, 1));

                    CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                    CHK(glBindTexture(GL_TEXTURE_2D, it0->second->texture_color()->handle<GLuint>()));

                    CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                    CHK(glBindTexture(GL_TEXTURE_2D, it1->second->texture_color()->handle<GLuint>()));

                    va().bind();
                    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                    CHK(glBindVertexArray(0));

                    // Reset to defaults
                    CHK(glActiveTexture(GL_TEXTURE0));
                } else if (interpolation_type_ == InterpolationType::OPTICAL_FLOW) {
                    GLint of_width = 640;
                    GLint of_height = 480;
                    auto fb_flow = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
                    {
                        ViewportGuard vg{ of_width, of_height };
                        auto fb_diff = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
                        // https://community.khronos.org/t/texture-can-not-keep-negative-value/66018/3
                        fb_diff->configure(FrameBufferConfig{.width = of_width, .height = of_height, .color_internal_format = GL_RGBA32F, .color_type = GL_FLOAT});
                        {
                            RenderToFrameBufferGuard rfg{ fb_diff };
                            rp_interpolate_of_diff_.use();

                            CHK(glUniform1i(rp_interpolate_of_diff_.screen_texture_color0_location, 0));
                            CHK(glUniform1i(rp_interpolate_of_diff_.screen_texture_color1_location, 1));
                            CHK(glUniform1f(rp_interpolate_of_diff_.width_location, (GLfloat)of_width));
                            CHK(glUniform1f(rp_interpolate_of_diff_.height_location, (GLfloat)of_height));

                            CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                            CHK(glBindTexture(GL_TEXTURE_2D, it0->second->texture_color()->handle<GLuint>()));

                            CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                            CHK(glBindTexture(GL_TEXTURE_2D, it1->second->texture_color()->handle<GLuint>()));

                            va().bind();
                            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                            CHK(glBindVertexArray(0));

                            // Reset to defaults
                            CHK(glActiveTexture(GL_TEXTURE0));
                        }
                        // https://community.khronos.org/t/texture-can-not-keep-negative-value/66018/3
                        fb_flow->configure(FrameBufferConfig{.width = of_width, .height = of_height, .color_internal_format = GL_RGBA32F, .color_type = GL_FLOAT});
                        {
                            RenderToFrameBufferGuard rfg{ fb_flow };

                            rp_interpolate_of_finalize_.use();

                            CHK(glUniform1i(rp_interpolate_of_finalize_.screen_texture_of_diff_location, 0));

                            CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                            CHK(glBindTexture(GL_TEXTURE_2D, fb_diff->texture_color()->handle<GLuint>()));

                            va().bind();
                            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                            CHK(glBindVertexArray(0));

                            // save_movie.save("/tmp/mov-", "-f", of_width, of_height);

                            // Reset to defaults
                            CHK(glActiveTexture(GL_TEXTURE0));
                        }
                        fb_diff->deallocate();
                    }
                    {
                        rp_interpolate_of_apply_.use();

                        CHK(glUniform1i(rp_interpolate_of_apply_.screen_texture_color0_location, 0));
                        CHK(glUniform1i(rp_interpolate_of_apply_.screen_texture_color1_location, 1));
                        CHK(glUniform1i(rp_interpolate_of_apply_.screen_texture_of_location, 2));

                        CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
                        CHK(glBindTexture(GL_TEXTURE_2D, it0->second->texture_color()->handle<GLuint>()));

                        CHK(glActiveTexture(GL_TEXTURE0 + 1)); // Texture unit 1
                        CHK(glBindTexture(GL_TEXTURE_2D, it1->second->texture_color()->handle<GLuint>()));

                        CHK(glActiveTexture(GL_TEXTURE0 + 2)); // Texture unit 2
                        CHK(glBindTexture(GL_TEXTURE_2D, fb_flow->texture_color()->handle<GLuint>()));

                        va().bind();
                        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                        CHK(glBindVertexArray(0));

                        // save_movie.save("/tmp/mov-", "-i", of_width, of_height);

                        // Reset to defaults
                        CHK(glActiveTexture(GL_TEXTURE0));
                    }
                }
            }
        }
    }
    return true;
}

void MotionInterpolationLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "MotionInterpolationLogic\n";
    child_logic_.print(ostr, depth + 1);
}
