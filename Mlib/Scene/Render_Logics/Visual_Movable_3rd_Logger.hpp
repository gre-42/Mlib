#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class RenderableText;

class VisualMovable3rdLogger: public DestructionObserver, public RenderLogic, public AdvanceTime {
public:
    VisualMovable3rdLogger(
        RenderLogic& scene_logic,
        SceneNode& scene_node,
        AdvanceTimes& advance_times,
        StatusWriter* status_writer,
        unsigned int log_components,
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
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float>& iv() const override;
    virtual bool requires_postprocessing() const override;

private:
    std::unique_ptr<RenderableText> renderable_text_;
    RenderLogic& scene_logic_;
    SceneNode& scene_node_;
    AdvanceTimes& advance_times_;
    StatusWriter* status_writer_;
    unsigned int log_components_;
    std::string text_;
    FixedArray<float, 2> offset_;
    float line_distance_pixels_;
    std::string ttf_filename_;
    float font_height_pixels_;
};

}
