#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <regex>

namespace Mlib {

class CreateGunKeyBinding: public LoadSceneInstanceFunction {
public:
    static UserFunction user_function;
private:
    explicit CreateGunKeyBinding(RenderableScene& renderable_scene);
    void execute(const std::smatch& match, const UserFunctionArgs& args);
};

}
