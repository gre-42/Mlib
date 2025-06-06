#pragma once
#include <Mlib/Scene_Graph/Elements/Blended.hpp>

namespace Mlib {

struct Light;
struct Skidmark;
struct SceneGraphConfig;
struct RenderConfig;
class IDynamicLights;
struct RenderPass;

struct ListOfBlended {
    ListOfBlended();
    ~ListOfBlended();
    void render(
        IDynamicLights* dynamic_lights,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass);
    std::list<Blended> list;
};

struct ListsOfBlended {
    ListsOfBlended();
    ~ListsOfBlended();
    ListOfBlended early;
    ListOfBlended late;
};

}
