#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

class JsonMacroArguments;

class CreateCarControllerIdleBinding: public LoadRenderableSceneInstanceFunction {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
private:
    explicit CreateCarControllerIdleBinding(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
