#pragma once
#include <cstdint>

namespace Mlib {

struct Material;
enum class ExternalRenderPassType: uint32_t;

bool instances_are_visible(
    const Material& m,
    ExternalRenderPassType external_render_pass);

}
