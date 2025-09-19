#include "Acoustic_Velocity_Subdomain_Logic.hpp"
#include <Mlib/Geometry/Angle.hpp>
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
#include <Mlib/Render/Instance_Handles/Texture_Layer_Properties.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Logics/Diff_Vertex_Shader.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Shader_Version_3_0.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

namespace Vel {
static const float SCALE = 0.5f;
static const float OFFSET = 0.5f;
static const float ISCALE = 2.f;
static const float IOFFSET = -1.f;
}

AcousticVelocityRenderProgram::AcousticVelocityRenderProgram()
    : velocity_fields(-1)
{}

AcousticVelocityRenderProgram::~AcousticVelocityRenderProgram() = default;

AcousticVelocitySkidmarkRenderProgram::AcousticVelocitySkidmarkRenderProgram() = default;
AcousticVelocitySkidmarkRenderProgram::~AcousticVelocitySkidmarkRenderProgram() = default;

AcousticVelocitySubdomainLogic::AcousticVelocitySubdomainLogic(
    DanglingBaseClassRef<SceneNode> skidmark_node,
    std::shared_ptr<Skidmark> skidmark,
    const FixedArray<SceneDir, 2>& directional_velocity,
    float radial_velocity,
    float angular_velocity,
    const AxisAlignedBoundingBox<float, 2>& velocity_region,
    int texture_width,
    int texture_height,
    std::chrono::steady_clock::duration velocity_dt,
    float c,
    float dt,
    float dx,
    float intensity_normalization,
    float reference_inner_directional_velocity,
    float maximum_inner_velocity,
    const BoundaryLimitation& boundary_limitation,
    float skidmark_strength)
    : MovingNodeLogic{ skidmark_node }
    , on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , velocity_fields_{ uninitialized }
    , offset_velocity_renderer_{ 2 }
    , skidmark_{ std::move(skidmark) }
    , directional_velocity_{ directional_velocity }
    , radial_velocity_{ radial_velocity }
    , angular_velocity_{ angular_velocity }
    , angle_{ 0.f }
    , velocity_region_{ velocity_region }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , velocity_dt_{ velocity_dt }
    , c_{ c }
    , dt_{ dt }
    , dx_{ dx }
    , intensity_normalization_{ intensity_normalization }
    , reference_inner_directional_velocity_{ reference_inner_directional_velocity }
    , maximum_inner_velocity_{ maximum_inner_velocity }
    , boundary_limitation_{ boundary_limitation }
    , skidmark_strength_{ skidmark_strength }
    , i012_{ 0 }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

AcousticVelocitySubdomainLogic::~AcousticVelocitySubdomainLogic() {
    on_destroy.clear();
    deallocate();
}

void AcousticVelocitySubdomainLogic::deallocate() {
    // skidmark_->texture = nullptr;
    velocity_fields_ = nullptr;
    temp_velocity_field_ = nullptr;
    skidmark_field_ = nullptr;
}

void AcousticVelocitySubdomainLogic::render_moving_node(
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
    if (!any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
        THROW_OR_ABORT("SkidmarkLogic received wrong rendering");
    }
    if (velocity_dt_ != std::chrono::steady_clock::duration{0}) {
        auto v3 = skidmark_node_->velocity(frame_id.external_render_pass.time, velocity_dt_);
        auto v2 = FixedArray<SceneDir, 2>{v3(0), v3(2)};
        auto l = std::sqrt(sum(squared(v2)));
        if (l > reference_inner_directional_velocity_) {
            v2 /= l;
        } else {
            v2 /= reference_inner_directional_velocity_;
        }
        set_directional_velocity(v2 * maximum_inner_velocity_);
    }
    if (skidmark_field_ == nullptr) {
        auto velocity_cfg = FrameBufferConfig{
            .width = texture_width_,
            .height = texture_height_,
            .color_internal_format = GL_RGB16F,
            .color_format = GL_RGB,
            .color_type =  GL_HALF_FLOAT,
            .color_magnifying_interpolation_mode = InterpolationMode::NEAREST,
            .depth_kind = FrameBufferChannelKind::NONE,
            .wrap_s = GL_CLAMP_TO_EDGE,
            .wrap_t = GL_CLAMP_TO_EDGE,
            .nsamples_msaa = 1
        };
        auto skidmark_cfg = FrameBufferConfig{
            .width = texture_width_,
            .height = texture_height_,
            .color_internal_format = GL_RGB16F,
            .color_format = GL_RGB,
            .color_type =  GL_HALF_FLOAT,
            .color_magnifying_interpolation_mode = InterpolationMode::LINEAR,
            .depth_kind = FrameBufferChannelKind::NONE,
            .wrap_s = GL_CLAMP_TO_EDGE,
            .wrap_t = GL_CLAMP_TO_EDGE,
            .nsamples_msaa = 1
        };
        for (auto& f : velocity_fields_.flat_iterable()) {
            f = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
            f->configure(velocity_cfg);
        }
        temp_velocity_field_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        temp_velocity_field_->configure(velocity_cfg);
        skidmark_field_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        skidmark_field_->configure(skidmark_cfg);
        initialize_velocity_fields();
    }
    iterate(offset.has_value() ? *offset : fixed_zeros<float, 2>());
    skidmark_->texture = skidmark_field_->texture_color();
    skidmark_->vp = vp;
}

void AcousticVelocitySubdomainLogic::iterate(const FixedArray<float, 2>& offset) {
    apply_offset(offset);
    collide_and_stream();
    calculate_skidmark_field();
    i012_ = (i012_ + 1) % 3;
}

void AcousticVelocitySubdomainLogic::save_debug_images(const std::string& prefix) {
    StbImage3(skidmark_field_->color_to_stb_image(3)).save_to_file(prefix + "skidmark.png");
    for (size_t v = 0; v < 3; ++v) {
        StbImage1(velocity_fields_(v)->color_to_stb_image(2)).save_to_file(prefix + "velocity_" + std::to_string(v) + ".png");
    }
}

void AcousticVelocitySubdomainLogic::initialize_velocity_fields() {
    for (size_t v = 0; v < 3; ++v) {
        RenderToFrameBufferGuard rfg{ velocity_fields_(v) };
        ViewportGuard vg{ texture_width_, texture_height_ };
        clear_color({ 0.f, 0.f, 1.f, 1.f });
    }
}

void AcousticVelocitySubdomainLogic::apply_offset(const FixedArray<float, 2>& offset) {
    for (auto& f : velocity_fields_.flat_iterable()) {
        offset_velocity_renderer_.apply_offset(
            offset,
            texture_width_,
            texture_height_,
            f,
            temp_velocity_field_);
    }
}

void AcousticVelocitySubdomainLogic::collide_and_stream() {
    auto& rp = acoustic_render_program_;
    if (!rp.allocated()) {
        auto vs = diff_vertex_shader(texture_width_, texture_height_);
        
        std::stringstream fs;
        fs << std::fixed;
        fs << SHADER_VER << FRAGMENT_PRECISION;
        fs << "out vec2 u_2;" << std::endl;
        fs << "in vec2 TexCoords0;" << std::endl;
        for (int dim = 0; dim < 2; ++dim) {
            for (int h = 0; h < 2; ++h) {
                fs << "in vec2 TexCoords" << dim << h << ';' << std::endl;
            }
        }
        fs << "uniform vec2 inner_directional_velocity;" << std::endl;
        fs << "uniform float inner_radial_velocity;" << std::endl;
        fs << "uniform vec2 inner_min;" << std::endl;
        fs << "uniform vec2 inner_max;" << std::endl;
        fs << "uniform vec2 inner_center;" << std::endl;
        fs << "uniform float idx_c_dt_2;" << std::endl;
        fs << "uniform float intensity_normalization;" << std::endl;
        for (size_t t = 0; t < 2; ++t) {
            fs << "uniform sampler2D velocity_field" << t << ';' << std::endl;
        }
        fs << "void main() {" << std::endl;
        fs << "    vec2 radial_vector = TexCoords0 - inner_center;" << std::endl;
        fs << "    float rlen = length(radial_vector);" << std::endl;
        fs << "    if (all(greaterThan(TexCoords0, inner_min)) && all(lessThan(TexCoords0, inner_max))) {" << std::endl;
        fs << "        u_2 = inner_directional_velocity;" << std::endl;
        fs << "        if (rlen > 1e-6) {" << std::endl;
        fs << "            u_2 += (inner_radial_velocity / rlen) * radial_vector;" << std::endl;
        fs << "        }" << std::endl;
        fs << "    } else {" << std::endl;
        for (size_t t = 0; t < 2; ++t) {
            fs << "        vec2 u_" << t << " = texture(velocity_field" << t << ", TexCoords0).rg * " << Vel::ISCALE << " + " << Vel::IOFFSET << ';' << std::endl;
        }
        for (int dim = 0; dim < 2; ++dim) {
            for (int h = 0; h < 2; ++h) {
                fs << "        vec2 u_" << dim << h << " = texture(velocity_field1, TexCoords" << dim << h << ").rg * " << Vel::ISCALE << " + " << Vel::IOFFSET << ';' << std::endl;
            }
        }
        fs << "        vec2 Lu = u_00 + u_01 + u_10 + u_11 - 4.0 * u_1;" << std::endl;
        fs << "        u_2 = Lu * idx_c_dt_2 + 2.0 * u_1 - u_0;" << std::endl;
        fs << "        u_2 *= intensity_normalization;" << std::endl;
        fs << "    }" << std::endl;
        float vm = boundary_limitation_.max;
        float fo = boundary_limitation_.falloff;
        float rn = 1 - fo;
        fs << "    float max_len = " << vm << " * max(1.0 - max(2.0 * rlen - " << fo << ", 0.0) / " << rn << ", 0.0);" << std::endl;
        fs << "    float len = length(u_2);" << std::endl;
        fs << "    if (len > max_len) {" << std::endl;
        fs << "        u_2 *= max_len / len;" << std::endl;
        fs << "    }" << std::endl;
        fs << "    u_2 = u_2 * " << Vel::SCALE << " + " << Vel::OFFSET << ';' << std::endl;
        fs << "}" << std::endl;
        // linfo() << "--------- collide_and_stream -----------";
        // lraw() << vs.str();
        // lraw() << fs.str();
        rp.allocate(vs.c_str(), fs.str().c_str());
        rp.inner_directional_velocity = rp.get_uniform_location("inner_directional_velocity");
        rp.inner_radial_velocity = rp.get_uniform_location("inner_radial_velocity");
        rp.inner_min = rp.get_uniform_location("inner_min");
        rp.inner_max = rp.get_uniform_location("inner_max");
        rp.inner_center = rp.get_uniform_location("inner_center");
        rp.idx_c_dt_2 = rp.get_uniform_location("idx_c_dt_2");
        rp.intensity_normalization = rp.get_uniform_location("intensity_normalization");
        for (size_t t = 0; t < 2; ++t) {
            rp.velocity_fields(t) = rp.get_uniform_location(
                ("velocity_field" + std::to_string(t)).c_str());
        }
    }
    rp.use();
    {
        angle_ = normalized_radians(angle_ + angular_velocity_);
        std::scoped_lock lock{ velocity_mutex_ };
        CHK(glUniform2fv(rp.inner_directional_velocity, 1, directional_velocity_.flat_begin()));
        CHK(glUniform1f(rp.inner_radial_velocity, radial_velocity_ * std::sin(angle_)));
        CHK(glUniform2fv(rp.inner_min, 1, velocity_region_.min.flat_begin()));
        CHK(glUniform2fv(rp.inner_max, 1, velocity_region_.max.flat_begin()));
        CHK(glUniform2fv(rp.inner_center, 1, velocity_region_.center().flat_begin()));
    }
    CHK(glUniform1f(rp.idx_c_dt_2, squared(c_ * dt_ / dx_)));
    CHK(glUniform1f(rp.intensity_normalization, intensity_normalization_));
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ velocity_fields_(i012_) };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    TextureBinder tb;
    for (size_t t = 0; t < 2; ++t) {
        tb.bind(
            rp.velocity_fields(t),
            *velocity_fields_((i012_ + 1 + t) % 3)->texture_color(),
            InterpolationPolicy::NEAREST_NEIGHBOR,
            TextureLayerProperties::NONE);
    }
    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
    CHK(glActiveTexture(GL_TEXTURE0));
}

void AcousticVelocitySubdomainLogic::calculate_skidmark_field() {
    auto& rp = skidmark_render_program_;
    if (!rp.allocated()) {
        auto vs = diff_vertex_shader(texture_width_, texture_height_);

        std::stringstream fs;
        fs << std::fixed;
        fs << SHADER_VER << FRAGMENT_PRECISION;
        fs << "out vec3 skidmark_field;" << std::endl;
        fs << "in vec2 TexCoords00;" << std::endl;
        fs << "in vec2 TexCoords01;" << std::endl;
        fs << "in vec2 TexCoords10;" << std::endl;
        fs << "in vec2 TexCoords11;" << std::endl;
        fs << "uniform float skidmark_strength;" << std::endl;
        fs << "uniform sampler2D velocity_field;" << std::endl;
        fs << "void main() {" << std::endl;
        // fs << "    vec2 alpha = texture(velocity_field, TexCoords0).rg;" << std::endl;
        // fs << "    vec2 vel = alpha * " << Vel::ISCALE << " + " << Vel::IOFFSET << ';' << std::endl;
        // fs << "    skidmark_field.rgb = vec3(0.0, 0.0, 100 * dot(vel, vel));" << std::endl;
        fs << "    float div =" << std::endl;
        fs << "        texture(velocity_field, TexCoords01).r -" << std::endl;
        fs << "        texture(velocity_field, TexCoords00).r +" << std::endl;
        fs << "        texture(velocity_field, TexCoords11).g -" << std::endl;
        fs << "        texture(velocity_field, TexCoords10).g;" << std::endl;
        fs << "    div = div * " << Vel::ISCALE << ';' << std::endl;
        fs << "    float color = skidmark_strength * div + 0.5;" << std::endl;
        fs << "    skidmark_field.rgb = vec3(color, color, color);" << std::endl;
        fs << "}" << std::endl;
        // linfo() << "--------- calculate_skidmark_field -----------";
        // lraw() << fs.str();
        rp.allocate(vs.c_str(), fs.str().c_str());
        rp.skidmark_strength = rp.get_uniform_location("skidmark_strength");
        rp.velocity_field = rp.get_uniform_location("velocity_field");
    }
    rp.use();
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ skidmark_field_ };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    CHK(glUniform1f(rp.skidmark_strength, skidmark_strength_));
    TextureBinder tb;
    tb.bind(
        rp.velocity_field,
        *velocity_fields_(i012_)->texture_color(),
        InterpolationPolicy::NEAREST_NEIGHBOR,
        TextureLayerProperties::NONE);
    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
    CHK(glActiveTexture(GL_TEXTURE0));
}

void AcousticVelocitySubdomainLogic::set_directional_velocity(const FixedArray<SceneDir, 2>& directional_velocity) {
    std::scoped_lock lock{ velocity_mutex_ };
    directional_velocity_ = directional_velocity;
}

void AcousticVelocitySubdomainLogic::set_radial_velocity(SceneDir radial_velocity) {
    std::scoped_lock lock{ velocity_mutex_ };
    radial_velocity_ = radial_velocity;
}

void AcousticVelocitySubdomainLogic::set_velocity_region(const AxisAlignedBoundingBox<float, 2>& velocity_region) {
    std::scoped_lock lock{ velocity_mutex_ };
    velocity_region_ = velocity_region;
}

void AcousticVelocitySubdomainLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "AcousticVelocitySubdomainLogic";
}
