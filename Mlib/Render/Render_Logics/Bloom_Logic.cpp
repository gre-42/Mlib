#include "Bloom_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_Channel_Kind.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static GenShaderText threshold_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screen_texture_color;" << std::endl;
    sstr << "uniform vec3 brightness_threshold;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    vec3 color = texture(screen_texture_color, TexCoords.st).rgb;" << std::endl;
    sstr << "    FragColor = vec4(max(vec3(0, 0, 0), color.rgb - brightness_threshold), 1.0);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

static GenShaderText blend_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D screen_texture_color;" << std::endl;
    sstr << "uniform sampler2D bloom_texture_color;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    vec3 screen_color = texture(screen_texture_color, TexCoords.st).rgb;" << std::endl;
    sstr << "    vec3 bloom_color = texture(bloom_texture_color, TexCoords.st).rgb;" << std::endl;
    sstr << "    FragColor = vec4(screen_color + bloom_color, 1.0);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

BloomLogic::BloomLogic(
    RenderLogic& child_logic,
    const FixedArray<float, 3>& brightness_threshold,
    const FixedArray<uint32_t, 2>& niterations)
    : child_logic_{ child_logic }
    , brightness_threshold_{ brightness_threshold }
    , niterations_{ niterations }
    , screen_fbs_{ std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
    , bloom_fbs_{
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION),
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
{}

BloomLogic::~BloomLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> BloomLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.try_render_setup(lx, ly, frame_id);
}

bool BloomLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("BloomLogic::render");
    // TimeGuard time_guard{"BloomLogic::render", "BloomLogic::render"};
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("BloomLogic did not receive standard rendering");
    }
    if (all(brightness_threshold_ == 1.f)) {
        child_logic_.render_auto_setup(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id,
            setup);
    } else {
        assert_true(render_config.nsamples_msaa > 0);

        if (!rp_threshold_.allocated()) {
            rp_threshold_.allocate(simple_vertex_shader_text_, threshold_fragment_shader_text());
            rp_threshold_.screen_texture_color_location = rp_threshold_.get_uniform_location("screen_texture_color");
            rp_threshold_.brightness_threshold_location = rp_threshold_.get_uniform_location("brightness_threshold");
        }
        if (!rp_blend_.allocated()) {
            rp_blend_.allocate(simple_vertex_shader_text_, blend_fragment_shader_text());
            rp_blend_.screen_texture_color_location = rp_blend_.get_uniform_location("screen_texture_color");
            rp_blend_.bloom_texture_color_location = rp_blend_.get_uniform_location("bloom_texture_color");
        }

        auto width = lx.ilength();
        auto height = ly.ilength();
        screen_fbs_->configure({
            .width = width,
            .height = height,
            .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{ screen_fbs_ };
            ViewportGuard vg{ width, height };

            child_logic_.render_auto_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                frame_id,
                setup);
        }

        for (auto& fbs : bloom_fbs_) {
            fbs->configure({
                .width = width,
                .height = height,
                .depth_kind = FrameBufferChannelKind::NONE,
                .wrap_s = GL_CLAMP_TO_EDGE,
                .wrap_t = GL_CLAMP_TO_EDGE});
        }
        size_t bloom_target_id = 0;
        {
            RenderToFrameBufferGuard rfg{ bloom_fbs_[bloom_target_id] };
            ViewportGuard vg{ width, height };
            notify_rendering(CURRENT_SOURCE_LOCATION);

            rp_threshold_.use();

            CHK(glUniform1i(rp_threshold_.screen_texture_color_location, 0));
            CHK(glUniform3fv(rp_threshold_.brightness_threshold_location, 1, brightness_threshold_.flat_begin()));

            CHK(glActiveTexture(GL_TEXTURE0 + 0));
            CHK(glBindTexture(GL_TEXTURE_2D, screen_fbs_->texture_color()->handle<GLuint>()));

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            CHK(glActiveTexture(GL_TEXTURE0));
        }
        lowpass_.render(width, height, niterations_, bloom_fbs_, bloom_target_id);
        {
            notify_rendering(CURRENT_SOURCE_LOCATION);
            rp_blend_.use();

            CHK(glUniform1i(rp_blend_.screen_texture_color_location, 0));
            CHK(glUniform1i(rp_blend_.bloom_texture_color_location, 1));

            CHK(glActiveTexture(GL_TEXTURE0 + 0));
            CHK(glBindTexture(GL_TEXTURE_2D, screen_fbs_->texture_color()->handle<GLuint>()));

            CHK(glActiveTexture(GL_TEXTURE0 + 1));
            CHK(glBindTexture(GL_TEXTURE_2D, bloom_fbs_[bloom_target_id]->texture_color()->handle<GLuint>()));

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    return true;
}

void BloomLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "BloomLogic\n";
    child_logic_.print(ostr, depth + 1);
}
