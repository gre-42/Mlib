#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

enum class StatusComponents;
class RenderLogics;
class AdvanceTimes;
class SceneNode;
class TextResource;
class ILayoutPixels;
template <typename TData, size_t... tshape>
class FixedArray;

class VisualMovable3rdLogger: public RenderLogic, public DestructionObserver<SceneNode&>, public IAdvanceTime {
public:
    VisualMovable3rdLogger(
        RenderLogic& scene_logic,
        const DanglingRef<SceneNode>& scene_node,
        RenderLogics& render_logics,
        AdvanceTimes& advance_times,
        StatusWriter& status_writer,
        StatusComponents log_components,
        std::string ttf_filename,
        const FixedArray<float, 2>& offset,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance);
    virtual ~VisualMovable3rdLogger();

    virtual void notify_destroyed(SceneNode& destroyed_object) override;

    virtual void advance_time(float dt, const StaticWorld& world) override;

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    DestructionFunctionsRemovalTokens on_node_clear;
private:
    std::unique_ptr<TextResource> renderable_text_;
    RenderLogic& scene_logic_;
    FastMutex mutex_;
    DanglingPtr<SceneNode> scene_node_;
    StatusWriter& status_writer_;
    StatusComponents log_components_;
    ThreadSafeString text_;
    FixedArray<float, 2> offset_;
    FixedArray<float, 3> font_color_;
    const ILayoutPixels& line_distance_;
    std::string ttf_filename_;
    const ILayoutPixels& font_height_;
};

}
