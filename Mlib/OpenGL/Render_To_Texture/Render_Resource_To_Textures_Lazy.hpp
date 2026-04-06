#pragma once
#include <memory>
#include <string>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
struct OrthoCameraConfig;
enum class FrameBufferChannelKind;
template <typename TData, size_t... tshape>
class FixedArray;
enum class ColorExtrapolationMode;
template <class T>
class VariableAndHash;

void render_resource_to_textures_lazy(
    VariableAndHash<std::string> resource,
    VariableAndHash<std::string> color_texture_name,
    VariableAndHash<std::string> depth_texture_name,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const OrthoCameraConfig& ortho_camera_config,
    FrameBufferChannelKind depth_kind,
    const FixedArray<int, 2>& texture_size,
    int nsamples_msaa,
    float dpi,
    ColorExtrapolationMode color_extrapolation_mode);

}
