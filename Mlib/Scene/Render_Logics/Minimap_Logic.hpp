#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Data_Display/Centered_Texture_Image_Logic.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <mutex>

namespace Mlib {

class SceneNode;
class IWidget;
class ILayoutPixels;
class AdvanceTimes;
class RenderLogics;
class Player;
class ObjectPool;

class MinimapLogic:
    public RenderLogic,
    public IAdvanceTime
{
public:
    MinimapLogic(
        ObjectPool& object_pool,
        AdvanceTimes& advance_times,
        RenderLogics& render_logics,
        const DanglingBaseClassRef<Player>& player,
        const DanglingBaseClassRef<SceneNode>& node,
        const VariableAndHash<std::string>& map_image_resource_name,
        const VariableAndHash<std::string>& locator_image_resource_name,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& locator_size,
        float pointer_reference_length,
        float scale,
        const FixedArray<float, 2>& size,
        const FixedArray<double, 2>& offset);
    ~MinimapLogic();

    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogics& render_logics_;
    DanglingBaseClassRef<SceneNode> node_;
    CenteredTextureImageLogic centered_texture_image_logic_;
    FillWithTextureLogic locator_logic_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& locator_size_;
    float pointer_reference_length_;
    float scale_;
    FixedArray<float, 2> size_;
    FixedArray<double, 2> offset_;
    FastMutex pose_mutex_;
    FixedArray<double, 2> position_;
    float angle_;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
};

}
