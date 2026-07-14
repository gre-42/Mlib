#include "Fxaa_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Misc/Log.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Gen_Shader_Text.hpp>
#include <Mlib/OpenGL/Instance_Handles/Render_Guards.hpp>
#include <Mlib/OpenGL/Render_Config.hpp>
#include <Mlib/OpenGL/Render_Setup.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Shader_Version_3_0.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <sstream>

using namespace Mlib;

// From: https://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/3/
static GenShaderText vertex_shader_text = [](){
    std::stringstream sstr;
    sstr << vertex_shader_preamble();
    sstr << "layout (location = 0) in vec2 aPos;\n";
    sstr << "layout (location = 1) in vec2 aTexCoords;\n";
    sstr << "\n";
    sstr << "out vec2 TexCoords;\n";
    sstr << "out vec4 posPos;\n";
    sstr << "uniform float FXAA_SUBPIX_SHIFT = 1.0/4.0;\n";
    sstr << "uniform float rt_w; // GeeXLab built-in\n";
    sstr << "uniform float rt_h; // GeeXLab built-in\n";
    sstr << "\n";
    sstr << "void main(void)\n";
    sstr << "{\n";
    sstr << "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n";
    sstr << "    TexCoords = aTexCoords;\n";
    sstr << "    vec2 rcpFrame = vec2(1.0/rt_w, 1.0/rt_h);\n";
    sstr << "    posPos.xy = aTexCoords.xy;\n";
    sstr << "    posPos.zw = aTexCoords.xy - \n";
    sstr << "                  (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));\n";
    sstr << "}\n";
    return sstr.str();
};

static GenShaderText fragment_shader_text = [](){
    std::stringstream sstr;
    sstr << fragment_shader_preamble();
    sstr << "uniform sampler2D tex0; // 0\n";
    sstr << "uniform float rt_w; // GeeXLab built-in\n";
    sstr << "uniform float rt_h; // GeeXLab built-in\n";
    sstr << "uniform float FXAA_SPAN_MAX = 8.0;\n";
    sstr << "uniform float FXAA_REDUCE_MUL = 1.0/8.0;\n";
    sstr << "in vec4 posPos;\n";
    sstr << "in vec2 TexCoords;\n";
    sstr << "\n";
    sstr << "#define FxaaInt2 ivec2\n";
    sstr << "#define FxaaFloat2 vec2\n";
    sstr << "#define FxaaTexLod0(t, p) texture2D(t, p, 0.0)\n";
    sstr << "#define FxaaTexOff(t, p, o) texture2D(t, p + o, 0.0)\n";
    sstr << "\n";
    sstr << "vec3 FxaaPixelShader( \n";
    sstr << "  vec4 posPos, // Output of FxaaVertexShader interpolated across screen.\n";
    sstr << "  sampler2D tex, // Input texture.\n";
    sstr << "  vec2 rcpFrame) // Constant {1.0/frameWidth, 1.0/frameHeight}.\n";
    sstr << "{   \n";
    sstr << "/*---------------------------------------------------------*/\n";
    sstr << "    #define FXAA_REDUCE_MIN   (1.0/128.0)\n";
    sstr << "    //#define FXAA_REDUCE_MUL   (1.0/8.0)\n";
    sstr << "    //#define FXAA_SPAN_MAX     8.0\n";
    sstr << "/*---------------------------------------------------------*/\n";
    sstr << "    vec3 rgbNW = FxaaTexLod0(tex, posPos.zw).xyz;\n";
    sstr << "    vec3 rgbNE = FxaaTexOff(tex, posPos.zw, vec2(rcpFrame.x, 0)).xyz;\n";
    sstr << "    vec3 rgbSW = FxaaTexOff(tex, posPos.zw, vec2(0, rcpFrame.y)).xyz;\n";
    sstr << "    vec3 rgbSE = FxaaTexOff(tex, posPos.zw, rcpFrame.xy).xyz;\n";
    sstr << "    vec3 rgbM  = FxaaTexLod0(tex, posPos.xy).xyz;\n";
    sstr << "/*---------------------------------------------------------*/\n";
    sstr << "    vec3 luma = vec3(0.299, 0.587, 0.114);\n";
    sstr << "    float lumaNW = dot(rgbNW, luma);\n";
    sstr << "    float lumaNE = dot(rgbNE, luma);\n";
    sstr << "    float lumaSW = dot(rgbSW, luma);\n";
    sstr << "    float lumaSE = dot(rgbSE, luma);\n";
    sstr << "    float lumaM  = dot(rgbM,  luma);\n";
    sstr << "/*---------------------------------------------------------*/\n";
    sstr << "    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));\n";
    sstr << "    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));\n";
    sstr << "/*---------------------------------------------------------*/\n";
    sstr << "    vec2 dir; \n";
    sstr << "    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));\n";
    sstr << "    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));\n";
    sstr << "/*---------------------------------------------------------*/\n";
    sstr << "    float dirReduce = max(\n";
    sstr << "        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),\n";
    sstr << "        FXAA_REDUCE_MIN);\n";
    sstr << "    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);\n";
    sstr << "    dir = min(FxaaFloat2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX), \n";
    sstr << "          max(FxaaFloat2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), \n";
    sstr << "          dir * rcpDirMin)) * rcpFrame.xy;\n";
    sstr << "/*--------------------------------------------------------*/\n";
    sstr << "    vec3 rgbA = (1.0/2.0) * (\n";
    sstr << "        FxaaTexLod0(tex, posPos.xy + dir * (1.0/3.0 - 0.5)).xyz +\n";
    sstr << "        FxaaTexLod0(tex, posPos.xy + dir * (2.0/3.0 - 0.5)).xyz);\n";
    sstr << "    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (\n";
    sstr << "        FxaaTexLod0(tex, posPos.xy + dir * (0.0/3.0 - 0.5)).xyz +\n";
    sstr << "        FxaaTexLod0(tex, posPos.xy + dir * (3.0/3.0 - 0.5)).xyz);\n";
    sstr << "    float lumaB = dot(rgbB, luma);\n";
    sstr << "    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;\n";
    sstr << "    return rgbB; }\n";
    sstr << "\n";
    sstr << "vec4 PostFX(sampler2D tex, vec2 uv, float time)\n";
    sstr << "{\n";
    sstr << "  vec4 c = vec4(0.0);\n";
    sstr << "  vec2 rcpFrame = vec2(1.0/rt_w, 1.0/rt_h);\n";
    sstr << "  c.rgb = FxaaPixelShader(posPos, tex, rcpFrame);\n";
    sstr << "  //c.rgb = 1.0 - texture2D(tex, posPos.xy).rgb;\n";
    sstr << "  c.a = 1.0;\n";
    sstr << "  return c;\n";
    sstr << "}\n";
    sstr << "\n";
    sstr << "void main() \n";
    sstr << "{ \n";
    sstr << "  vec2 uv = TexCoords.st;\n";
    sstr << "  gl_FragColor = PostFX(tex0, uv, 0.0);\n";
    sstr << "}\n";
    return sstr.str();
};

FxaaLogic::FxaaLogic(RenderLogic& child_logic)
    : child_logic_{child_logic}
    , initialized_{false}
    , fbs_{ std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
{}

FxaaLogic::~FxaaLogic() {
    on_destroy.clear();
}

void FxaaLogic::ensure_initialized() {
    if (!initialized_) {
        rp_.allocate(vertex_shader_text(), fragment_shader_text());

        // https://www.khronos.org/opengl/wiki/Example/Texture_Shader_Binding
        rp_.screen_texture_color_location = rp_.get_uniform_location("tex0");
        rp_.rt_w_location = rp_.get_uniform_location("rt_w");
        rp_.rt_h_location = rp_.get_uniform_location("rt_h");
        initialized_ = true;
    }
}

std::optional<RenderSetup> FxaaLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.render_setup(lx, ly, frame_id);
}

void FxaaLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    LOG_FUNCTION("FxaaLogic::render");
    // TimeGuard time_guard{"FxaaLogic::render", "FxaaLogic::render"};
    if (!render_config.fxaa || !setup.camera->get_requires_postprocessing()) {
        child_logic_.render_with_setup(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id,
            setup);
    } else {
        assert_true(render_config.nsamples_msaa > 0);

        ensure_initialized();

        int width = lx.ilength();
        int height = ly.ilength();
        fbs_->configure({.width = width, .height = height});
        {
            RenderToFrameBufferGuard rfg{fbs_};
            child_logic_.render_with_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                frame_id,
                setup);
        }

        // Now draw a quad plane with the attached framebuffer color texture
        // Depth test should already be disabled
        // CHK(glDisable(GL_DEPTH_TEST)); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        // CHK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f)); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
        // CHK(glClear(GL_COLOR_BUFFER_BIT));

        {
            notify_rendering(CURRENT_SOURCE_LOCATION);
            rp_.use();

            CHK(glUniform1i(rp_.screen_texture_color_location, 0));
            CHK(glUniform1f(rp_.rt_w_location, (float)width));
            CHK(glUniform1f(rp_.rt_h_location, (float)height));
            CHK(glActiveTexture(GL_TEXTURE0 + 0)); // Texture unit 0
            CHK(glBindTexture(GL_TEXTURE_2D, fbs_->texture_color()->handle<GLuint>()));  // use the color attachment texture as the texture of the quad plane

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            // Reset to defaults
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
}

void FxaaLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FxaaLogic\n";
    child_logic_.print(ostr, depth + 1);
}
