#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

enum class StatusComponents;
class AdvanceTimes;
class SceneNode;
class TextResource;
class StatusWriter;

class VisualMovableLogger: public RenderLogic, public DestructionObserver, public RenderTextLogic, public AdvanceTime {
public:
    VisualMovableLogger(
        SceneNode& scene_node,
        AdvanceTimes& advance_times,
        StatusWriter* status_writer,
        StatusComponents log_components,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels);
    virtual ~VisualMovableLogger();

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
    StatusWriter* status_writer_;
    StatusComponents log_components_;
    std::string text_;
};

}
