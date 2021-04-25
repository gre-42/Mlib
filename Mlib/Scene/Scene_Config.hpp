#pragma once
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Scene_Graph/Camera_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

namespace Mlib {

struct SceneConfig {
    RenderConfig& render_config;
    CameraConfig& camera_config;
    SceneGraphConfig& scene_graph_config;
    PhysicsEngineConfig& physics_engine_config;
};

}
