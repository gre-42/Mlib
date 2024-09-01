#include "Bloom_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gen_Shader_Text.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_Channel_Kind.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Shader_Version.hpp>
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
    sstr << "    float brightness = dot(color, brightness_threshold);" << std::endl;
    sstr << "    if (brightness > 1.0) {" << std::endl;
    sstr << "        FragColor = vec4(color.rgb, 1.0);" << std::endl;
    sstr << "    } else {" << std::endl;
    sstr << "        FragColor = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl;
    sstr << "    }" << std::endl;
    sstr << "}" << std::endl;
    return sstr.str();
}};

static GenShaderText filter_fragment_shader_text{[](size_t axis)
{
    std::stringstream sstr;
    sstr << SHADER_VER << FRAGMENT_PRECISION;
    sstr << "out vec4 FragColor;" << std::endl;
    sstr << std::endl;
    sstr << "in vec2 TexCoords;" << std::endl;
    sstr << std::endl;
    sstr << "uniform sampler2D texture_color;" << std::endl;
    sstr << "uniform float offset;" << std::endl;
    sstr << std::endl;
    sstr << "void main()" << std::endl;
    sstr << "{" << std::endl;
    if (axis == 0) {
        sstr << "    vec3 color =" << std::endl;
        sstr << "          texture(texture_color, TexCoords.st).rgb" << std::endl;
        sstr << "        + texture(texture_color, TexCoords.st + vec2(offset, 0.0)).rgb" << std::endl;
        sstr << "        + texture(texture_color, TexCoords.st - vec2(offset, 0.0)).rgb;" << std::endl;
    } else if (axis == 1) {
        sstr << "    vec3 color =" << std::endl;
        sstr << "          texture(texture_color, TexCoords.st).rgb" << std::endl;
        sstr << "        + texture(texture_color, TexCoords.st + vec2(0.0, offset)).rgb" << std::endl;
        sstr << "        + texture(texture_color, TexCoords.st - vec2(0.0, offset)).rgb;" << std::endl;
    } else {
        THROW_OR_ABORT("Unknown texture axis");
    }
    sstr << "    FragColor = vec4(color / 3.0, 1.0);" << std::endl;
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
    sstr << "    FragColor = vec4(max(screen_color, bloom_color), 1.0);" << std::endl;
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
{}

BloomLogic::~BloomLogic() {
    on_destroy.clear();
}

void BloomLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("BloomLogic::render");
    // TimeGuard time_guard{"BloomLogic::render", "BloomLogic::render"};
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("BloomLogic did not receive standard rendering");
    }
    if (all(niterations_ == 0u) || !child_logic_.requires_postprocessing()) {
        child_logic_.render(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    } else {
        assert_true(render_config.nsamples_msaa > 0);

        if (!rp_threshold_.allocated()) {
            rp_threshold_.allocate(simple_vertex_shader_text_, threshold_fragment_shader_text());
            rp_threshold_.screen_texture_color_location = rp_threshold_.get_uniform_location("screen_texture_color");
            rp_threshold_.brightness_threshold_location = rp_threshold_.get_uniform_location("brightness_threshold");
        }
        for (auto&& [axis, rp] : enumerate(rp_filter_.flat_iterable())) {
            if (!rp.allocated()) {
                rp.allocate(simple_vertex_shader_text_, filter_fragment_shader_text(axis));
                rp.texture_color_location = rp.get_uniform_location("texture_color");
                rp.lowpass_offset_location = rp.get_uniform_location("offset");
            }
        }
        if (!rp_blend_.allocated()) {
            rp_blend_.allocate(simple_vertex_shader_text_, blend_fragment_shader_text());
            rp_blend_.screen_texture_color_location = rp_blend_.get_uniform_location("screen_texture_color");
            rp_blend_.bloom_texture_color_location = rp_blend_.get_uniform_location("bloom_texture_color");
        }

        auto width = lx.ilength();
        auto height = ly.ilength();
        screen_fbs_.configure({
            .width = width,
            .height = height,
            .color_internal_format = GL_RGB32F,
            .color_type = GL_FLOAT,
            .nsamples_msaa = render_config.nsamples_msaa});
        {
            RenderToFrameBufferGuard rfg{ screen_fbs_ };
            child_logic_.render(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
        }

        for (auto& fbs : bloom_fbs_.flat_iterable()) {
            fbs.configure({
                .width = width,
                .height = height,
                .color_internal_format = GL_RGB32F,
                .color_type = GL_FLOAT,
                .depth_kind = FrameBufferChannelKind::NONE,
                .wrap_s = GL_CLAMP_TO_EDGE,
                .wrap_t = GL_CLAMP_TO_EDGE});
        }
        size_t bloom_target_id = 0;
        {
            RenderToFrameBufferGuard rfg{ bloom_fbs_(bloom_target_id) };
            RenderToScreenGuard rsg;

            rp_threshold_.use();

            CHK(glUniform1i(rp_threshold_.screen_texture_color_location, 0));
            CHK(glUniform3fv(rp_threshold_.brightness_threshold_location, 1, brightness_threshold_.flat_begin()));

            CHK(glActiveTexture(GL_TEXTURE0 + 0));
            CHK(glBindTexture(GL_TEXTURE_2D, screen_fbs_.texture_color()));

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            CHK(glActiveTexture(GL_TEXTURE0));
        }
        for (auto&& [axis, rp] : enumerate(rp_filter_.flat_iterable())) {
            rp.use();
            float offset;
            if (axis == 0) {
                offset = 1.f / integral_to_float<float>(width);
            } else if (axis == 1) {
                offset = 1.f / integral_to_float<float>(height);
            } else {
                THROW_OR_ABORT("Texture axis");
            }
            for (size_t i = 0; i < niterations_(axis); ++i) {
                size_t bloom_source_id = bloom_target_id;
                bloom_target_id = (bloom_source_id + 1) % 2;
                RenderToFrameBufferGuard rfg{ bloom_fbs_(bloom_target_id) };
                RenderToScreenGuard rsg;

                CHK(glUniform1i(rp.texture_color_location, 0));
                CHK(glUniform1f(rp.lowpass_offset_location, offset * integral_to_float<float>(i + 1)));

                CHK(glActiveTexture(GL_TEXTURE0 + 0));
                CHK(glBindTexture(GL_TEXTURE_2D, bloom_fbs_(bloom_source_id).texture_color()));

                va().bind();
                CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
                CHK(glBindVertexArray(0));

                CHK(glActiveTexture(GL_TEXTURE0));
            }
        }

        {
            RenderToScreenGuard rsg;
            rp_blend_.use();

            CHK(glUniform1i(rp_blend_.screen_texture_color_location, 0));
            CHK(glUniform1i(rp_blend_.bloom_texture_color_location, 1));

            CHK(glActiveTexture(GL_TEXTURE0 + 0));
            CHK(glBindTexture(GL_TEXTURE_2D, screen_fbs_.texture_color()));

            CHK(glActiveTexture(GL_TEXTURE0 + 1));
            CHK(glBindTexture(GL_TEXTURE_2D, bloom_fbs_(bloom_target_id).texture_color()));

            va().bind();
            CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
            CHK(glBindVertexArray(0));

            // Reset to defaults
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
}

float BloomLogic::near_plane() const {
    return child_logic_.near_plane();
}

float BloomLogic::far_plane() const {
    return child_logic_.far_plane();
}

const FixedArray<ScenePos, 4, 4>& BloomLogic::vp() const {
    return child_logic_.vp();
}

const TransformationMatrix<float, ScenePos, 3>& BloomLogic::iv() const {
    return child_logic_.iv();
}

bool BloomLogic::requires_postprocessing() const {
    return child_logic_.requires_postprocessing();
}

void BloomLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "BloomLogic\n";
    child_logic_.print(ostr, depth + 1);
}
