#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene/Load_Scene_User_Function.hpp>

namespace Mlib {

class JsonMacroArguments;

class CreateVisualPlayerBulletCount: public LoadSceneInstanceFunction {
public:
    static LoadSceneUserFunction user_function;
    static const std::string key;
private:
    explicit CreateVisualPlayerBulletCount(RenderableScene& renderable_scene);
    void execute(const JsonMacroArguments& json_macro_arguments, const LoadSceneUserFunctionArgs& args);
};

}
