#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

class CreateVideoModeSelectorLogic: public LoadRenderableSceneInstanceFunction {
public:
    explicit CreateVideoModeSelectorLogic(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
