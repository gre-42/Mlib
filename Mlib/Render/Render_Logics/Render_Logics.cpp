#include "Render_Logics.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

static std::map<ZorderAndId, DestructionFunctionsTokensObject<RenderLogic>>::iterator
    find_render_logic(const RenderLogic& render_logic, std::map<ZorderAndId, DestructionFunctionsTokensObject<RenderLogic>>& lst) {
    return std::find_if(
        lst.begin(),
        lst.end(),
        [&render_logic](auto& v){ return v.second.get() == &render_logic; });
}

RenderLogics::RenderLogics(UiFocus& ui_focus)
    : ui_focus_{ ui_focus }
    , next_smallest_id_{ 0 }
    , next_largest_id_{ 1 }
{}

RenderLogics::~RenderLogics() {
    on_destroy.clear();
    if (!render_logics_.empty()) {
        for (const auto& [_, r] : render_logics_) {
            r->on_destroy.print_source_locations();
        }
        verbose_abort("Render-logics remaining");
    }
}

std::optional<RenderSetup> RenderLogics::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    LOG_FUNCTION("RenderLogics::init");
    return std::nullopt;
}

void RenderLogics::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("RenderLogics::render");

    std::shared_lock lock{ mutex_ };

    for (const auto& [_, c] : render_logics_) {
        if ([this, &c=c](){
            std::shared_lock lock{ ui_focus_.focuses.mutex };
            return ui_focus_.has_focus(c->focus_filter());}())
        {
            c->render_toplevel(
                lx,
                ly,
                render_config,
                scene_graph_config,
                render_results,
                frame_id);
        }
    }
}

void RenderLogics::print(std::ostream& ostr, size_t depth) const {
    std::shared_lock lock{ mutex_ };
    ostr << std::string(depth, ' ') << "RenderLogics\n";
    for (const auto& c : render_logics_) {
        c.second->print(ostr, depth + 1);
        ostr << '\n';
    }
}

void RenderLogics::prepend(const DanglingBaseClassRef<RenderLogic>& render_logic, int z_order, SourceLocation loc) {
    insert(render_logic, true, z_order, loc);
}

void RenderLogics::append(const DanglingBaseClassRef<RenderLogic>& render_logic, int z_order, SourceLocation loc) {
    insert(render_logic, false, z_order, loc);
}

void RenderLogics::remove(const RenderLogic& render_logic) {
    std::scoped_lock lock{ mutex_ };
    auto it = find_render_logic(render_logic, render_logics_);
    if (it == render_logics_.end()) {
        verbose_abort("Could not find render logic to be removed");
    }
    render_logics_.erase(it);
}

void RenderLogics::insert(const DanglingBaseClassRef<RenderLogic>& render_logic, bool prepend, int z_order, SourceLocation loc) {
    std::scoped_lock lock{ mutex_ };
    ZorderAndId zi{
        .z = z_order,
        .id = prepend
            ? next_smallest_id_--
            : next_largest_id_++
    };
    auto it = render_logics_.try_emplace(zi, render_logic, CURRENT_SOURCE_LOCATION);
    if (!it.second) {
        verbose_abort("Could not insert render logic");
    }
    it.first->second.on_destroy(
        [this, k = it.first->first]() {
            std::scoped_lock lock{ mutex_ };
            if (render_logics_.erase(k) != 1) {
                verbose_abort("Could not erase render logic");
            }
        }, loc
    );
}
