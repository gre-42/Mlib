#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene/User_Function.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

class DeleteRootNode: public LoadSceneInstanceFunction {
public:
    static LoadSceneUserFunction user_function;
private:
    explicit DeleteRootNode(RenderableScene& renderable_scene);
    void execute(const Mlib::re::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
