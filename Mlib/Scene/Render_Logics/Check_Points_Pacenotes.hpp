#pragma once
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Misc/Pacenote_Reader.hpp>
#include <Mlib/Render/Data_Display/Pacenote_Display.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <optional>
#include <vector>

namespace Mlib {

class SceneNode;
class CheckPoints;
class RenderLogicGallery;
class IWidget;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;

class CheckPointsPacenotes: public IAdvanceTime, public RenderLogic {
public:
    CheckPointsPacenotes(
        RenderLogicGallery& gallery,
        const std::vector<std::string>& pictures_left,
        const std::vector<std::string>& pictures_right,
        const ILayoutPixels& widget_distance,
        std::unique_ptr<IWidget>&& text_widget,
        std::unique_ptr<IWidget>&& picture_widget,
        const ILayoutPixels& font_height,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        const FixedArray<float, 3>& font_color,
        const std::string& pacenotes_filename,
        const DanglingBaseClassRef<const CheckPoints>& check_points,
        size_t nlaps,
        double pacenotes_meters_read_ahead,
        double pacenotes_minimum_covered_meters,
        size_t pacenotes_maximum_number,
        FocusFilter focus_filter);
    ~CheckPointsPacenotes();

    // IAdvanceTime
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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void preload() const;

private:
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    const ILayoutPixels& widget_distance_;
    std::unique_ptr<IWidget> text_widget_;
    std::unique_ptr<IWidget> picture_widget_;
    const ILayoutPixels& font_height_;
    DanglingBaseClassPtr<const CheckPoints> check_points_;
    PacenoteReader pacenote_reader_;
    std::vector<const Pacenote*> pacenotes_;
    TextResource text_;
    PacenoteDisplay display_;
    FocusFilter focus_filter_;
    DestructionFunctionsRemovalTokens on_destroy_check_points_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
