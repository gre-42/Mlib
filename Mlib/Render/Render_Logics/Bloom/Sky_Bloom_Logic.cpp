#include "Sky_Bloom_Logic.hpp"
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
#include <Mlib/Render/Instance_Handles/Texture_Binder.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Layer_Properties.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static GenShaderText modulate_fragment_shader_text{[]()
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D foreground;" << std::endl;
    sstr << "uniform sampler2D background;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    float fg_a = texture(foreground, TexCoords.st).a;" << std::endl;
    sstr << "    vec3 bg_rgb = texture(background, TexCoords.st).rgb;" << std::endl;
    sstr << "    FragColor = vec4(bg_rgb * (1.0 - fg_a), 1.0);" << std::endl;
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
    sstr << "uniform sampler2D foreground;" << std::endl;
    sstr << "uniform sampler2D background;" << std::endl;
    sstr << "uniform sampler2D bloom;" << std::endl;
    sstr << "uniform vec3 intensities;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    sstr << "    vec4 fg = texture(foreground, TexCoords.st).rgba;" << std::endl;
    sstr << "    vec3 bg = texture(background, TexCoords.st).rgb;" << std::endl;
    sstr << "    vec3 lo = texture(bloom, TexCoords.st).rgb;" << std::endl;
    sstr << "    vec3 a = min(lo * intensities, vec3(1.0, 1.0, 1.0));" << std::endl;
    sstr << "    FragColor = vec4(mix(mix(bg, fg.rgb, fg.a), bg, a), 1.0);" << std::endl;
    // sstr << "    FragColor = vec4(mix(bg, fg.rgb, fg.a) + lo * intensities, 1.0);" << std::endl;
    // sstr << "    FragColor.rgb = FragColor.rgb * 0.01 + fg.a;" << std::endl;
    // sstr << "    FragColor.rgb = FragColor.rgb * 0.01 + bg;" << std::endl;
    // sstr << "    FragColor.rgb = FragColor.rgb * 0.01 + lo;" << std::endl;
    // sstr << "    FragColor.rgb = FragColor.rgb * 0.01 + fg.rgb;" << std::endl;
    // sstr << "    FragColor.rgb = FragColor.rgb * 0.01 + mix(bg, fg.rgb, fg.a);" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

SkyBloomLogic::SkyBloomLogic(
    RenderLogic& child_logic,
    const FixedArray<float, 2>& stddev,
    const FixedArray<float, 3>& intensities)
    : child_logic_{ child_logic }
    , stddev_{ stddev }
    , intensities_{ intensities }
    , lowpass_max_{
        Lowpass::Params2{
            Lowpass::Params1{NormalParameters{.stddev = stddev(0)}},
            Lowpass::Params1{NormalParameters{.stddev = stddev(1)}}
        },
        LowpassFlavor::MAX
    }
    , foreground_fbs_{ std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
    , background_fbs_{ std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
    , bloom_fbs_{
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION),
        std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION) }
{}

SkyBloomLogic::~SkyBloomLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> SkyBloomLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic_.try_render_setup(lx, ly, frame_id);
}

bool SkyBloomLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("SkyBloomLogic::render");
    // TimeGuard time_guard{"SkyBloomLogic::render", "SkyBloomLogic::render"};
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("SkyBloomLogic did not receive standard rendering");
    }
    if (all(intensities_ == 0.f)) {
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

        if (!rp_modulate_.allocated()) {
            rp_modulate_.allocate(simple_vertex_shader_text_, modulate_fragment_shader_text());
            rp_modulate_.foreground_texture_location = rp_modulate_.get_uniform_location("foreground");
            rp_modulate_.background_texture_location = rp_modulate_.get_uniform_location("background");
        }
        if (!rp_blend_.allocated()) {
            rp_blend_.allocate(simple_vertex_shader_text_, blend_fragment_shader_text());
            rp_blend_.foreground_texture_location = rp_blend_.get_uniform_location("foreground");
            rp_blend_.background_texture_location = rp_blend_.get_uniform_location("background");
            rp_blend_.bloom_texture_color_location = rp_blend_.get_uniform_location("bloom");
            rp_blend_.intensities_location = rp_blend_.get_uniform_location("intensities");
        }

        auto width = lx.ilength();
        auto height = ly.ilength();
        foreground_fbs_->configure({
            .width = width,
            .height = height,
            .color_internal_format = GL_RGBA,
            .color_format = GL_RGBA,
            .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{ foreground_fbs_ };
            ViewportGuard vg{ width, height };

            auto fcp = frame_id;
            fcp.external_render_pass.pass = ExternalRenderPassType::STANDARD_FOREGROUND;
            child_logic_.render_auto_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                fcp,
                setup);
        }

        background_fbs_->configure({
            .width = width,
            .height = height,
            .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{ background_fbs_ };
            ViewportGuard vg{ width, height };

            auto fcp = frame_id;
            fcp.external_render_pass.pass = ExternalRenderPassType::STANDARD_BACKGROUND;
            child_logic_.render_auto_setup(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                fcp,
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

            rp_modulate_.use();

            TextureBinder tb;
            tb.bind(
                rp_modulate_.foreground_texture_location,
                *foreground_fbs_->texture_color(),
                InterpolationPolicy::NEAREST_NEIGHBOR,
                TextureLayerProperties::NONE);
            tb.bind(
                rp_modulate_.background_texture_location,
                *background_fbs_->texture_color(),
                InterpolationPolicy::NEAREST_NEIGHBOR,
                TextureLayerProperties::NONE);

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            CHK(glActiveTexture(GL_TEXTURE0));
        }
        lowpass_max_.render(width, height, fixed_ones<uint32_t, 2>(), bloom_fbs_, bloom_target_id);
        {
            notify_rendering(CURRENT_SOURCE_LOCATION);
            rp_blend_.use();

            CHK(glUniform3fv(rp_blend_.intensities_location, 1, intensities_.flat_begin()));

            TextureBinder tb;
            tb.bind(
                rp_blend_.foreground_texture_location,
                *foreground_fbs_->texture_color(),
                InterpolationPolicy::NEAREST_NEIGHBOR,
                TextureLayerProperties::NONE);
            
            tb.bind(
                rp_blend_.background_texture_location,
                *background_fbs_->texture_color(),
                InterpolationPolicy::NEAREST_NEIGHBOR,
                TextureLayerProperties::NONE);
            
            tb.bind(
                rp_blend_.bloom_texture_color_location,
                *bloom_fbs_[bloom_target_id]->texture_color(),
                InterpolationPolicy::NEAREST_NEIGHBOR,
                TextureLayerProperties::NONE);

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    return true;
}

void SkyBloomLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "SkyBloomLogic\n";
    child_logic_.print(ostr, depth + 1);
}
