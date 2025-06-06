#include "List_Of_Blended.hpp"
#include <Mlib/Scene_Graph/Elements/Dynamic_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Lights.hpp>
#include <Mlib/Scene_Graph/Render_Pass_Extended.hpp>

using namespace Mlib;

ListOfBlended::ListOfBlended() = default;
ListOfBlended::~ListOfBlended() = default;

void ListOfBlended::render(
    IDynamicLights* dynamic_lights,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass)
{
    list.sort([](Blended& a, Blended& b){ return a.sorting_key() > b.sorting_key(); });
    for (const auto& b : list) {
        DynamicStyle dynamic_style{ dynamic_lights != nullptr
            ? dynamic_lights->get_color(b.m.t)
            : fixed_zeros<float, 3>() };
        b.renderable().render(
            b.mvp,
            b.m,
            iv,
            &dynamic_style,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            render_pass,
            b.animation_state.get(),
            b.color_style);
    }
}

ListsOfBlended::ListsOfBlended() = default;
ListsOfBlended::~ListsOfBlended() = default;
