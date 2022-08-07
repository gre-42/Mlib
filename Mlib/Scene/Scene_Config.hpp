#pragma once

namespace Mlib {

struct RenderConfig;
struct CameraConfig;
struct SceneGraphConfig;
struct PhysicsEngineConfig;

struct SceneConfig {
    RenderConfig& render_config;
    CameraConfig& camera_config;
    SceneGraphConfig& scene_graph_config;
    PhysicsEngineConfig& physics_engine_config;
};

}
