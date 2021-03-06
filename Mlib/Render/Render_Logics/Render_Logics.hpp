#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <map>
#include <memory>
#include <mutex>

namespace Mlib {

class SceneNode;
struct UiFocus;
class DeleteNodeMutex;

struct ZorderAndId {
    int z;
    int id;
    std::strong_ordering operator <=> (const ZorderAndId&) const = default;
};

struct SceneNodeAndRenderLogic {
    SceneNode* node;
    std::shared_ptr<RenderLogic> render_logic;
};

class RenderLogics: public RenderLogic, public DestructionObserver {
public:
    RenderLogics(DeleteNodeMutex& delete_node_mutex, UiFocus& ui_focus);
    ~RenderLogics();
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    virtual void notify_destroyed(void* destroyed_object) override;

    void prepend(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic);
    void append(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic);
    void remove(const RenderLogic& render_logic);
private:
    void insert(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic, bool prepend);
    std::map<ZorderAndId, SceneNodeAndRenderLogic> render_logics_;
    DeleteNodeMutex& delete_node_mutex_;
    UiFocus& ui_focus_;
    int next_smallest_id_;
    int next_largest_id_;
};

}
