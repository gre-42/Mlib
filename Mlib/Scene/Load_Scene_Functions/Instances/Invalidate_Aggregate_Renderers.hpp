#pragma once
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class InvalidateAggregateRenderers: public LoadRenderableSceneInstanceFunction {
public:
    explicit InvalidateAggregateRenderers(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
