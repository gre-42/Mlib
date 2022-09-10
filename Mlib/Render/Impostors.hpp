#pragma once
#include <Mlib/Scene_Graph/Interfaces/IImpostors.hpp>

namespace Mlib {

class RenderLogics;
class RenderLogic;
class Scene;
class SelectedCameras;

class Impostors: public IImpostors {
public:
    explicit Impostors(
        RenderLogics& render_logics,
        RenderLogic& child_logic,
        Scene& scene,
        SelectedCameras& cameras);
    virtual void create_impostor(SceneNode& scene_node) override;
private:
    RenderLogics& render_logics_;
    RenderLogic& child_logic_;
    Scene& scene_;
    SelectedCameras& cameras_;
};

}
