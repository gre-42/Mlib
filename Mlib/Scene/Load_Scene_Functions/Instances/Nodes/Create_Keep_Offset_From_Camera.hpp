#pragma once
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class CreateKeepOffsetFromCamera: public LoadRenderableSceneInstanceFunction {
public:
    explicit CreateKeepOffsetFromCamera(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
