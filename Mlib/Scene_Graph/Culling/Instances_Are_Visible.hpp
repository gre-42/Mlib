#pragma once

namespace Mlib {

struct Material;
enum class ExternalRenderPassType;

bool instances_are_visible(
    const Material& m,
    ExternalRenderPassType external_render_pass);

}
