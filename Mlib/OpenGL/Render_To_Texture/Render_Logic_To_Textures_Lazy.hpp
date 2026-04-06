#pragma once
#include <memory>
#include <string>

namespace Mlib {

class RenderLogic;
class RenderingResources;
enum class FrameBufferChannelKind;
template <typename TData, size_t... tshape>
class FixedArray;
template <class T>
class VariableAndHash;
enum class ColorExtrapolationMode;

void render_logic_to_textures_lazy(
    const std::shared_ptr<RenderLogic>& child_logic,
    RenderingResources& rendering_resources,
    FrameBufferChannelKind depth_kind,
    const FixedArray<int, 2>& texture_size,
    int nsamples_msaa,
    float dpi,
    ColorExtrapolationMode color_extrapolation_mode,
    VariableAndHash<std::string> color_texture_name,
    VariableAndHash<std::string> depth_texture_name);

}
