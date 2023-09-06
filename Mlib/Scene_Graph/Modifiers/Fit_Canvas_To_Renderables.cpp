#include "Fit_Canvas_To_Renderables.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

void Mlib::fit_canvas_to_renderables(
    Scene& scene,
    const TransformationMatrix<float, double, 3>& v,
    OrthoCamera& camera,
    ExternalRenderPassType render_pass)
{
    AxisAlignedBoundingBox<float, 3> aabb;
    scene.visit([&](
        const TransformationMatrix<float, double, 3>& m,
        const std::map<std::string, std::shared_ptr<const Renderable>>& renderables)
    {
        auto mv = v * m;
        for (const auto& [n, r] : renderables) {
            try {
                r->extend_aabb(mv, render_pass, aabb);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Could not extend static light AABB for renderable \"" + n + "\": " + e.what());
            }
        }
    });
    if (any(aabb.min() >= aabb.max())) {
        THROW_OR_ABORT("Scene AABB not positive");
    }
    camera.set_left_plane(aabb.min()(0));
    camera.set_right_plane(aabb.max()(0));
    camera.set_bottom_plane(aabb.min()(1));
    camera.set_top_plane(aabb.max()(1));
    camera.set_near_plane(-aabb.max()(2));
    camera.set_far_plane(-aabb.min()(2));
}
