#include "Pssg_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg_Arrays.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

template <class TPos>
void Mlib::load_renderable_pssg(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPos>& cfg,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables)
{
    if constexpr (std::is_same_v<TPos, float>) {
        auto pa = load_pssg_arrays<float, ScenePos>(istr, name, cfg);
        for (auto&& [n, a] : pa.resources) {
            auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
            hr->acvas->scvas = { a };
            scene_node_resources.add_resource(n, hr);
        }
        for (const auto& [i, ins] : enumerate(pa.instances)) {
            auto instance_name = ins.resource_name + std::to_string(i);
            scene_node_resources.add_instantiable(instance_name, ins);
            added_instantiables.push_back(instance_name);
        }
    } else if constexpr (std::is_same_v<TPos, double>) {
        auto pa = load_pssg_arrays<double, ScenePos>(istr, name, cfg);
        for (auto&& [n, a] : pa.resources) {
            auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
            hr->acvas->dcvas = { a };
            scene_node_resources.add_resource(n, hr);
        }
        for (const auto& [i, ins] : enumerate(pa.instances)) {
            auto instance_name = ins.resource_name + std::to_string(i);
            scene_node_resources.add_instantiable(instance_name, ins);
            added_instantiables.push_back(instance_name);
        }
    } else {
        THROW_OR_ABORT("Unknown mesh precision");
    }
}

template <class TPos>
void Mlib::load_renderable_pssg(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables)
{
    auto f = create_ifstream(filename, std::ios::binary);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + "\" for reading");
    }
    load_renderable_pssg(*f, filename, cfg, scene_node_resources, added_scene_node_resources, added_instantiables);
}

namespace Mlib {

template void load_renderable_pssg<float>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

template void load_renderable_pssg<double>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<double>& cfg,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

template void load_renderable_pssg<float>(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

template void load_renderable_pssg<double>(
    const std::string& filename,
    const LoadMeshConfig<double>& cfg,
    SceneNodeResources& scene_node_resources,
    std::list<std::string>& added_scene_node_resources,
    std::list<std::string>& added_instantiables);

}
