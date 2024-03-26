#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <memory>

namespace Mlib {

enum class StatusComponents;
class AdvanceTimes;
class SceneNode;
class TextResource;
class ILayoutPixels;

class VisualMovable3rdLogger: public RenderLogic, public DestructionObserver<DanglingRef<SceneNode>>, public IAdvanceTime, public DanglingBaseClass {
public:
    VisualMovable3rdLogger(
        RenderLogic& scene_logic,
        DanglingRef<SceneNode> scene_node,
        AdvanceTimes& advance_times,
        StatusWriter& status_writer,
        StatusComponents log_components,
        std::string ttf_filename,
        const FixedArray<float, 2>& offset,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance);
    virtual ~VisualMovable3rdLogger();

    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override;

    virtual void advance_time(float dt) override;

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::unique_ptr<TextResource> renderable_text_;
    RenderLogic& scene_logic_;
    DanglingPtr<SceneNode> scene_node_;
    AdvanceTimes& advance_times_;
    StatusWriter& status_writer_;
    StatusComponents log_components_;
    ThreadSafeString text_;
    FixedArray<float, 2> offset_;
    const ILayoutPixels& line_distance_;
    std::string ttf_filename_;
    const ILayoutPixels& font_height_;
};

}
