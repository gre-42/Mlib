#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Cfd/Lbm/D2q9.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Moving_Node_Logic.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <cmath>
#include <string>

namespace Mlib {

class FrameBuffer;
class SceneNode;
class FillWithTextureLogic;
class IParticleRenderer;
struct Skidmark;
using FluidDomainLbmModel = LbmModelD2Q9<float>;

struct MacroscopicRenderProgram: public RenderProgram {
    MacroscopicRenderProgram(const MacroscopicRenderProgram&) = delete;
    MacroscopicRenderProgram& operator = (const MacroscopicRenderProgram&) = delete;
public:
    MacroscopicRenderProgram();
    ~MacroscopicRenderProgram();
    GLint inner_velocity = -1;
    GLint inner_min = -1;
    GLint inner_max = -1;
    FixedArray<GLint, FluidDomainLbmModel::ndirections> good_momentum_magnitude_fields;
};

struct CollideRenderProgram: public RenderProgram {
    CollideRenderProgram(const CollideRenderProgram&) = delete;
    CollideRenderProgram& operator = (const CollideRenderProgram&) = delete;
public:
    CollideRenderProgram();
    ~CollideRenderProgram();
    GLint density_and_velocity_field = -1;
    GLint good_momentum_magnitude_field = -1;
    GLint speed_of_sound2 = -1;
    GLint speed_of_sound4 = -1;
    GLint time_relaxation_constant = -1;
};

struct StreamRenderProgram: public RenderProgram {
    StreamRenderProgram(const StreamRenderProgram&) = delete;
    StreamRenderProgram& operator = (const StreamRenderProgram&) = delete;
public:
    StreamRenderProgram();
    ~StreamRenderProgram();
    GLint temp_momentum_magnitude_field = -1;
};

struct SkidmarkRenderProgram: public RenderProgram {
    SkidmarkRenderProgram(const SkidmarkRenderProgram&) = delete;
    SkidmarkRenderProgram& operator = (const SkidmarkRenderProgram&) = delete;
public:
    SkidmarkRenderProgram();
    ~SkidmarkRenderProgram();
    GLint density_and_velocity_field = -1;
};

class FluidSubdomainLogic final: public MovingNodeLogic, private GenericPostProcessingLogic {
    constexpr static const float speed_of_sound = 1.f / std::sqrt(3.f);
    constexpr static const float speed_of_sound2 = squared(speed_of_sound);
    constexpr static const float speed_of_sound4 = squared(speed_of_sound2);
    constexpr static const float time_relaxation_constant = 0.55f;
public:
    FluidSubdomainLogic(
        DanglingRef<SceneNode> skidmark_node,
        std::shared_ptr<Skidmark> skidmark,
        const FixedArray<SceneDir, 2>& velocity_vector,
        float angular_velocity,
        const AxisAlignedBoundingBox<float, 2>& velocity_region,
        int texture_width,
        int texture_height,
        std::chrono::steady_clock::duration velocity_dt);
    virtual ~FluidSubdomainLogic();

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

    void set_velocity_vector(const FixedArray<SceneDir, 2>& velocity_vector);
    void set_velocity_region(const AxisAlignedBoundingBox<float, 2>& velocity_region);
    void save_debug_images(const std::string& prefix);

    DestructionFunctionsRemovalTokens on_skidmark_node_clear;
private:
    void iterate();
    void initialize_momentum_magnitude_fields();
    void calculate_macroscopic_variables();
    void collide();
    void stream();
    void calculate_skidmark_field();
    void deallocate();
    MacroscopicRenderProgram macroscopic_render_program_;
    FixedArray<CollideRenderProgram, FluidDomainLbmModel::ndirections> collide_render_programs_;
    FixedArray<StreamRenderProgram, FluidDomainLbmModel::ndirections> stream_render_programs_;
    FixedArray<std::shared_ptr<FrameBuffer>, FluidDomainLbmModel::ndirections> good_momentum_magnitude_fields_;
    FixedArray<std::shared_ptr<FrameBuffer>, FluidDomainLbmModel::ndirections> temp_momentum_magnitude_fields_;
    std::shared_ptr<FrameBuffer> density_and_velocity_field_;
    std::shared_ptr<FrameBuffer> skidmark_field_;
    SkidmarkRenderProgram skidmark_render_program_;
    std::shared_ptr<Skidmark> skidmark_;
    mutable FastMutex velocity_mutex_;
    FixedArray<SceneDir, 2> velocity_vector_;
    float angular_velocity_;
    float angle_;
    AxisAlignedBoundingBox<float, 2> velocity_region_;
    int texture_width_;
    int texture_height_;
    std::chrono::steady_clock::duration velocity_dt_;
    std::shared_ptr<FillWithTextureLogic> old_render_texture_logic_;
    DeallocationToken deallocation_token_;
};

}
