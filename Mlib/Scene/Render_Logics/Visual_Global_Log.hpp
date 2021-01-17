#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

class RenderableText;
class BaseLog;
enum class LogEntrySeverity;

class VisualGlobalLog: public RenderLogic, public RenderTextLogic {
public:
    VisualGlobalLog(
        BaseLog& base_log,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        size_t nentries,
        LogEntrySeverity severity);
    virtual ~VisualGlobalLog();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

private:
    BaseLog& base_log_;
    size_t nentries_;
    LogEntrySeverity severity_;
};

}
