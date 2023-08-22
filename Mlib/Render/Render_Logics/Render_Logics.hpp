#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <compare>
#include <map>
#include <memory>

namespace Mlib {

class SceneNode;
struct UiFocus;

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
    explicit RenderLogics(UiFocus& ui_focus);
    ~RenderLogics();
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    virtual void notify_destroyed(const Object& destroyed_object) override;

    void prepend(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic);
    void append(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic);
    void remove(const RenderLogic& render_logic);
private:
    void insert(SceneNode* scene_node, const std::shared_ptr<RenderLogic>& render_logic, bool prepend);
    std::map<ZorderAndId, SceneNodeAndRenderLogic> render_logics_;
    UiFocus& ui_focus_;
    int next_smallest_id_;
    int next_largest_id_;
    mutable SafeSharedMutex mutex_;
};

}
