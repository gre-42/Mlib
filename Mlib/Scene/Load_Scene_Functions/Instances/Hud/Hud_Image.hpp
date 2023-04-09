#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene/User_Function.hpp>

namespace Mlib {

class JsonMacroArguments;

class HudImage: public LoadSceneInstanceFunction {
public:
    static LoadSceneUserFunction user_function;
    static const std::string key;
private:
    explicit HudImage(RenderableScene& renderable_scene);
    void execute(const JsonMacroArguments& json_macro_arguments, const LoadSceneUserFunctionArgs& args);
};

}
