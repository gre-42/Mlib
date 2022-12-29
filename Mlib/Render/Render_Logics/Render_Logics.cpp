#include "Render_Logics.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

static std::map<ZorderAndId, SceneNodeAndRenderLogic>::iterator
    find_render_logic(const SceneNode* node, std::map<ZorderAndId, SceneNodeAndRenderLogic>& lst) {
    return std::find_if(
        lst.begin(),
        lst.end(),
        [node](const auto& v){ return v.second.node == node; });
}

static std::map<ZorderAndId, SceneNodeAndRenderLogic>::iterator
    find_render_logic(const RenderLogic* render_logic, std::map<ZorderAndId, SceneNodeAndRenderLogic>& lst) {
    return std::find_if(
        lst.begin(),
        lst.end(),
        [render_logic](const auto& v){ return v.second.render_logic.get() == render_logic; });
}

RenderLogics::RenderLogics(DeleteNodeMutex& delete_node_mutex, UiFocus& ui_focus)
: delete_node_mutex_{delete_node_mutex},
  ui_focus_{ui_focus},
  next_smallest_id_{0},
  next_largest_id_{1}
{}

RenderLogics::~RenderLogics() {
    std::unique_lock lock{mutex_};
    std::set<SceneNode*> visited_nodes;
    for (const auto& n : render_logics_) {
        if ((n.second.node != nullptr) && !visited_nodes.contains(n.second.node)) {
            visited_nodes.insert(n.second.node);
            n.second.node->destruction_observers.remove(this);
        }
    }
}

void RenderLogics::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::lock_guard dlock{delete_node_mutex_};
    std::shared_lock rlock{mutex_};

    LOG_FUNCTION("RenderLogics::render");
    for (const auto& c : render_logics_) {
        bool has_focus;
        {
            std::shared_lock lock{ui_focus_.focuses.mutex};
            has_focus = ui_focus_.has_focus(c.second.render_logic->focus_filter());
        }
        if (has_focus) {
            c.second.render_logic->render(width, height, render_config, scene_graph_config, render_results, frame_id);
        }
    }
}

void RenderLogics::print(std::ostream& ostr, size_t depth) const {
    std::shared_lock lock{mutex_};
    ostr << std::string(depth, ' ') << "RenderLogics\n";
    for (const auto& c : render_logics_) {
        c.second.render_logic->print(ostr, depth + 1);
    }
}

void RenderLogics::prepend(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic) {
    std::unique_lock lock{mutex_};
    insert_unsafe(scene_node, render_logic, true);
}

void RenderLogics::append(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic) {
    std::unique_lock lock{mutex_};
    insert_unsafe(scene_node, render_logic, false);
}

void RenderLogics::remove(const RenderLogic& render_logic) {
    std::lock_guard lock{ delete_node_mutex_ };
    auto it = find_render_logic(&render_logic, render_logics_);
    if (it == render_logics_.end()) {
        THROW_OR_ABORT("Could not find render logic to be removed");
    }
    render_logics_.erase(it);
}

void RenderLogics::insert_unsafe(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic, bool prepend) {
    if (scene_node != nullptr &&
        (find_render_logic(scene_node, render_logics_) == render_logics_.end()))
    {
        scene_node->destruction_observers.add(this);
    }
    ZorderAndId zi{
        .z = RenderingContextStack::z_order(),
        .id = prepend
            ? next_smallest_id_--
            : next_largest_id_++
    };
    if (!render_logics_.insert(std::make_pair(zi, SceneNodeAndRenderLogic{scene_node, render_logic})).second) {
        THROW_OR_ABORT("Could not insert render logic");
    }
}

void RenderLogics::notify_destroyed(Object* destroyed_object) {
    std::lock_guard lock{ delete_node_mutex_ };
    size_t nfound = 0;
    while(true) {
        auto del = [destroyed_object](std::map<ZorderAndId, SceneNodeAndRenderLogic>& lst) {
            auto it = find_render_logic((SceneNode*)destroyed_object, lst);
            if (it == lst.end()) {
                return false;
            }
            lst.erase(it);
            return true;
        };
        if (!del(render_logics_)) {
            break;
        }
        ++nfound;
    }
    if (nfound == 0) {
        THROW_OR_ABORT("Could not find render logic to be deleted");
    }
}
