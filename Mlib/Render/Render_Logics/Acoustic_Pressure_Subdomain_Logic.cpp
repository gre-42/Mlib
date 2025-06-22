#include "Acoustic_Pressure_Subdomain_Logic.hpp"
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

namespace Pres {
static const float SCALE = 0.5f;
static const float OFFSET = 0.5f;
static const float ISCALE = 2.f;
static const float IOFFSET = -1.f;
}

AcousticPressureRenderProgram::AcousticPressureRenderProgram()
    : pressure_fields(-1)
{}

AcousticPressureRenderProgram::~AcousticPressureRenderProgram() = default;

AcousticPressureSkidmarkRenderProgram::AcousticPressureSkidmarkRenderProgram() = default;
AcousticPressureSkidmarkRenderProgram::~AcousticPressureSkidmarkRenderProgram() = default;

AcousticPressureSubdomainLogic::AcousticPressureSubdomainLogic(
    DanglingRef<SceneNode> skidmark_node,
    std::shared_ptr<Skidmark> skidmark,
    float inner_pressure,
    float angular_velocity,
    const AxisAlignedBoundingBox<float, 2>& inner_region,
    int texture_width,
    int texture_height,
    float c,
    float dt,
    float dx,
    float intensity_normalization,
    const BoundaryLimitation& boundary_limitation,
    float skidmark_strength)
    : MovingNodeLogic{ skidmark_node }
    , on_skidmark_node_clear{ skidmark_node->on_clear, CURRENT_SOURCE_LOCATION }
    , pressure_fields_{ uninitialized }
    , offset_pressure_renderer_{ 1 }
    , skidmark_{ std::move(skidmark) }
    , inner_pressure_{ inner_pressure }
    , angular_velocity_{ angular_velocity }
    , angle_{ 0.f }
    , inner_region_{ inner_region }
    , texture_width_{ texture_width }
    , texture_height_{ texture_height }
    , c_{ c }
    , dt_{ dt }
    , dx_{ dx }
    , intensity_normalization_{ intensity_normalization }
    , boundary_limitation_{ boundary_limitation }
    , skidmark_strength_{ skidmark_strength }
    , i012_{ 0 }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

AcousticPressureSubdomainLogic::~AcousticPressureSubdomainLogic() {
    on_destroy.clear();
    deallocate();
}

void AcousticPressureSubdomainLogic::deallocate() {
    // skidmark_->texture = nullptr;
    pressure_fields_ = nullptr;
    temp_pressure_field_ = nullptr;
    skidmark_field_ = nullptr;
}

void AcousticPressureSubdomainLogic::render_moving_node(
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
    if (skidmark_field_ == nullptr) {
        auto r_cfg = FrameBufferConfig{
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
        for (auto& f : pressure_fields_.flat_iterable()) {
            f = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
            f->configure(r_cfg);
        }
        temp_pressure_field_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        temp_pressure_field_->configure(r_cfg);
        skidmark_field_ = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        skidmark_field_->configure(rgb_cfg);
        initialize_pressure_fields();
    }
    iterate(offset.has_value() ? *offset : fixed_zeros<float, 2>());
    skidmark_->texture = skidmark_field_->texture_color();
    skidmark_->vp = vp;
}

void AcousticPressureSubdomainLogic::iterate(const FixedArray<float, 2>& offset) {
    apply_offset(offset);
    collide_and_stream();
    calculate_skidmark_field();
    i012_ = (i012_ + 1) % 3;
}

void AcousticPressureSubdomainLogic::save_debug_images(const std::string& prefix) {
    StbImage3(skidmark_field_->color_to_stb_image(3)).save_to_file(prefix + "skidmark.png");
    for (size_t v = 0; v < 3; ++v) {
        StbImage1(pressure_fields_(v)->color_to_stb_image(1)).save_to_file(prefix + "pressure_" + std::to_string(v) + ".png");
    }
}

void AcousticPressureSubdomainLogic::initialize_pressure_fields() {
    for (size_t v = 0; v < 3; ++v) {
        RenderToFrameBufferGuard rfg{ pressure_fields_(v) };
        ViewportGuard vg{ texture_width_, texture_height_ };
        clear_color({ 0.f, 1.f, 1.f, 1.f });
    }
}

void AcousticPressureSubdomainLogic::apply_offset(const FixedArray<float, 2>& offset) {
    for (auto& f : pressure_fields_.flat_iterable()) {
        offset_pressure_renderer_.apply_offset(
            offset,
            texture_width_,
            texture_height_,
            f,
            temp_pressure_field_);
    }
}

void AcousticPressureSubdomainLogic::collide_and_stream() {
    auto& rp = acoustic_render_program_;
    if (!rp.allocated()) {
        auto vs = diff_vertex_shader(texture_width_, texture_height_);
        
        std::stringstream fs;
        fs << SHADER_VER << FRAGMENT_PRECISION;
        fs << "out float p_2;" << std::endl;
        fs << "in vec2 TexCoords0;" << std::endl;
        for (int dim = 0; dim < 2; ++dim) {
            for (int h = 0; h < 2; ++h) {
                fs << "in vec2 TexCoords" << dim << h << ';' << std::endl;
            }
        }
        fs << "uniform float inner_pressure;" << std::endl;
        fs << "uniform vec2 inner_min;" << std::endl;
        fs << "uniform vec2 inner_max;" << std::endl;
        fs << "uniform vec2 inner_center;" << std::endl;
        fs << "uniform float idx_c_dt_2;" << std::endl;
        fs << "uniform float intensity_normalization;" << std::endl;
        for (size_t t = 0; t < 2; ++t) {
            fs << "uniform sampler2D pressure_field" << t << ';' << std::endl;
        }
        fs << "void main() {" << std::endl;
        for (size_t t = 0; t < 2; ++t) {
            fs << "    float p_" << t << " = texture(pressure_field" << t << ", TexCoords0).r * " << Pres::ISCALE << " + " << Pres::IOFFSET << ';' << std::endl;
        }
        for (int dim = 0; dim < 2; ++dim) {
            for (int h = 0; h < 2; ++h) {
                fs << "    float p_" << dim << h << " = texture(pressure_field1, TexCoords" << dim << h << ").r * " << Pres::ISCALE << " + " << Pres::IOFFSET << ';' << std::endl;
            }
        }
        fs << "    float Lp = p_00 + p_01 + p_10 + p_11 - 4 * p_1;" << std::endl;
        fs << "    p_2 = Lp * idx_c_dt_2 + 2 * p_1 - p_0;" << std::endl;
        fs << "    vec2 radial_vector = TexCoords0 - inner_center;" << std::endl;
        fs << "    float rlen = length(radial_vector);" << std::endl;
        fs << "    if (all(greaterThan(TexCoords0, inner_min)) && all(lessThan(TexCoords0, inner_max))) {" << std::endl;
        fs << "        p_2 += inner_pressure;" << std::endl;
        fs << "    }" << std::endl;
        fs << "    p_2 *= intensity_normalization;" << std::endl;
        float pm = boundary_limitation_.max;
        float fo = boundary_limitation_.falloff;
        float rn = 1 - fo;
        fs << "    float max_len = " << pm << " * max(1.0 - max(2.0 * rlen - " << fo << ", 0.0) / " << rn << ", 0.0);" << std::endl;
        fs << "    float len = abs(p_2);" << std::endl;
        fs << "    if (len > max_len) {" << std::endl;
        fs << "        p_2 *= max_len / len;" << std::endl;
        fs << "    }" << std::endl;
        fs << "    p_2 = p_2 * " << Pres::SCALE << " + " << Pres::OFFSET << ';' << std::endl;
        fs << "}" << std::endl;
        // linfo() << "--------- collide_and_stream -----------";
        // lraw() << vs.str();
        // lraw() << fs.str();
        rp.allocate(vs.c_str(), fs.str().c_str());
        rp.inner_pressure = rp.get_uniform_location("inner_pressure");
        rp.inner_min = rp.get_uniform_location("inner_min");
        rp.inner_max = rp.get_uniform_location("inner_max");
        rp.inner_center = rp.get_uniform_location("inner_center");
        rp.idx_c_dt_2 = rp.get_uniform_location("idx_c_dt_2");
        rp.intensity_normalization = rp.get_uniform_location("intensity_normalization");
        for (size_t t = 0; t < 2; ++t) {
            rp.pressure_fields(t) = rp.get_uniform_location(
                ("pressure_field" + std::to_string(t)).c_str());
        }
    }
    rp.use();
    {
        angle_ = std::fmod(angle_ + angular_velocity_, (float)(2 * M_PI));
        std::scoped_lock lock{ inner_mutex_ };
        CHK(glUniform1f(rp.inner_pressure, inner_pressure_ * std::sin(angle_)));
        CHK(glUniform2fv(rp.inner_min, 1, inner_region_.min.flat_begin()));
        CHK(glUniform2fv(rp.inner_max, 1, inner_region_.max.flat_begin()));
        CHK(glUniform2fv(rp.inner_center, 1, inner_region_.center().flat_begin()));
    }
    CHK(glUniform1f(rp.idx_c_dt_2, squared(c_ * dt_ / dx_)));
    CHK(glUniform1f(rp.intensity_normalization, intensity_normalization_));
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ pressure_fields_(i012_) };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    TextureBinder tb;
    for (size_t t = 0; t < 2; ++t) {
        tb.bind(rp.pressure_fields(t), *pressure_fields_((i012_ + 1 + t) % 3)->texture_color());
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
    CHK(glActiveTexture(GL_TEXTURE0));
}

void AcousticPressureSubdomainLogic::calculate_skidmark_field() {
    auto& rp = skidmark_render_program_;
    if (!rp.allocated()) {
        std::stringstream fs;
        fs << SHADER_VER << FRAGMENT_PRECISION;
        fs << "out vec3 skidmark_field;" << std::endl;
        fs << "in vec2 TexCoords;" << std::endl;
        fs << "uniform float skidmark_strength;" << std::endl;
        fs << "uniform sampler2D pressure_field;" << std::endl;
        fs << "void main() {" << std::endl;
        fs << "    float p = texture(pressure_field, TexCoords).r;" << std::endl;
        fs << "    p = p * " << Pres::ISCALE << " + " << Pres::IOFFSET << ';' << std::endl;
        fs << "    float color = skidmark_strength * p + 0.5;" << std::endl;
        fs << "    skidmark_field.rgb = vec3(color, color, color);" << std::endl;
        fs << "}" << std::endl;
        // linfo() << "--------- calculate_skidmark_field -----------";
        // lraw() << fs.str();
        rp.allocate(simple_vertex_shader_text_, fs.str().c_str());
        rp.skidmark_strength = rp.get_uniform_location("skidmark_strength");
        rp.pressure_field = rp.get_uniform_location("pressure_field");
    }
    rp.use();
    ViewportGuard vg{ texture_width_, texture_height_ };
    RenderToFrameBufferGuard rfg{ skidmark_field_ };

    notify_rendering(CURRENT_SOURCE_LOCATION);
    CHK(glUniform1f(rp.skidmark_strength, skidmark_strength_));
    TextureBinder tb;
    tb.bind(rp.pressure_field, *pressure_fields_(i012_)->texture_color());
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    va().bind();
    CHK(glDrawArrays(GL_TRIANGLES, 0, 6));
    CHK(glBindVertexArray(0));
    CHK(glActiveTexture(GL_TEXTURE0));
}

void AcousticPressureSubdomainLogic::set_inner_pressure(SceneDir inner_pressure) {
    std::scoped_lock lock{ inner_mutex_ };
    inner_pressure_ = inner_pressure;
}

void AcousticPressureSubdomainLogic::set_inner_region(const AxisAlignedBoundingBox<float, 2>& inner_region) {
    std::scoped_lock lock{ inner_mutex_ };
    inner_region_ = inner_region;
}

void AcousticPressureSubdomainLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "AcousticPressureSubdomainLogic";
}
