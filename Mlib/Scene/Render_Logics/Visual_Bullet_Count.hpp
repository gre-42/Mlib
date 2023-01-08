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

class VisualBulletCount: public RenderLogic, public DestructionObserver, public RenderTextLogic, public AdvanceTime {
public:
    VisualBulletCount(
        AdvanceTimes& advance_times,
        Player& player,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        float font_height_pixels,
        float line_distance_pixels);
    virtual ~VisualBulletCount();

    virtual void notify_destroyed(Object& destroyed_object) override;

    virtual void advance_time(float dt) override;

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    AdvanceTimes& advance_times_;
    Player& player_;
    FixedArray<float, 2> size_;
    std::string text_;
    std::mutex mutex_;
};

}
