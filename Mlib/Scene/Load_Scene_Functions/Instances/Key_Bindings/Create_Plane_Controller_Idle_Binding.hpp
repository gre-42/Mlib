#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonMacroArguments;

struct LoadSceneJsonUserFunctionArgs;

class CreatePlaneControllerIdleBinding: public LoadRenderableSceneInstanceFunction {
public:
    explicit CreatePlaneControllerIdleBinding(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
