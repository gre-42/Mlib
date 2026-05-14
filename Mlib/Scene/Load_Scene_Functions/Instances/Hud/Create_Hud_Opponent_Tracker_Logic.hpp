#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene/Load_Renderable_Scene_Instance_Function.hpp>

namespace Mlib {

struct LoadSceneJsonUserFunctionArgs;

#ifdef WITHOUT_GRAPHICS
class CreatePhyicsOpponentTracker: public LoadPhysicsSceneInstanceFunction {
public:
    explicit CreatePhyicsOpponentTracker(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};
#else
class CreateHudOpponentTracker: public LoadRenderableSceneInstanceFunction {
public:
    explicit CreateHudOpponentTracker(RenderableScene& renderable_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};
#endif

}
