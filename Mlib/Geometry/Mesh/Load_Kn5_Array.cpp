#include "Load_Kn5_Array.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Mesh/Ambient_Occlusion_By_Curvature.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/IDds_Resources.hpp>
#include <Mlib/Geometry/Mesh/Load_Kn5.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

using namespace Mlib;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> Mlib::load_kn5_array(
    const std::string& file_or_directory,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources)
{
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;

    auto append_kn5 = [&](const std::string& filename) {
        auto kn5 = load_kn5(filename);
        if (dds_resources != nullptr) {
            for (auto& [name, content] : kn5.textures) {
                dds_resources->insert_texture(name, std::move(content), TextureAlreadyExistsBehavior::IGNORE);
            }
        }
        for (const auto& [_, node] : kn5.nodes) {
            TriangleList<TPos> tl{
                node.name,
                Material{
                    .reflection_map = cfg.reflection_map,
                    .occluded_pass = cfg.occluded_pass,
                    .occluder_pass = cfg.occluder_pass,
                    .alpha_distances = OrderableFixedArray{cfg.alpha_distances},
                    .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                    .aggregate_mode = cfg.aggregate_mode,
                    .transformation_mode = cfg.transformation_mode,
                    .center_distances = cfg.center_distances,
                    .max_triangle_distance = cfg.max_triangle_distance,
                    .cull_faces = cfg.cull_faces_default},
                cfg.physics_material};
            static const DECLARE_REGEX(collide_reg, "^(\\d+)");
            Mlib::re::smatch match;
            if (Mlib::re::regex_search(node.name, match, collide_reg)) {
                tl.physics_material_ |= PhysicsMaterial::ATTR_COLLIDE;
                tl.physics_material_ |= PhysicsMaterial::ATTR_CONCAVE;
            }
            if (!node.isRenderable) {
                tl.physics_material_ &= ~PhysicsMaterial::ATTR_VISIBLE;
            }
            if (dds_resources != nullptr) {
                if (node.materialID.has_value()) {
                    const auto& material = kn5.materials.at(node.materialID.value());
                    // From: http://www.toms-sim-side.de/tutorials/dokumente/AC_convert.pdf
                    //       https://assettocorsamods.net/threads/setting-up-trees.162/
                    if ((material.shader == "ksGrass") ||
                        (material.shader == "ksTree"))
                    {
                        tl.material_.merge_textures = true;
                        tl.material_.continuous_blending_z_order = 2;
                    }
                    if ((material.shader == "ksGrass") ||
                        (material.shader == "ksTree") ||
                        (material.shader == "ksPerPixelAT") ||
                        (material.shader == "ksPerPixelAT") ||
                        (material.shader == "ksPerPixelAT_NM") ||
                        (material.shader == "ksPerPixelAlpha") ||
                        (material.shader == "ksPerPixelMultiMap_AT_NMDetail"))
                    {
                        tl.material_.blend_mode = cfg.blend_mode;
                    } else if ((material.shader == "ksPerPixel") ||
                               (material.shader == "ksPerPixelNM") ||
                               (material.shader == "ksPerPixelMultiMap") ||
                               (material.shader == "ksMultilayer") ||
                               (material.shader == "ksMultilayer_fresnel_nm"))
                    {
                        tl.material_.blend_mode = BlendMode::OFF;
                    } else {
                        THROW_OR_ABORT("Unknown shader: \"" + material.shader + '"');
                    }
                    tl.material_.emissivity = OrderableFixedArray{fixed_full<float, 3>(material.ksEmissive)};
                    tl.material_.ambience = OrderableFixedArray{fixed_full<float, 3>(material.ksAmbient)};
                    tl.material_.diffusivity = OrderableFixedArray{fixed_full<float, 3>(material.ksDiffuse)};
                    tl.material_.specularity = OrderableFixedArray{fixed_full<float, 3>(material.ksSpecular)};
                    tl.material_.specular_exponent = material.ksSpecularEXP;
                    if (!material.txDiffuse.empty()) {
                        tl.material_.textures = {BlendMapTexture{
                            .texture_descriptor = {
                                .color = material.txDiffuse,
                                .normal = material.txNormal,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS}}};
                        tl.material_.compute_color_mode();
                    }
                }
            }
            for (const auto& tri : node.triangles) {
                tl.draw_triangle_with_normals(
                    node.position.at(tri(0)) TEMPLATEV casted<TPos>(),
                    node.position.at(tri(1)) TEMPLATEV casted<TPos>(),
                    node.position.at(tri(2)) TEMPLATEV casted<TPos>(),
                    node.normal.at(tri(0)),
                    node.normal.at(tri(1)),
                    node.normal.at(tri(2)),
                    {1.f, 1.f, 1.f},                            // c00
                    {1.f, 1.f, 1.f},                            // c10
                    {1.f, 1.f, 1.f},                            // c01
                    node.uv.at(tri(0)),
                    node.uv.at(tri(1)),
                    node.uv.at(tri(2)),
                    {},                                         // b00
                    {},                                         // b10
                    {},                                         // b01
                    cfg.triangle_tangent_error_behavior);
            }
            result.push_back(tl.triangle_array());
        }
    };
    if (std::filesystem::is_directory(file_or_directory)) {
        for (const auto& f : list_dir(file_or_directory)) {
            if (!f.path().string().ends_with(".kn5")) {
                continue;
            }
            append_kn5(f.path().string());
        }
    } else {
        append_kn5(file_or_directory);
    }
    FixedArray<float, 3, 3> rotation_matrix_p{tait_bryan_angles_2_matrix(cfg.rotation)};
    auto rotation_matrix_n = inv(rotation_matrix_p).value().T();
    for (auto& l : result) {
        for (auto& t : l->triangles) {
            for (auto& v : t.flat_iterable()) {
                v.position *= cfg.scale TEMPLATEV casted<TPos>();
                v.position = dot1d(rotation_matrix_p TEMPLATEV casted<TPos>(), v.position);
                v.position += cfg.position;
                v.normal = dot1d(rotation_matrix_n, v.normal);
                v.tangent = dot1d(rotation_matrix_p, v.tangent);
            }
        }
    }
    ambient_occlusion_by_curvature(result, cfg.laplace_ao_strength);
    return result;
}

namespace Mlib {
template std::list<std::shared_ptr<ColoredVertexArray<float>>> load_kn5_array<float>(const std::string& file_or_directory, const LoadMeshConfig<float>&, IDdsResources*);
template std::list<std::shared_ptr<ColoredVertexArray<double>>> load_kn5_array<double>(const std::string& file_or_directory, const LoadMeshConfig<double>&, IDdsResources*);
}
