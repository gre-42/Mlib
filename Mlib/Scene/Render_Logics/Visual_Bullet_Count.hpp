#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>

namespace Mlib {

class AdvanceTimes;
class TextResource;
class Player;
class RenderLogics;
class IWidget;
class ObjectPool;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;

class VisualBulletCount:
    public RenderLogic,
    public RenderTextLogic,
    public IAdvanceTime
{
public:
    VisualBulletCount(
        ObjectPool& object_pool,
        AdvanceTimes& advance_times,
        RenderLogics& render_logics,
        const DanglingBaseClassRef<Player>& player,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter);
    virtual ~VisualBulletCount();

    virtual void advance_time(float dt, const StaticWorld& world) override;

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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    AdvanceTimes& advance_times_;
    RenderLogics& render_logics_;
    DanglingBaseClassRef<Player> player_;
    std::unique_ptr<IWidget> widget_;
    std::string text_;
    FastMutex mutex_;
    FocusFilter focus_filter_;
};

}
