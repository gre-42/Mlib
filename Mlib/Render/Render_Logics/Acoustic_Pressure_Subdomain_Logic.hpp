#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Boundary_Limitation.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Moving_Node_Logic.hpp>
#include <Mlib/Render/Render_Logics/Offset_Renderer.hpp>
#include <Mlib/Render/Render_Logics/Periodicity.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cmath>
#include <string>

namespace Mlib {

class ITextureHandle;
class FrameBuffer;
class SceneNode;
class FillWithTextureLogic;
struct Skidmark;

struct AcousticPressureWindRenderProgram: public RenderProgram {
    AcousticPressureWindRenderProgram(const AcousticPressureWindRenderProgram&) = delete;
    AcousticPressureWindRenderProgram& operator = (const AcousticPressureWindRenderProgram&) = delete;
public:
    AcousticPressureWindRenderProgram();
    ~AcousticPressureWindRenderProgram();
    GLint wind_texture = -1;
};

struct AcousticPressureRenderProgram: public RenderProgram {
    AcousticPressureRenderProgram(const AcousticPressureRenderProgram&) = delete;
    AcousticPressureRenderProgram& operator = (const AcousticPressureRenderProgram&) = delete;
public:
    AcousticPressureRenderProgram();
    ~AcousticPressureRenderProgram();
    GLint inner_pressure = -1;
    GLint inner_min = -1;
    GLint inner_max = -1;
    GLint inner_center = -1;
    GLint idx_c_dt_2 = -1;
    GLint intensity_normalization = -1;
    FixedArray<GLint, 3> pressure_fields;
    GLint wind_amplitude = -1;
    GLint wind_field = -1;
};

struct AcousticPressureSkidmarkRenderProgram: public RenderProgram {
    AcousticPressureSkidmarkRenderProgram(const AcousticPressureSkidmarkRenderProgram&) = delete;
    AcousticPressureSkidmarkRenderProgram& operator = (const AcousticPressureSkidmarkRenderProgram&) = delete;
public:
    AcousticPressureSkidmarkRenderProgram();
    ~AcousticPressureSkidmarkRenderProgram();
    GLint skidmark_strength = -1;
    GLint pressure_field = -1;
};

class AcousticPressureSubdomainLogic final: public MovingNodeLogic, private GenericPostProcessingLogic {
public:
    AcousticPressureSubdomainLogic(
        DanglingBaseClassRef<SceneNode> skidmark_node,
        std::shared_ptr<Skidmark> skidmark,
        float inner_pressure,
        float inner_angular_velocity,
        const AxisAlignedBoundingBox<float, 2>& inner_region,
        float wind_amplitude,
        float wind_angular_velocity,
        float wind_cutoff,
        std::shared_ptr<ITextureHandle> wind_texture,
        int texture_width,
        int texture_height,
        float c = 1.1f,
        float dt = 0.01f,
        float dx = 0.1f,
        float intensity_normalization = 0.99f,
        const BoundaryLimitation& boundary_limitation = BoundaryLimitation{},
        Periodicity periodicity = Periodicity::APERIODIC,
        float skidmark_strength = 1.f);
    virtual ~AcousticPressureSubdomainLogic();

    virtual void render_moving_node(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const Bijection<TransformationMatrix<float, ScenePos, 3>>& bi,
        const FixedArray<ScenePos, 4, 4>& vp,
        const std::optional<FixedArray<float, 2>>& offset) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_inner_pressure(float inner_pressure);
    void set_inner_region(const AxisAlignedBoundingBox<float, 2>& inner_region);
    void save_debug_images(const std::string& prefix);

    DestructionFunctionsRemovalTokens on_skidmark_node_clear;
private:
    void iterate(const FixedArray<float, 2>& offset);
    void apply_offset(const FixedArray<float, 2>& offset);
    void initialize_fields();
    void collide_and_stream();
    void calculate_skidmark_field();
    void deallocate();
    AcousticPressureWindRenderProgram wind_render_program_;
    AcousticPressureRenderProgram acoustic_render_program_;
    FixedArray<std::shared_ptr<FrameBuffer>, 3> pressure_fields_;
    std::shared_ptr<ITextureHandle> wind_texture_;
    std::shared_ptr<FrameBuffer> wind_field_;
    OffsetRenderer offset_grayscale_renderer_;
    std::shared_ptr<FrameBuffer> temp_pressure_field_;
    std::shared_ptr<FrameBuffer> temp_wind_field_;
    std::shared_ptr<FrameBuffer> skidmark_field_;
    AcousticPressureSkidmarkRenderProgram skidmark_render_program_;
    std::shared_ptr<Skidmark> skidmark_;
    mutable FastMutex inner_mutex_;
    float inner_pressure_;
    float inner_angular_velocity_;
    float inner_angle_;
    AxisAlignedBoundingBox<float, 2> inner_region_;
    float wind_amplitude_;
    float wind_angular_velocity_;
    float wind_cutoff_;
    float wind_angle_;
    int texture_width_;
    int texture_height_;
    float c_;
    float dt_;
    float dx_;
    float intensity_normalization_;
    BoundaryLimitation boundary_limitation_;
    Periodicity periodicity_;
    float skidmark_strength_;
    size_t i012_;
    DeallocationToken deallocation_token_;
};

}
