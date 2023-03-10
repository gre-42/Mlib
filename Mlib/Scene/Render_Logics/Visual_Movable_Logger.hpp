#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <list>
#include <memory>

namespace Mlib {

class VisualMovableLoggerView;
class AdvanceTimes;

class VisualMovableLogger: public RenderLogic, public DestructionObserver, public AdvanceTime {
public:
    explicit VisualMovableLogger(AdvanceTimes& advance_times);
    virtual ~VisualMovableLogger();

    void add_logger(std::unique_ptr<VisualMovableLoggerView>&& logger);

    virtual void notify_destroyed(const Object& destroyed_object) override;

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
    AdvanceTimes& advance_times_;
    std::list<std::unique_ptr<VisualMovableLoggerView>> loggers_;
};

}
