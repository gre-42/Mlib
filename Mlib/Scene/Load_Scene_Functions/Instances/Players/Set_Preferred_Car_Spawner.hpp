#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

class SetPreferredCarSpawner: public LoadSceneInstanceFunction {
public:
    explicit SetPreferredCarSpawner(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
