#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class CreatePlaneControllerKeyBinding: public LoadRenderableSceneInstanceFunction {
public:
    explicit CreatePlaneControllerKeyBinding(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
