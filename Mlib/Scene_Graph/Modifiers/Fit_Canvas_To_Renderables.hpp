#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

class Scene;
class OrthoCamera;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class ExternalRenderPassType;

void fit_canvas_to_renderables(
    Scene& scene,
    const TransformationMatrix<float, ScenePos, 3>& v,
    OrthoCamera& camera,
    ExternalRenderPassType render_pass);

}
