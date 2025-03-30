#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class InstantiateStatics: public LoadSceneInstanceFunction {
public:
    explicit InstantiateStatics(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
