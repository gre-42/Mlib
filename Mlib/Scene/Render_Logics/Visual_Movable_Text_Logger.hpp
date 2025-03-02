#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Scene/Render_Logics/Visual_Movable_Logger_View.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

namespace Mlib {

enum class StatusComponents;
class TextResource;
class StatusWriter;
class IWidget;
class ExpressionWatcher;

class VisualMovableTextLogger: public VisualMovableLoggerView, private RenderTextLogic, public virtual DanglingBaseClass {
public:
    VisualMovableTextLogger(
        StatusWriter& status_writer,
        StatusComponents log_components,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance);
    virtual ~VisualMovableTextLogger();

    virtual void advance_time(float dt, const StaticWorld& world) override;

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

private:
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    StatusWriter& status_writer_;
    StatusComponents log_components_;
    ThreadSafeString text_;
    std::unique_ptr<IWidget> widget_;
};

}
