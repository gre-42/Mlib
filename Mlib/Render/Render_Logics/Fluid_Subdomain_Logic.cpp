#include "Fluid_Subdomain_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Binder.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

namespace Dens {
static const float SCALE = 0.5f;
static const float ISCALE = 2.f;
}

namespace Vel {
static const float SCALE = 0.5f;
static const float OFFSET = 0.5f;
static const float ISCALE = 2.f;
static const float IOFFSET = -1.f;
}

MacroscopicRenderProgram::MacroscopicRenderProgram()
    : good_momentum_magnitude_fields(-1)
{}

MacroscopicRenderProgram::~MacroscopicRenderProgram() = default;

CollideRenderProgram::CollideRenderProgram() = default;
CollideRenderProgram::~CollideRenderProgram() = default;

StreamRenderProgram::StreamRenderProgram() = default;
StreamRenderProgram::~StreamRenderProgram() = default;

FluidSkidmarkRenderProgram::FluidSkidmarkRenderProgram() = default;
FluidSkidmarkRenderProgram::~FluidSkidmarkRenderProgram() = default;

FluidSubdomainLogic::FluidSubdomainLogic(
    DanglingRef<SceneNode> skidmark_node,
    std::shared_ptr<Skidmark> skidmark,
    const FixedArray<SceneDir, 2>& velocity_vector,
    float angular_velocity,
    const AxisAlignedBoundingBox<float, 2>& velocity_region,
    int texture_width,
    int texture_height,
    std::chrono::steady_clock::duration velocity_dt,
    float speed_of_sound,
    float time_relaxation_constant,
    float density_normalization,
    float reference_inner_velocity,
    float maximum_inner_velocity)
    : MovingNodeLogic{ skidmark_node }
    , on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , collide_render_programs_{ uninitialized }
    , stream_render_programs_{ uninitialized }
    , good_momentum_magnitude_fields_{ uninitialized }
    , temp_momentum_magnitude_fields_{ uninitialized }
    , skidmark_{ std::move(skidmark) }
    , velocity_vector_{ velocity_vector }
    , angular_velocity_{ angular_velocity }
    , angle_{ 0.f }
    , velocity_region_{ velocity_region }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , velocity_dt_{ velocity_dt }
    , speed_of_sound_{ speed_of_sound / std::sqrt(3.f) }
    , speed_of_sound2_{ squared(speed_of_sound_) }
    , speed_of_sound4_{ squared(speed_of_sound2_) }
    , time_relaxation_constant_{ time_relaxation_constant }
    , density_normalization_{ density_normalization }
    , reference_inner_velocity_{ reference_inner_velocity }
    , maximum_inner_velocity_{ maximum_inner_velocity }
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
    skidmark_field_ = nullptr;
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
    if (velocity_dt_ != std::chrono::steady_clock::duration{0}) {
        auto v3 = skidmark_node_->velocity(frame_id.external_render_pass.time, velocity_dt_);
        auto v2 = FixedArray<SceneDir, 2>{v3(0), v3(2)};
        auto l = std::sqrt(sum(squared(v2)));
        if (l > reference_inner_velocity_) {
            v2 /= l;
        } else {
            v2 /= reference_inner_velocity_;
        }
        set_velocity_vector(v2 * maximum_inner_velocity_);
    }
    if (density_and_velocity_field_ == nullptr) {
        auto gray_cfg = FrameBufferConfig{
            .width = texture_width_,
            .height = texture_height_,
            .color_internal_format = GL_R16F,
            .color_format = GL_RED,
            .color_type =  GL_HALF_FLOAT,
            .color_filter_type = GL_NEAREST,
            .depth_kind = FrameBufferChannelKind::NONE,
            .wrap_s = GL_CLAMP_TO_EDGE,
            .wrap_t = GL_CLAMP_TO_EDGE,
            .nsamples_msaa = 1
        };
        auto rgb_cfg = FrameBufferConfig{
            .width = texture_width_,
            .height = texture_height_,
            .color_internal_format = GL_RGB16F,
            .color_format = GL_RGB,
            .color_type =  GL_HALF_FLOAT,
            .color_filter_type = GL_NEAREST,
            .depth_kind = FrameBufferChannelKind::NONE,
            .wrap_s = GL_CLAMP_TO_EDGE,
            .wrap_t = GL_CLAMP_TO_EDGE,
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
        skidmark_field_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        skidmark_field_->configure(rgb_cfg);
        initialize_momentum_magnitude_fields();
        calculate_macroscopic_variables();
    }
    iterate();
    skidmark_->texture = skidmark_field_->texture_color();
    skidmark_->vp = vp;
}

void FluidSubdomainLogic::iterate() {
    collide();
    stream();
    calculate_macroscopic_variables();
    calculate_skidmark_field();
}

void FluidSubdomainLogic::save_debug_images(const std::string& prefix) {
    StbImage3(density_and_velocity_field_->color_to_stb_image(3)).save_to_file(prefix + "density_and_velocity.png");
    for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
        StbImage1(good_momentum_magnitude_fields_(v)->color_to_stb_image(1)).save_to_file(prefix + "good_" + std::to_string(v) + ".png");
        StbImage1(temp_momentum_magnitude_fields_(v)->color_to_stb_image(1)).save_to_file(prefix + "temp_" + std::to_string(v) + ".png");
    }
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
        sstr << SHADER_VER << FRAGMENT_PRECISION;
        sstr << "out vec3 density_and_velocity_field;" << std::endl;
        sstr << "in vec2 TexCoords;" << std::endl;
        sstr << "uniform vec2 inner_velocity;" << std::endl;
        sstr << "uniform vec2 inner_min;" << std::endl;
        sstr << "uniform vec2 inner_max;" << std::endl;
        for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
            sstr << "uniform sampler2D good_momentum_magnitude_field" << v << ';' << std::endl;
        }
        sstr << "void main() {" << std::endl;
        sstr << "    float dens = 0.0;" << std::endl;
        sstr << "    vec2 mv = vec2(0.0, 0.0);" << std::endl;
        for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
            const auto& dir = FluidDomainLbmModel::discrete_velocity_directions[v];
            sstr << "    {" << std::endl;
            sstr << "        float m = texture(good_momentum_magnitude_field" << v << ", TexCoords).r;" << std::endl;
            sstr << "        dens += m;" << std::endl;
            sstr << "        mv += m * vec2(" << dir(0) << ", " << dir(1) << ");" << std::endl;
            sstr << "    }" << std::endl;
        }
        sstr << "    density_and_velocity_field.x = (1 + (dens - 1) * " << density_normalization_ << ") * " << Dens::SCALE << ';' << std::endl;
        sstr << "    if (all(greaterThan(TexCoords, inner_min)) && all(lessThan(TexCoords, inner_max))) {" << std::endl;
        sstr << "        density_and_velocity_field.yz = inner_velocity * " << Vel::SCALE << " + " << Vel::OFFSET << ';' << std::endl;
        sstr << "    } else {" << std::endl;
        sstr << "        density_and_velocity_field.yz = mv / dens * " << Vel::SCALE << " + " << Vel::OFFSET << ';' << std::endl;
        sstr << "    }" << std::endl;
        sstr << "}" << std::endl;
        // linfo() << "--------- calculate_macroscopic_variables -----------";
        // lraw() << sstr.str();
        rp.allocate(simple_vertex_shader_text_, sstr.str().c_str());
        rp.inner_velocity = rp.get_uniform_location("inner_velocity");
        rp.inner_min = rp.get_uniform_location("inner_min");
        rp.inner_max = rp.get_uniform_location("inner_max");
        for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
            rp.good_momentum_magnitude_fields(v) = rp.get_uniform_location(
                ("good_momentum_magnitude_field" + std::to_string(v)).c_str());
        }
    }
    rp.use();
    {
        angle_ = std::fmod(angle_ + angular_velocity_, (float)(2 * M_PI));
        std::scoped_lock lock{ velocity_mutex_ };
        CHK(glUniform2fv(rp.inner_velocity, 1, (velocity_vector_ * std::sin(angle_)).flat_begin()));
        CHK(glUniform2fv(rp.inner_min, 1, velocity_region_.min.flat_begin()));
        CHK(glUniform2fv(rp.inner_max, 1, velocity_region_.max.flat_begin()));
    }
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ density_and_velocity_field_ };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    TextureBinder tb;
    for (size_t v = 0; v < FluidDomainLbmModel::ndirections; ++v) {
        tb.bind(rp.good_momentum_magnitude_fields(v), *good_momentum_magnitude_fields_(v)->texture_color());
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
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
            sstr << SHADER_VER << FRAGMENT_PRECISION;
            sstr << "out float temp_momentum_magnitude_field;" << std::endl;
            sstr << "in vec2 TexCoords;" << std::endl;
            sstr << "uniform sampler2D density_and_velocity_field;" << std::endl;
            sstr << "uniform sampler2D good_momentum_magnitude_field;" << std::endl;
            sstr << "uniform float speed_of_sound2;" << std::endl;
            sstr << "uniform float speed_of_sound4;" << std::endl;
            sstr << "uniform float time_relaxation_constant;" << std::endl;
            sstr << "void main() {" << std::endl;
            sstr << "    vec3 dv = texture(density_and_velocity_field, TexCoords).xyz;" << std::endl;
            sstr << "    vec2 flow_velocity = dv.yz * " << Vel::ISCALE << " + " << Vel::IOFFSET << ';' << std::endl;
            sstr << "    float dens = dv.x * " << Dens::ISCALE << ';' << std::endl;
            sstr << "    float vel2 = dot(flow_velocity, flow_velocity);" << std::endl;
            sstr << "    float velocity_v = texture(good_momentum_magnitude_field, TexCoords).r;" << std::endl;
            sstr << "    float first_term = velocity_v;" << std::endl;
            sstr << "    // the flow velocity" << std::endl;
            sstr << "    float dotted = dot(flow_velocity, vec2(" << dirs(0) << ", " << dirs(1) << "));" << std::endl;
            sstr << "    // the taylor expainsion of equilibrium term" << std::endl;
            sstr << "    float taylor = 1 + (dotted / speed_of_sound2) + ((dotted * dotted) / (2 * speed_of_sound4)) -" << std::endl;
            sstr << "        (vel2 / (2 * speed_of_sound2));" << std::endl;
            sstr << "    float equilibrium = dens * taylor * " << weight << ';' << std::endl;
            sstr << "    if ((TexCoords.x == 0.0) || (TexCoords.x == 1.0) ||" << std::endl;
            sstr << "        (TexCoords.y == 0.0) || (TexCoords.y == 1.0))" << std::endl;
            sstr << "    {" << std::endl;
            // sstr << "        temp_momentum_magnitude_field = equilibrium;" << std::endl;
            sstr << "        temp_momentum_magnitude_field = " << weight << ';' << std::endl;
            sstr << "    } else {" << std::endl;
            sstr << "        float second_term = (equilibrium - velocity_v) / time_relaxation_constant;" << std::endl;
            sstr << "        temp_momentum_magnitude_field = first_term + second_term;" << std::endl;
            sstr << "    }" << std::endl;
            sstr << "}" << std::endl;
            // linfo() << "--------- collide " << v << " -----------";
            // lraw() << sstr.str();
            rp.allocate(simple_vertex_shader_text_, sstr.str().c_str());
            rp.density_and_velocity_field = rp.get_uniform_location("density_and_velocity_field");
            rp.good_momentum_magnitude_field = rp.get_uniform_location("good_momentum_magnitude_field");
            rp.speed_of_sound2 = rp.get_uniform_location("speed_of_sound2");
            rp.speed_of_sound4 = rp.get_uniform_location("speed_of_sound4");
            rp.time_relaxation_constant = rp.get_uniform_location("time_relaxation_constant");
        }
        rp.use();
        ViewportGuard vg{ texture_width_, texture_height_ };
        RenderToFrameBufferGuard rfg{ temp_momentum_magnitude_fields_(v) };

        notify_rendering(CURRENT_SOURCE_LOCATION);
        CHK(glUniform1fv(rp.speed_of_sound2, 1, &speed_of_sound2_));
        CHK(glUniform1fv(rp.speed_of_sound4, 1, &speed_of_sound4_));
        CHK(glUniform1fv(rp.time_relaxation_constant, 1, &time_relaxation_constant_));
        TextureBinder tb;
        tb.bind(rp.density_and_velocity_field, *density_and_velocity_field_->texture_color());
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        tb.bind(rp.good_momentum_magnitude_field, *good_momentum_magnitude_fields_(v)->texture_color());
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
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

            std::stringstream vs;
            vs << SHADER_VER;
            vs << "layout (location = 0) in vec2 aPos;" << std::endl;
            vs << "layout (location = 1) in vec2 aTexCoords;" << std::endl;
            vs << std::endl;
            vs << "out vec2 TexCoords0;" << std::endl;
            vs << "out vec2 TexCoords1;" << std::endl;
            vs << std::endl;
            vs << "void main()" << std::endl;
            vs << "{" << std::endl;
            vs << "    TexCoords0 = aTexCoords;" << std::endl;
            vs << "    TexCoords1 = aTexCoords - vec2(" <<
                dir(0) / (float)texture_width_ << ", " << dir(1) / (float)texture_height_ << ");" << std::endl;
            vs << "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);" << std::endl;
            vs << "}" << std::endl;

            std::stringstream fs;
            fs << SHADER_VER << FRAGMENT_PRECISION;
            fs << "out float good_momentum_magnitude_field;" << std::endl;
            fs << "in vec2 TexCoords0;" << std::endl;
            fs << "in vec2 TexCoords1;" << std::endl;
            fs << "uniform sampler2D temp_momentum_magnitude_field;" << std::endl;
            fs << "void main() {" << std::endl;
            fs << "    if ((TexCoords0.x == 0.0) || (TexCoords0.x == 1.0) ||" << std::endl;
            fs << "        (TexCoords0.y == 0.0) || (TexCoords0.y == 1.0))" << std::endl;
            fs << "    {" << std::endl;
            fs << "        good_momentum_magnitude_field = texture(temp_momentum_magnitude_field, TexCoords0).r;" << std::endl;
            fs << "    } else {" << std::endl;
            fs << "        good_momentum_magnitude_field = texture(temp_momentum_magnitude_field, TexCoords1).r;" << std::endl;
            fs << "    }" << std::endl;
            fs << "}" << std::endl;
            // linfo() << "--------- stream vs " << v << " -----------";
            // lraw() << vs.str();
            // linfo() << "--------- stream fs " << v << " -----------";
            // lraw() << fs.str();
            rp.allocate(vs.str().c_str(), fs.str().c_str());
            rp.temp_momentum_magnitude_field = rp.get_uniform_location("temp_momentum_magnitude_field");
        }
        rp.use();
        ViewportGuard vg{ texture_width_, texture_height_ };
        RenderToFrameBufferGuard rfg{ good_momentum_magnitude_fields_(v) };

        notify_rendering(CURRENT_SOURCE_LOCATION);
        TextureBinder tb;
        tb.bind(rp.temp_momentum_magnitude_field, *temp_momentum_magnitude_fields_(v)->texture_color());
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        va().bind();
        CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
        CHK(glBindVertexArray(0));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
}

void FluidSubdomainLogic::calculate_skidmark_field() {
    auto& rp = skidmark_render_program_;
    if (!rp.allocated()) {
        std::stringstream sstr;
        sstr << SHADER_VER << FRAGMENT_PRECISION;
        sstr << "out vec3 skidmark_field;" << std::endl;
        sstr << "in vec2 TexCoords;" << std::endl;
        sstr << "uniform sampler2D density_and_velocity_field;" << std::endl;
        sstr << "void main() {" << std::endl;
        sstr << "    float alpha = clamp((texture(density_and_velocity_field, TexCoords).r * " << Dens::ISCALE << " - 0.7) / 0.6, 0.0, 1.0);" << std::endl;
        // sstr << "    float alpha = texture(density_and_velocity_field, TexCoords).r;" << std::endl;
        sstr << "    skidmark_field.rgb = vec3(0, 1 - alpha, alpha);" << std::endl;
        sstr << "}" << std::endl;
        // linfo() << "--------- calculate_skidmark_field -----------";
        // lraw() << sstr.str();
        rp.allocate(simple_vertex_shader_text_, sstr.str().c_str());
        rp.density_and_velocity_field = rp.get_uniform_location("density_and_velocity_field");
    }
    rp.use();
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ skidmark_field_ };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    TextureBinder tb;
    tb.bind(rp.density_and_velocity_field, *density_and_velocity_field_->texture_color());
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
    CHK(glActiveTexture(GL_TEXTURE0));
}

void FluidSubdomainLogic::set_velocity_vector(const FixedArray<SceneDir, 2>& velocity_vector) {
    std::scoped_lock lock{ velocity_mutex_ };
    velocity_vector_ = velocity_vector;
}

void FluidSubdomainLogic::set_velocity_region(const AxisAlignedBoundingBox<float, 2>& velocity_region) {
    std::scoped_lock lock{ velocity_mutex_ };
    velocity_region_ = velocity_region;
}

void FluidSubdomainLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FluidSubdomainLogic";
}
