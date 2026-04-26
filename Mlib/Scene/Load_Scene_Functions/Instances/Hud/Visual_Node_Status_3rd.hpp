#pragma once
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class VisualNodeStatus3rd: public LoadRenderableSceneInstanceFunction {
public:
    explicit VisualNodeStatus3rd(RenderableScene& render_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
