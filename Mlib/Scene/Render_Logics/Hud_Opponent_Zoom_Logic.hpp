#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unordered_Set.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>
#include <mutex>

namespace Mlib {

class Player;
class Players;
class ObjectPool;
class IWidget;
class Scene;
class RenderLogics;

class HudOpponentZoomLogic: public RenderLogic {
public:
    HudOpponentZoomLogic(
        ObjectPool& object_pool,
        Scene& scene,
        std::unique_ptr<RenderLogic>&& scene_logic,
        RenderLogics& render_logics,
        Players& players,
        const DanglingBaseClassRef<Player>& player,
        const std::optional<std::vector<DanglingBaseClassPtr<const SceneNode>>>& exclusive_nodes,
        std::unique_ptr<IWidget>&& widget,
        float fov,
        float zoom);
    ~HudOpponentZoomLogic();

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    Scene& scene_;
    Players& players_;
    DanglingBaseClassRef<Player> player_;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
    std::unique_ptr<RenderLogic> scene_logic_;
    std::optional<DanglingUnorderedSet<const SceneNode>> exclusive_nodes_;
    std::unique_ptr<IWidget> widget_;
    float fov_;
    float scaled_fov_;
};

}
