#include "Fluid_Subdomain_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

MacroscopicRenderProgram::MacroscopicRenderProgram()
    : good_momentum_magnitudes_fields(-1)
{}

MacroscopicRenderProgram::~MacroscopicRenderProgram() = default;

CollideRenderProgram::CollideRenderProgram() = default;
CollideRenderProgram::~CollideRenderProgram() = default;

StreamRenderProgram::StreamRenderProgram() = default;
StreamRenderProgram::~StreamRenderProgram() = default;

FluidSubdomainLogic::FluidSubdomainLogic(
    DanglingRef<SceneNode> skidmark_node,
    std::shared_ptr<Skidmark> skidmark,
    const FixedArray<SceneDir, 2>& velocity,
    int texture_width,
    int texture_height,
    std::chrono::steady_clock::duration velocity_dt)
    : MovingNodeLogic{ skidmark_node }
    , on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , collide_render_programs_{ uninitialized }
    , stream_render_programs_{ uninitialized }
    , good_momentum_magnitude_fields_{ uninitialized }
    , temp_momentum_magnitude_fields_{ uninitialized }
    , skidmark_{ std::move(skidmark) }
    , velocity_{ velocity }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , velocity_dt_{ velocity_dt }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

FluidSubdomainLogic::~FluidSubdomainLogic() {
    on_destroy.clear();
    deallocate();
}

void FluidSubdomainLogic::deallocate() {
    // skidmark_->texture = nullptr;
    good_momentum_magnitude_fields_ = nullptr;
    temp_momentum_magnitude_fields_ = nullptr;
    density_and_velocity_field_ = nullptr;
}

void FluidSubdomainLogic::render_moving_node(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const Bijection<TransformationMatrix<float, ScenePos, 3>>& bi,
    const FixedArray<ScenePos, 4, 4>& vp,
    const std::optional<FixedArray<float, 2>>& offset)
{
    LOG_FUNCTION("SkidmarkLogic::render");
    if (frame_id.external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        THROW_OR_ABORT("SkidmarkLogic received wrong rendering");
    }
    if (density_and_velocity_field_ == nullptr) {
        auto gray_cfg = FrameBufferConfig{
            .width = texture_width_,
            .height = texture_height_,
            .color_internal_format = GL_R16F,
            .color_type = GL_FLOAT,
            .depth_kind = FrameBufferChannelKind::NONE,
            .nsamples_msaa = 1
        };
        auto rgb_cfg = FrameBufferConfig{
            .width = texture_width_,
            .height = texture_height_,
            .color_internal_format = GL_RGBA16F,
            .color_type = GL_FLOAT,
            .depth_kind = FrameBufferChannelKind::NONE,
            .nsamples_msaa = 1
        };
        for (auto& f : good_momentum_magnitude_fields_.flat_iterable()) {
            f = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
            f->configure(gray_cfg);
        }
        for (auto& f : temp_momentum_magnitude_fields_.flat_iterable()) {
            f = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
            f->configure(gray_cfg);
        }
        density_and_velocity_field_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        density_and_velocity_field_->configure(rgb_cfg);
        initialize_momentum_magnitude_fields();
        calculate_macroscopic_variables();
    }
    if (velocity_dt_ != std::chrono::steady_clock::duration{}) {
        auto v3 = skidmark_node_->velocity(frame_id.external_render_pass.time, velocity_dt_);
        set_velocity({v3(0), v3(2)});
    }
    iterate();
    skidmark_->texture = density_and_velocity_field_->texture_color();
    skidmark_->vp = vp;
}

void FluidSubdomainLogic::iterate() {
    collide();
    stream();
    calculate_macroscopic_variables();
}

void FluidSubdomainLogic::initialize_momentum_magnitude_fields() {
    for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
        {
            RenderToFrameBufferGuard rfg{ good_momentum_magnitude_fields_(v) };
            ViewportGuard vg{ texture_width_, texture_height_ };
            clear_color({ FluidDomainLbmModel::weights[v], 1.f, 1.f, 1.f });
        }
        {
            RenderToFrameBufferGuard rfg{ temp_momentum_magnitude_fields_(v) };
            ViewportGuard vg{ texture_width_, texture_height_ };
            clear_color({ FluidDomainLbmModel::weights[v], 1.f, 1.f, 1.f });
        }
    }
}

void FluidSubdomainLogic::calculate_macroscopic_variables() {
    auto& rp = macroscopic_render_program_;
    if (!rp.allocated()) {
        std::stringstream sstr;
        for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
            sstr << "uniform sampler2D good_momentum_magnitudes_field" << v << ";" << std::endl;
        }
        sstr << "void main() {" << std::endl;
        sstr << "    float dens = 0.0;" << std::endl;
        sstr << "    vec2 mv = vec2(0.0, 0.0);" << std::endl;
        for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
            const auto& dir = FluidDomainLbmModel::discrete_velocity_directions[v];
            sstr << "    {" << std::endl;
            sstr << "        m = texture(good_momentum_magnitudes_field" << v << ", uv);" << std::endl;
            sstr << "        dens += m;" << std::endl;
            sstr << "        mv += m * vec2(" << dir(0) << ", " << dir(1) << ");" << std::endl;
            sstr << "    }" << std::endl;
        }
        sstr << "    density_field = dens;" << std::endl;
        sstr << "    velocity_field = mv / dens;" << std::endl;
        sstr << "}" << std::endl;
        rp.allocate(simple_vertex_shader_text_, sstr.str().c_str());
        for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
            rp.good_momentum_magnitudes_fields(v) = rp.get_uniform_location(
                ("good_momentum_magnitudes_field" + std::to_string(v)).c_str());
        }
    }
    rp.use();
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ density_and_velocity_field_ };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
        CHK(glUniform1i(rp.good_momentum_magnitudes_fields(v), 0));
        auto vi = integral_cast<GLenum>(GL_TEXTURE0 + v);
        CHK(glActiveTexture(vi));
        CHK(glBindTexture(GL_TEXTURE_2D, good_momentum_magnitude_fields_(v)->texture_color()->handle<GLuint>()));
    }
    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
    CHK(glActiveTexture(GL_TEXTURE0));
}

void FluidSubdomainLogic::collide() {
    for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
        auto& rp = collide_render_programs_(v);
        if (!rp.allocated()) {
            const auto& dirs = FluidDomainLbmModel::discrete_velocity_directions[v];
            const auto& weight = FluidDomainLbmModel::weights[v];
            std::stringstream sstr;
            sstr << "out float temp_momentum_magnitudes_field;" << std::endl;
            sstr << "void main() {" << std::endl;
            sstr << "    vec3 dv = density_and_velocity_field(uv).xyz;" << std::endl;
            sstr << "    vec2 flow_velocity = dv.yz;" << std::endl;
            sstr << "    float dens = dv.x;" << std::endl;
            sstr << "    float vel2 = dot(flow_velocity, flow_velocity);" << std::endl;
            sstr << "    float velocity_v = texture(good_momentum_magnitudes_field, vec3(uv, " << v << ".0));" << std::endl;
            sstr << "    float first_term = velocity_v;" << std::endl;
            sstr << "    // the flow velocity" << std::endl;
            sstr << "    float dotted = dot0d(flow_velocity, vec2(" << dirs(0) << ", " << dirs(1) << "));" << std::endl;
            sstr << "    // the taylor expainsion of equilibrium term" << std::endl;
            sstr << "    float taylor = 1 + (dotted / speed_of_sound2) + (squared(dotted) / (2 * speed_of_sound4)) -" << std::endl;
            sstr << "        (vel2 / (2 * speed_of_sound2));" << std::endl;
            sstr << "    float equilibrium = dens * taylor * " << weight << ";" << std::endl;
            sstr << "    if ((uv.x == 0.0) || (ux.x == 1.0) ||" << std::endl;
            sstr << "        (uv.y == 0.0) || (ux.y == 1.0)" << std::endl;
            sstr << "    {" << std::endl;
            sstr << "        temp_momentum_magnitudes_field = equilibrium;" << std::endl;
            sstr << "    } else {" << std::endl;
            sstr << "        float second_term = (equilibrium - velocity_v) / time_relaxation_constant;" << std::endl;
            sstr << "        temp_momentum_magnitudes_field = first_term + second_term;" << std::endl;
            sstr << "    }" << std::endl;
            sstr << "}" << std::endl;
            rp.allocate(simple_vertex_shader_text_, sstr.str().c_str());
            rp.density_and_velocity_field = rp.get_uniform_location("density_and_velocity_field");
        }
        rp.use();
        ViewportGuard vg{ texture_width_, texture_height_ };
        RenderToFrameBufferGuard rfg{ good_momentum_magnitude_fields_(v) };

        notify_rendering(CURRENT_SOURCE_LOCATION);
        CHK(glUniform1i(rp.density_and_velocity_field, 0));
        CHK(glActiveTexture(GL_TEXTURE0 + 0));
        CHK(glBindTexture(GL_TEXTURE_2D, density_and_velocity_field_->texture_color()->handle<GLuint>()));
        va().bind();
        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
        CHK(glBindVertexArray(0));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
}

void FluidSubdomainLogic::stream() {
    for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
        auto& rp = stream_render_programs_(v);
        if (!rp.allocated()) {
            const auto& dirs = FluidDomainLbmModel::discrete_velocity_directions;
            const auto& dir = dirs[v];
            std::stringstream sstr;
            sstr << SHADER_VER << FRAGMENT_PRECISION;
            sstr << "uniform sampler2D temp_momentum_magnitudes_field;" << std::endl;
            sstr << "void main() {" << std::endl;
            sstr << "    if ((uv.x == 0.0) || (ux.x == 1.0) ||" << std::endl;
            sstr << "        (uv.y == 0.0) || (ux.y == 1.0)" << std::endl;
            sstr << "    {" << std::endl;
            sstr << "        good = texture(temp, uv).r;" << std::endl;
            sstr << "    } else {" << std::endl;
            sstr << "        good = texture(temp_momentum_magnitudes_field, uv - vec2(" << dir(0) << ", " << dir(1) << "))).r;" << std::endl;
            sstr << "    }" << std::endl;
            sstr << "}" << std::endl;
            rp.allocate(simple_vertex_shader_text_, sstr.str().c_str());
            rp.temp_momentum_magnitudes_field = rp.get_uniform_location("temp_momentum_magnitudes_field");
        }
        rp.use();
        ViewportGuard vg{ texture_width_, texture_height_ };
        RenderToFrameBufferGuard rfg{ good_momentum_magnitude_fields_(v) };

        notify_rendering(CURRENT_SOURCE_LOCATION);
        CHK(glUniform1i(rp.temp_momentum_magnitudes_field, 0));
        CHK(glActiveTexture(GL_TEXTURE0 + 0));
        CHK(glBindTexture(GL_TEXTURE_2D, temp_momentum_magnitude_fields_(v)->texture_color()->handle<GLuint>()));
        va().bind();
        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
        CHK(glBindVertexArray(0));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
}

void FluidSubdomainLogic::set_velocity(const FixedArray<SceneDir, 2>& velocity) {
    std::scoped_lock lock{ velocity_mutex_ };
    velocity_ = velocity;
}


void FluidSubdomainLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FluidSubdomainLogic";
}
