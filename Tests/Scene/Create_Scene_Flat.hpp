#pragma once

namespace Mlib {

class SceneNodeResources;
class Scene;
class PhysicsEngine;
class SelectedCameras;
struct PhysicsEngineConfig;

void create_scene_flat(
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras,
    const PhysicsEngineConfig& physics_cfg,
    unsigned int seed);

}
