#include "Load_Kn5_Array.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Interfaces/IRace_Logic.hpp>
#include <Mlib/Geometry/Mesh/Ambient_Occlusion_By_Curvature.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load_Kn5.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Io/Ini_Parser.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

using namespace Mlib;

static const FixedArray<float, 3, 3> M = {
    -1.f, 0.f, 0.f,
    0.f, 1.f, 0.f,
    0.f, 0.f, -1.f};

FixedArray<float, 3, 3> trafo(const FixedArray<float, 3, 3>& R) {
    return dot2d(dot2d(M.T(), R), M);
}

static FixedArray<float, 3, 3> ac_start_to_car(const FixedArray<float, 3, 3>& R)
{
    if (R(1u, 1u) > 0.f) {
        // Akagi
        return dot2d(R, tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, float(M_PI), 0.f}));
    } else {
        // Hondarribia, Irohazaka
        return trafo(dot2d(R, tait_bryan_angles_2_matrix(FixedArray<float, 3>{float(M_PI), 0.f, 0.f})));
    }
}

static TransformationMatrix<float, double, 3> ac_start_to_car(const TransformationMatrix<float, double, 3>& tm)
{
    return TransformationMatrix<float, double, 3>{ac_start_to_car(tm.R()), tm.t()};
}

static TransformationMatrix<float, double, 3> ac_center(
    const TransformationMatrix<float, double, 3>& left,
    const TransformationMatrix<float, double, 3>& right)
{
    return TransformationMatrix<float, double, 3>{
        left.R(),
        (left.t() + right.t()) / 2.};
}

enum class MetaAttributes {
    NONE = 0,
    VISIBLE = (1 << 0),
    COLLIDABLE = (1 << 1),
    GRASS = (1 << 2),
    ROAD = (1 << 3),
    TREE = (1 << 4)
};

MetaAttributes operator ~ (MetaAttributes a) {
    return (MetaAttributes)(~int(a));
}

MetaAttributes operator & (MetaAttributes a, MetaAttributes b) {
    return (MetaAttributes)(int(a) & int(b));
}

MetaAttributes& operator &= (MetaAttributes& a, MetaAttributes b) {
    (int&)a &= (int)b;
    return a;
}

MetaAttributes& operator |= (MetaAttributes& a, MetaAttributes b) {
    (int&)a |= (int)b;
    return a;
}

bool any(MetaAttributes attr) {
    return attr != MetaAttributes::NONE;
}

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> Mlib::load_kn5_array(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources,
    IRaceLogic* race_logic)
{
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;

    auto append_kn5 = [&](const std::string& filename) {
        auto kn5 = load_kn5(filename);
        if (dds_resources != nullptr) {
            for (auto& [name, content] : kn5.textures) {
                dds_resources->insert_texture(name, std::move(content), TextureAlreadyExistsBehavior::WARN);
            }
        }
        if (race_logic != nullptr) {
            std::map<std::string, const kn5Node*> nodes;
            for (const auto& [_, node] : kn5.nodes) {
                nodes[node.name] = &node;
            }
            {
                auto it = nodes.find("AC_PIT_0");
                if (it != nodes.end()) {
                    race_logic->set_start_pose(ac_start_to_car(it->second->hmatrix.casted<float, double>()), 0);
                }
            }
            {
                auto it_l = nodes.find("AC_AB_START_L");
                auto it_r = nodes.find("AC_AB_START_R");
                if ((it_l != nodes.end()) && (it_r != nodes.end())) {
                    race_logic->set_start_pose(
                        ac_center(
                            it_l->second->hmatrix.casted<float, double>(),
                            it_r->second->hmatrix.casted<float, double>()),
                        0);
                }
            }
            {
                auto it = nodes.find("AC_START_0");
                if (it != nodes.end()) {
                    race_logic->set_start_pose(ac_start_to_car(it->second->hmatrix.casted<float, double>()), 0);
                }
            }
            // AC_AB_START_L/R
            // AC_PIT_(\\d+)
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
            auto attrs = MetaAttributes::VISIBLE;
            static const DECLARE_REGEX(collide_reg, "^(\\d+)?(\\w+)");
            static const DECLARE_REGEX(grass_reg, "^(?:GR\\b|GRASS)");
            static const DECLARE_REGEX(road_reg, "^ROAD");
            static const DECLARE_REGEX(tree_reg, "^tree");
            Mlib::re::smatch match;
            if (Mlib::re::regex_search(node.name, match, collide_reg)) {
                if (match[1].matched) {
                    size_t id = safe_stoz(match[1].str());
                    if (id > 0) {
                        attrs |= MetaAttributes::COLLIDABLE;
                    }
                }
                if (match[2].str().starts_with("WALL_col")) {
                    attrs &= ~MetaAttributes::VISIBLE;
                }
                if (Mlib::re::regex_search(match[2].str(), grass_reg)) {
                    attrs |= MetaAttributes::GRASS;
                }
                if (Mlib::re::regex_search(match[2].str(), road_reg)) {
                    attrs |= MetaAttributes::ROAD;
                }
            }
            if (any(attrs & MetaAttributes::COLLIDABLE)) {
                tl.physics_material |= PhysicsMaterial::ATTR_COLLIDE;
                tl.physics_material |= PhysicsMaterial::ATTR_CONCAVE;
            }
            if (!node.isRenderable || !any(attrs & MetaAttributes::VISIBLE)) {
                tl.physics_material &= ~PhysicsMaterial::ATTR_VISIBLE;
            }
            if (node.materialID.has_value()) {
                const auto& material = kn5.materials.at(node.materialID.value());
                // From: http://www.toms-sim-side.de/tutorials/dokumente/AC_convert.pdf
                //       https://assettocorsamods.net/threads/setting-up-trees.162/
                if ((material.shader == "ksGrass") ||
                    (material.shader == "ksTree") ||
                    Mlib::re::regex_search(node.name, tree_reg))
                {
                    attrs |= MetaAttributes::TREE;
                }
                if (any(attrs & MetaAttributes::TREE)) {
                    tl.material.merge_textures = true;
                    tl.material.continuous_blending_z_order = 2;
                    tl.material.wrap_mode_s = WrapMode::CLAMP_TO_EDGE;
                    tl.material.wrap_mode_t = WrapMode::CLAMP_TO_EDGE;
                    tl.material.occluded_pass = ExternalRenderPassType::NONE;
                    tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    tl.material.blend_mode = cfg.blend_mode;
                } else {
                    if (material.ksAlphaRef != 0.f) {
                        tl.material.occluded_pass = ExternalRenderPassType::NONE;
                        tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    } else {
                        tl.material.occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
                        tl.material.occluder_pass = ExternalRenderPassType::NONE;
                    }
                    if (material.shader == "ksPerPixelAlpha") {
                        tl.material.blend_mode = cfg.blend_mode;
                    } else if ((material.shader == "ksPerPixel") ||  // required for Akina track
                               (material.shader == "ksPerPixelAT") ||
                               (material.shader == "ksPerPixelAT_NM") ||
                               (material.shader == "ksPerPixelMultiMap_AT_NMDetail"))
                    {
                        tl.material.blend_mode = BlendMode::BINARY_05;
                    } else if ((material.shader == "ksPerPixelNM") ||
                               (material.shader == "ksPerPixelMultiMap") ||
                               (material.shader == "ksPerPixelMultiMap_NMDetail") ||
                               (material.shader == "ksPerPixelReflection") ||
                               (material.shader == "ksMultilayer") ||
                               (material.shader == "ksMultilayer_fresnel_nm"))
                    {
                        tl.material.blend_mode = BlendMode::OFF;
                    } else {
                        THROW_OR_ABORT("Unknown shader: \"" + material.shader + '"');
                    }
                }
                tl.material.emissivity = OrderableFixedArray{fixed_full<float, 3>(material.ksEmissive)};
                tl.material.ambience = OrderableFixedArray{fixed_full<float, 3>(material.ksAmbient)};
                tl.material.diffusivity = OrderableFixedArray{fixed_full<float, 3>(material.ksDiffuse)};
                tl.material.specularity = OrderableFixedArray{fixed_full<float, 3>(material.ksSpecular)};
                tl.material.specular_exponent = material.ksSpecularEXP;
                tl.material.detail_multiplier = material.magicMult;
                if (!material.txDiffuse.empty() &&
                    !material.txMask.empty() &&
                    (material.detailUVMultiplier != 0.f) &&
                    ((material.shader == "ksMultilayer") ||
                     (material.shader == "ksMultilayer_fresnel_nm")))
                {
                    tl.material.textures = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = material.txDiffuse,
                            .normal = material.txNormal,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .role = BlendMapRole::DETAIL_BASE}};
                    for (uint32_t i = 0; i < 3; ++i) {
                        if (material.txDetail(i).empty() ||
                            (material.mult(i) == 0.f) ||
                            (material.detailUVMultiplier == 0.f))
                        {
                            continue;
                        }
                        tl.material.textures.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = material.txMask,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                            .scale = material.mult(i),
                            .role = BlendMapRole::DETAIL_MASK_R + i});
                        tl.material.textures.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = material.txDetail(i),
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                            .scale = material.detailUVMultiplier,
                            .role = BlendMapRole::DETAIL_COLOR});
                    }
                    tl.material.compute_color_mode();
                } else {
                    if (!material.txDiffuse.empty()) {
                        tl.material.textures = {BlendMapTexture{
                            .texture_descriptor = {
                                .color = material.txDiffuse,
                                .normal = material.txNormal,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS}}};
                        tl.material.compute_color_mode();
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
    if (filename.ends_with(".ini")) {
        for (const auto& [name, section] : IniParser{filename}) {
            if (name.starts_with("MODEL_")) {
                auto it = section.find("FILE");
                if (it == section.end()) {
                    THROW_OR_ABORT("Could not find FILE variable in section of ini file: \"" + filename + '"');
                }
                append_kn5((std::filesystem::path{filename}.parent_path() / it->second).string());
            }
        }
    } else {
        append_kn5(filename);
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
template std::list<std::shared_ptr<ColoredVertexArray<float>>> load_kn5_array<float>(const std::string& file_or_directory, const LoadMeshConfig<float>&, IDdsResources*, IRaceLogic*);
template std::list<std::shared_ptr<ColoredVertexArray<double>>> load_kn5_array<double>(const std::string& file_or_directory, const LoadMeshConfig<double>&, IDdsResources*, IRaceLogic*);
}
