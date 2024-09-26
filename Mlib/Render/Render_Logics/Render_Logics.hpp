#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <compare>
#include <map>

namespace Mlib {

struct UiFocus;
template <class T>
class DestructionFunctionsTokensObject;

struct ZorderAndId {
    int z;
    int id;
    std::strong_ordering operator <=> (const ZorderAndId&) const = default;
};

class RenderLogics: public RenderLogic {
public:
    explicit RenderLogics(UiFocus& ui_focus);
    ~RenderLogics();
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
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void prepend(const DanglingBaseClassRef<RenderLogic>& render_logic, int z_order, SourceLocation loc);
    void append(const DanglingBaseClassRef<RenderLogic>& render_logic, int z_order, SourceLocation loc);
    void remove(const RenderLogic& render_logic);

private:
    void insert(const DanglingBaseClassRef<RenderLogic>& render_logic, bool prepend, int z_order, SourceLocation loc);
    std::map<ZorderAndId, DestructionFunctionsTokensObject<RenderLogic>> render_logics_;
    UiFocus& ui_focus_;
    int next_smallest_id_;
    int next_largest_id_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
