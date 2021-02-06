#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <memory>

namespace Mlib {

enum class StatusComponents;
class AdvanceTimes;
class SceneNode;
class TextResource;

class VisualMovable3rdLogger: public DestructionObserver, public RenderLogic, public AdvanceTime {
public:
    VisualMovable3rdLogger(
        RenderLogic& scene_logic,
        SceneNode& scene_node,
        AdvanceTimes& advance_times,
        StatusWriter* status_writer,
        StatusComponents log_components,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& offset,
        float font_height_pixels,
        float line_distance_pixels);
    virtual ~VisualMovable3rdLogger();

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
    std::unique_ptr<TextResource> renderable_text_;
    RenderLogic& scene_logic_;
    SceneNode& scene_node_;
    AdvanceTimes& advance_times_;
    StatusWriter* status_writer_;
    StatusComponents log_components_;
    std::string text_;
    FixedArray<float, 2> offset_;
    float line_distance_pixels_;
    std::string ttf_filename_;
    float font_height_pixels_;
};

}
