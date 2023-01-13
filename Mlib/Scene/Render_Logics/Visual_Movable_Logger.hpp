#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

enum class StatusComponents;
class AdvanceTimes;
class TextResource;
class StatusWriter;
class IWidget;

class VisualMovableLogger: public RenderLogic, public DestructionObserver, public RenderTextLogic, public AdvanceTime {
public:
    VisualMovableLogger(
        AdvanceTimes& advance_times,
        StatusWriter* status_writer,
        StatusComponents log_components,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutScalar& font_height,
        const ILayoutScalar& line_distance);
    virtual ~VisualMovableLogger();

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
    AdvanceTimes& advance_times_;
    StatusWriter* status_writer_;
    StatusComponents log_components_;
    std::string text_;
    std::unique_ptr<IWidget> widget_;
};

}
