#pragma once
#include <optional>
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
class SceneNodeResources;
struct ColoredVertexArrayFilter;
enum class BlendMode;
enum class InterpolationMode;
enum class ExternalRenderPassType;

void modify_rendering_material(
    const VariableAndHash<std::string>& resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& filter,
    std::optional<BlendMode> blend_mode,
    std::optional<ExternalRenderPassType> occluded_pass,
    std::optional<ExternalRenderPassType> occluder_pass,
    std::optional<InterpolationMode> magnifying_interpolation_mode,
    std::optional<std::string> histogram);

}
