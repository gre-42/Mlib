#pragma once

namespace Mlib {

struct RenderConfig;
struct SceneGraphConfig;
struct PhysicsEngineConfig;

struct SceneConfig {
    RenderConfig& render_config;
    SceneGraphConfig& scene_graph_config;
    PhysicsEngineConfig& physics_engine_config;
};

}
