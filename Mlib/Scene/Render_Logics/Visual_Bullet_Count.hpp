#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <mutex>

namespace Mlib {

class AdvanceTimes;
class TextResource;
class Player;
class IWidget;

class VisualBulletCount: public RenderLogic, public DestructionObserver<DanglingRef<const SceneNode>>, public RenderTextLogic, public AdvanceTime {
public:
    VisualBulletCount(
        AdvanceTimes& advance_times,
        Player& player,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance);
    virtual ~VisualBulletCount();

    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;

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
    Player& player_;
    std::unique_ptr<IWidget> widget_;
    std::string text_;
    std::mutex mutex_;
};

}
