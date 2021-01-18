#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class RenderingResources;

class HudImageLogic: public DestructionObserver, public FillWithTextureLogic, public AdvanceTime {
public:
    HudImageLogic(
        SceneNode& scene_node,
        AdvanceTimes& advance_times,
        const std::string& image_resource_name,
        const FixedArray<float, 2>& center,
        const FixedArray<float, 2>& size);

    virtual void notify_destroyed(void* destroyed_object) override;

    virtual void advance_time(float dt) override;

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

private:
    AdvanceTimes& advance_times_;
    FixedArray<float, 2> center_;
    FixedArray<float, 2> size_;
};

}
