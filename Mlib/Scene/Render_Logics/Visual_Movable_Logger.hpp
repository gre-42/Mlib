#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <list>
#include <memory>

namespace Mlib {

class VisualMovableLoggerView;
class AdvanceTimes;
class RenderLogics;
class Player;
class ObjectPool;

class VisualMovableLogger:
    public RenderLogic,
    public DestructionObserver<SceneNode&>,
    public IAdvanceTime
{
public:
    explicit VisualMovableLogger(
        ObjectPool& object_pool,
        AdvanceTimes& advance_times,
        RenderLogics& render_logics,
        DanglingRef<SceneNode> node,
        DanglingBaseClassPtr<Player> player,
        FocusFilter focus_filter);
    virtual ~VisualMovableLogger();

    void add_logger(std::unique_ptr<VisualMovableLoggerView>&& logger);

    virtual void notify_destroyed(SceneNode& destroyed_object) override;

    virtual void advance_time(float dt, const StaticWorld& world) override;

    // RenderLogic
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
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    DanglingBaseClassPtr<Player> player_;
    DanglingRef<SceneNode> node_;
    AdvanceTimes& advance_times_;
    RenderLogics& render_logics_;
    DestructionFunctionsRemovalTokens on_node_clear_;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
    std::list<std::unique_ptr<VisualMovableLoggerView>> loggers_;
    FocusFilter focus_filter_;
};

}
