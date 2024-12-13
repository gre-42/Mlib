#include "Dff_File_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff_Array.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TPos>
std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources,
    const DrawDistanceDb& dddb)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    auto trafo = FrameTransformation::ZERO_POSITION | FrameTransformation::IDENTITY_ROTATION;
    if constexpr (std::is_same_v<TPos, float>) {
        hr->acvas->scvas = load_dff(istr, name, cfg, dddb, trafo).renderables;
    } else if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
        hr->acvas->dcvas = load_dff(istr, name, cfg, dddb, trafo).renderables;
    } else {
        THROW_OR_ABORT("Unknown mesh precision");
    }
    return hr;
}

template <class TPos>
std::shared_ptr<ISceneNodeResource> Mlib::load_renderable_dff(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    const SceneNodeResources& scene_node_resources,
    const DrawDistanceDb& dddb)
{
    auto hr = std::make_shared<HeterogeneousResource>(scene_node_resources);
    auto trafo = FrameTransformation::ZERO_POSITION | FrameTransformation::IDENTITY_ROTATION;
    if constexpr (std::is_same_v<TPos, float>) {
        hr->acvas->scvas = load_dff(filename, cfg, dddb, trafo).renderables;
    } else if constexpr (std::is_same_v<TPos, CompressedScenePos>) {
        hr->acvas->dcvas = load_dff(filename, cfg, dddb, trafo).renderables;
    } else {
        THROW_OR_ABORT("Unknown mesh precision");
    }
    return hr;
}

namespace Mlib {

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<float>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources,
    const DrawDistanceDb& dddb);

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<CompressedScenePos>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<CompressedScenePos>& cfg,
    const SceneNodeResources& scene_node_resources,
    const DrawDistanceDb& dddb);

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<float>(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const SceneNodeResources& scene_node_resources,
    const DrawDistanceDb& dddb);

template std::shared_ptr<ISceneNodeResource> load_renderable_dff<CompressedScenePos>(
    const std::string& filename,
    const LoadMeshConfig<CompressedScenePos>& cfg,
    const SceneNodeResources& scene_node_resources,
    const DrawDistanceDb& dddb);

}
