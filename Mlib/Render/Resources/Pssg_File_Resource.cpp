#include "Pssg_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg_Arrays.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

template <class TResourcePos>
void Mlib::load_renderable_pssg(
    const PssgArrays<TResourcePos, ScenePos>& arrays,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables)
{
    for (auto&& [n, a] : arrays.resources) {
        auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
        hr->acvas->cvas<TResourcePos>() = { a };
        scene_node_resources.add_resource(n, hr);
    }
    for (const auto& [i, ins] : enumerate(arrays.instances)) {
        auto instance_name = ins.resource_name + std::to_string(i);
        scene_node_resources.add_instantiable(instance_name, ins);
        added_instantiables.push_back(instance_name);
    }
}

namespace Mlib {

template void load_renderable_pssg<float>(
    const PssgArrays<float, ScenePos>& arrays,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

template void load_renderable_pssg<CompressedScenePos>(
    const PssgArrays<CompressedScenePos, ScenePos>& arrays,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

}
