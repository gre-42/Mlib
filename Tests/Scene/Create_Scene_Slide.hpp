#pragma once

namespace Mlib {

class Scene;
class PhysicsEngine;
class SelectedCameras;

void create_scene_slide(
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras);

}
