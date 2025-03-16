#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class RootNodeInstance: public LoadSceneInstanceFunction {
public:
    explicit RootNodeInstance(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
