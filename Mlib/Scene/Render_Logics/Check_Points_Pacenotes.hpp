#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Pacenote_Reader.hpp>
#include <Mlib/Render/Data_Display/Pacenote_Display.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <shared_mutex>

namespace Mlib {

class RenderLogics;
class AdvanceTimes;
class SceneNode;
class CheckPoints;
class RenderLogicGallery;
class IWidget;

class CheckPointsPacenotes: public DestructionObserver, public AdvanceTime, public RenderLogic {
public:
    CheckPointsPacenotes(
        RenderLogicGallery& gallery,
        const std::vector<std::string>& pictures_left,
        const std::vector<std::string>& pictures_right,
        std::unique_ptr<IWidget>&& text_widget,
        std::unique_ptr<IWidget>&& picture_widget,
        const ILayoutPixels& font_height,
        const std::string& ttf_filename,
        const std::string& pacenotes_filename,
        const CheckPoints& check_points,
        size_t nlaps,
        size_t pacenotes_nread_ahead,
        RenderLogics& render_logics,
        AdvanceTimes& advance_times,
        SceneNode& moving_node);
    ~CheckPointsPacenotes();

    // AdvanceTime
    virtual void advance_time(float dt) override;
    // DestructionObserver
    virtual void notify_destroyed(const Object& destroyed_object) override;
    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::unique_ptr<IWidget> text_widget_;
    std::unique_ptr<IWidget> picture_widget_;
    const ILayoutPixels& font_height_;
    const CheckPoints* check_points_;
    PacenoteReader pacenote_reader_;
    const Pacenote* pacenote_;
    TextResource text_;
    PacenoteDisplay display_;
    RenderLogics& render_logics_;
    AdvanceTimes& advance_times_;
    SceneNode* moving_node_;
    mutable std::shared_mutex mutex_;
};

}
