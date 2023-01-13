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
class ILayoutScalar;

class VisualMovable3rdLogger: public RenderLogic, public DestructionObserver, public AdvanceTime {
public:
    VisualMovable3rdLogger(
        RenderLogic& scene_logic,
        SceneNode& scene_node,
        AdvanceTimes& advance_times,
        StatusWriter* status_writer,
        StatusComponents log_components,
        std::string ttf_filename,
        const FixedArray<float, 2>& offset,
        const ILayoutScalar& font_height,
        const ILayoutScalar& line_distance);
    virtual ~VisualMovable3rdLogger();

    virtual void notify_destroyed(Object& destroyed_object) override;

    virtual void advance_time(float dt) override;

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::unique_ptr<TextResource> renderable_text_;
    RenderLogic& scene_logic_;
    SceneNode& scene_node_;
    AdvanceTimes& advance_times_;
    StatusWriter* status_writer_;
    StatusComponents log_components_;
    std::string text_;
    FixedArray<float, 2> offset_;
    const ILayoutScalar& line_distance_;
    std::string ttf_filename_;
    const ILayoutScalar& font_height_;
};

}
