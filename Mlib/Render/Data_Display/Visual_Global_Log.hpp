#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

class TextResource;
class BaseLog;
enum class LogEntrySeverity;
class IWidget;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;

class VisualGlobalLog: public RenderLogic, public RenderTextLogic {
public:
    VisualGlobalLog(
        BaseLog& base_log,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        size_t nentries,
        LogEntrySeverity severity);
    virtual ~VisualGlobalLog();

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
    BaseLog& base_log_;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    size_t nentries_;
    LogEntrySeverity severity_;
    std::unique_ptr<IWidget> widget_;
};

}
