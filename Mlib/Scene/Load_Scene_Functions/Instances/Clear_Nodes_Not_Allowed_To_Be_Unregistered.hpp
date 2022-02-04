#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene/User_Function.hpp>
#include <regex>

namespace Mlib {

class ClearNodesNotAllowedToBeUnregistered: public LoadSceneInstanceFunction {
public:
    static LoadSceneUserFunction user_function;
private:
    explicit ClearNodesNotAllowedToBeUnregistered(RenderableScene& renderable_scene);
    void execute(const std::smatch& match, const LoadSceneUserFunctionArgs& args);
};

}
