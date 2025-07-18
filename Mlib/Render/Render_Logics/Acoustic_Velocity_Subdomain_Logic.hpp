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
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <cmath>
#include <string>

namespace Mlib {

class FrameBuffer;
class SceneNode;
class FillWithTextureLogic;
struct Skidmark;

struct AcousticVelocityRenderProgram: public RenderProgram {
    AcousticVelocityRenderProgram(const AcousticVelocityRenderProgram&) = delete;
    AcousticVelocityRenderProgram& operator = (const AcousticVelocityRenderProgram&) = delete;
public:
    AcousticVelocityRenderProgram();
    ~AcousticVelocityRenderProgram();
    GLint inner_directional_velocity = -1;
    GLint inner_radial_velocity = -1;
    GLint inner_min = -1;
    GLint inner_max = -1;
    GLint inner_center = -1;
    GLint idx_c_dt_2 = -1;
    GLint intensity_normalization = -1;
    FixedArray<GLint, 3> velocity_fields;
};

struct AcousticVelocitySkidmarkRenderProgram: public RenderProgram {
    AcousticVelocitySkidmarkRenderProgram(const AcousticVelocitySkidmarkRenderProgram&) = delete;
    AcousticVelocitySkidmarkRenderProgram& operator = (const AcousticVelocitySkidmarkRenderProgram&) = delete;
public:
    AcousticVelocitySkidmarkRenderProgram();
    ~AcousticVelocitySkidmarkRenderProgram();
    GLint skidmark_strength = -1;
    GLint velocity_field = -1;
};

class AcousticVelocitySubdomainLogic final: public MovingNodeLogic, private GenericPostProcessingLogic {
public:
    AcousticVelocitySubdomainLogic(
        DanglingRef<SceneNode> skidmark_node,
        std::shared_ptr<Skidmark> skidmark,
        const FixedArray<SceneDir, 2>& directional_velocity,
        float radial_velocity,
        float angular_velocity,
        const AxisAlignedBoundingBox<float, 2>& velocity_region,
        int texture_width,
        int texture_height,
        std::chrono::steady_clock::duration velocity_dt,
        float c = 1.1f,
        float dt = 0.01f,
        float dx = 0.1f,
        float intensity_normalization = 0.99f,
        float reference_inner_directional_velocity = 50 * kph,
        float maximum_inner_velocity = 0.2f,
        const BoundaryLimitation& boundary_limitation = BoundaryLimitation{},
        float skidmark_strength = 1.f);
    virtual ~AcousticVelocitySubdomainLogic();

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

    void set_directional_velocity(const FixedArray<SceneDir, 2>& directional_velocity);
    void set_radial_velocity(float radial_velocity);
    void set_velocity_region(const AxisAlignedBoundingBox<float, 2>& velocity_region);
    void save_debug_images(const std::string& prefix);

    DestructionFunctionsRemovalTokens on_skidmark_node_clear;
private:
    void iterate(const FixedArray<float, 2>& offset);
    void apply_offset(const FixedArray<float, 2>& offset);
    void initialize_velocity_fields();
    void collide_and_stream();
    void calculate_skidmark_field();
    void deallocate();
    AcousticVelocityRenderProgram acoustic_render_program_;
    FixedArray<std::shared_ptr<FrameBuffer>, 3> velocity_fields_;
    OffsetRenderer offset_velocity_renderer_;
    std::shared_ptr<FrameBuffer> temp_velocity_field_;
    std::shared_ptr<FrameBuffer> skidmark_field_;
    AcousticVelocitySkidmarkRenderProgram skidmark_render_program_;
    std::shared_ptr<Skidmark> skidmark_;
    mutable FastMutex velocity_mutex_;
    FixedArray<SceneDir, 2> directional_velocity_;
    float radial_velocity_;
    float angular_velocity_;
    float angle_;
    AxisAlignedBoundingBox<float, 2> velocity_region_;
    int texture_width_;
    int texture_height_;
    std::chrono::steady_clock::duration velocity_dt_;
    float c_;
    float dt_;
    float dx_;
    float intensity_normalization_;
    float reference_inner_directional_velocity_;
    float maximum_inner_velocity_;
    BoundaryLimitation boundary_limitation_;
    float skidmark_strength_;
    size_t i012_;
    DeallocationToken deallocation_token_;
};

}
