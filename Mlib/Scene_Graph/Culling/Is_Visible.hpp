#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <cstddef>
#include <cstdint>
#include <string>

namespace Mlib {

template <class TData, size_t tndim>
class ExtremalAxisAlignedBoundingBox;
template <class TData>
class Frustum3;
struct Material;
struct Morphology;
struct SceneGraphConfig;
enum class ExternalRenderPassType;
template <class TData>
class VisibilityCheck;

template <class TData>
bool is_visible(
    const VisibilityCheck<TData>& vc,
    const std::string& object_name,
    const Material& material,
    const Morphology& morphology,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<TData>* frustum,
    const ExtremalAxisAlignedBoundingBox<TData, 3>* aabb);

}
