#pragma once

namespace Mlib {

class SceneNodeResources;
class Scene;
class PhysicsEngine;
class SelectedCameras;

void create_scene_rod(
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras);

}
