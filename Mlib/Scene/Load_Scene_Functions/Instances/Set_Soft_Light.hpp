#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene/User_Function.hpp>
#include <regex>

namespace Mlib {

class SetSoftLight: public LoadSceneInstanceFunction {
public:
    static LoadSceneUserFunction user_function;
private:
    explicit SetSoftLight(RenderableScene& renderable_scene);
    void execute(const std::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
