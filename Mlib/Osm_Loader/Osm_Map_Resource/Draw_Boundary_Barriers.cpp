#include "Draw_Boundary_Barriers.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Barrier_Style.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static double curvature(const FixedArray<double, 3, 2>& points) {
    // From: https://en.wikipedia.org/wiki/Curvature#In_terms_of_a_general_parametrization
    auto d1 = 0.5 * (points[2] - points[0]);
    auto d2 = points[2] - 2. * points[1] + points[0];
    auto x1 = d1(0);
    auto x2 = d2(0);
    auto y1 = d1(1);
    auto y2 = d2(1);
    return (x1 * y2 - y1 * x2) / std::pow(squared(x1) + squared(y1), 3. / 2.);
}

// static double curvature_x(const FixedArray<FixedArray<double, 2>, 3>& points) {
//     auto v = points(2) - points(0);
//     FixedArray<double, 2> n{v(1), -v(0)};
//     double len2 = sum(squared(n));
//     double len = std::sqrt(len2);
//     n /= len;
//     auto diff = dot0d(n, 0.5 * (points(0) + points(2)) - points(1));
//     return -8. * diff / len2;
// }

static unsigned int curvature_to_gear(double k) {
    auto v_kph = 5 / std::sqrt(std::abs(k));
    static const std::vector<double> gear_to_velocity{
        5., 50., 70., 100., 150., 200.};
    auto it = std::upper_bound(gear_to_velocity.begin(), gear_to_velocity.end(), v_kph);
    return integral_cast<unsigned int>(it - gear_to_velocity.begin()) + 1;
}

void Mlib::draw_boundary_barriers(
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& inner_triangles,
    const Material& material,
    const Morphology& morphology,
    float scale,
    float uv_scale,
    float barrier_height,
    const BarrierStyle& barrier_style,
    ContourDetectionStrategy contour_detection_strategy)
{
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    auto contours = find_contours(inner_triangles, contour_detection_strategy);
    const auto& tl = tls.emplace_back(std::make_shared<TriangleList<CompressedScenePos>>(
        "boundary_barriers",
        material,
        morphology + PhysicsMaterial::ATTR_VISIBLE + PhysicsMaterial::ATTR_TWO_SIDED));
    // plot_mesh_svg(
    //     "/tmp/contours.svg",
    //     600,
    //     500,
    //     {},
    //     contours,
    //     {});

    tl->material.textures_color = { primary_rendering_resources.get_blend_map_texture(barrier_style.texture) };
    tl->material.blend_mode = barrier_style.blend_mode;
    tl->material.cull_faces = barrier_style.cull_faces;
    tl->material.reorient_uv0 = barrier_style.reorient_uv0;
    tl->material.shading = barrier_style.shading;
    tl->material.compute_color_mode();
    if (barrier_style.depth != 0.f) {
        THROW_OR_ABORT("Boundary barrier depth must be zero");
    }
    for (const auto& contour : contours) {
        if (contour.size() < 4) {
            continue;
        }
        auto inc_it = [&contour](auto& it){
            if (++it == contour.end()) {
                it = contour.begin();
            }
        };
        auto it3 = contour.begin();
        auto it0 = it3++;
        auto it1 = it3++;
        auto it2 = it3++;
        float length_mod1 = 0.f;
        while (it1 != contour.begin()) {
            FixedArray<CompressedScenePos, 4, 3> n{*it0, *it1, *it2, *it3};

            inc_it(it0);
            inc_it(it1);
            inc_it(it2);
            inc_it(it3);

            FixedArray<CompressedScenePos, 4, 2> n2{
                FixedArray<CompressedScenePos, 2>{n(0, 0), n(0, 1)},
                FixedArray<CompressedScenePos, 2>{n(1, 0), n(1, 1)},
                FixedArray<CompressedScenePos, 2>{n(2, 0), n(2, 1)},
                FixedArray<CompressedScenePos, 2>{n(3, 0), n(3, 1)}};

            FixedArray<CompressedScenePos, 3, 2> n2_0{n2[0], n2[1], n2[2]};
            FixedArray<CompressedScenePos, 3, 2> n2_1{n2[1], n2[2], n2[3]};
            double k = 0.5 * (
                curvature(n2_0.casted<double>() / fixed_full<double, 3, 2>(scale)) +
                curvature(n2_1.casted<double>() / fixed_full<double, 3, 2>(scale)));
            // Each way segment has (at least) two boundary edges.
            // The barrier is drawn on the side with positive curvature.
            // Note that the contours have the same {counter-}clockwise
            // orientation as the triangles they were computed from.
            if (k < 0) {
                continue;
            }
            unsigned int gear = curvature_to_gear(k);
            if (gear >= 3) {
                continue;
            }
            const auto& p0 = n[2];
            const auto& p1 = n[1];
            const auto& p0_2 = n2[2];
            const auto& p1_2 = n2[1];
            float width = (float)std::sqrt(sum(squared(p1_2 - p0_2)));
            FixedArray<float, 2> uv = 1.f / scale * uv_scale * barrier_style.uv;
            tl->draw_rectangle_wo_normals(
                {p1(0), p1(1), p1(2)},
                {p0(0), p0(1), p0(2)},
                {p0(0), p0(1), p1(2) + (CompressedScenePos)(barrier_height * scale)},
                {p1(0), p1(1), p1(2) + (CompressedScenePos)(barrier_height * scale)},
                Colors::WHITE,
                Colors::WHITE,
                Colors::WHITE,
                Colors::WHITE,
                FixedArray<float, 2>{length_mod1, 0.f} * uv,
                FixedArray<float, 2>{length_mod1 + width, 0.f} * uv,
                FixedArray<float, 2>{length_mod1 + width, barrier_height * scale} * uv,
                FixedArray<float, 2>{length_mod1, barrier_height * scale} * uv);
            length_mod1 = std::fmod(length_mod1 + width, 1.f / uv(0));
        }
    }
}
