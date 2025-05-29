#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Scene_Graph/Interfaces/IImposters.hpp>
#include <unordered_map>

namespace Mlib {

class IRenderableScene;
class RenderingResources;
class RenderLogics;
class RenderLogic;
class Scene;
class SelectedCameras;
class StandardRenderLogic;
class PostProcessingLogic;

struct ImposterInfoAndDestructionToken {
    template <class... Args>
    ImposterInfoAndDestructionToken(ImposterInfo info, Args&&... args)
        : info{ std::move(info) }
        , on_node_clear{ std::forward<Args>(args)... }
    {}
    ImposterInfo info;
    DestructionFunctionsRemovalTokens on_node_clear;
};

class DeferredInstantiator: public IImposters {
public:
    DeferredInstantiator();
    ~DeferredInstantiator();
    virtual void set_imposter_info(
        const DanglingRef<SceneNode>& scene_node,
        const ImposterInfo& info) override;
    void create_imposters(
        IRenderableScene* renderable_scene,
        RenderingResources& rendering_resources,
        RenderLogics& render_logics,
        RenderLogic& child_logic,
        Scene& scene,
        SelectedCameras& cameras) const;
    void set_background_color(
        const FixedArray<float, 3>& background_color);
    void apply_background_color(
        StandardRenderLogic& standard_render_logic,
        PostProcessingLogic& post_processing_logic);
private:
    std::unordered_map<DanglingPtr<SceneNode>, ImposterInfoAndDestructionToken> infos_;
    std::optional<FixedArray<float, 3>> background_color_;
    bool imposters_created_;
};

}
