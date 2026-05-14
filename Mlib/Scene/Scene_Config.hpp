#pragma once

namespace Mlib {

struct RenderConfig;
struct SceneGraphConfig;
struct PhysicsEngineConfig;

struct SceneConfig {
    #ifndef WITHOUT_GRAPHICS
    RenderConfig& render_config;
    #endif
    SceneGraphConfig& scene_graph_config;
    PhysicsEngineConfig& physics_engine_config;
};

}
