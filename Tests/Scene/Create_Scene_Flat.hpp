#pragma once

namespace Mlib {

class Scene;
class PhysicsEngine;
class SelectedCameras;
struct PhysicsEngineConfig;

void create_scene_flat(
    Scene& scene,
    PhysicsEngine& pe,
    SelectedCameras& selected_cameras,
    const PhysicsEngineConfig& physics_cfg,
    unsigned int seed);

}
