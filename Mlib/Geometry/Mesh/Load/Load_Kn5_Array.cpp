#include "Load_Kn5_Array.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Interfaces/IRace_Logic.hpp>
#include <Mlib/Geometry/Mesh/Ambient_Occlusion_By_Curvature.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Kn5.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Io/Ini_Parser.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

using namespace Mlib;

static const FixedArray<double, 3> SPAWN_OFFSET = {0., 2., 0.};

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
        return dot2d(R, tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, 180.f * degrees, 0.f}));
    } else {
        // Hondarribia, Irohazaka
        return trafo(dot2d(R, tait_bryan_angles_2_matrix(FixedArray<float, 3>{180.f * degrees, 0.f, 0.f})));
    }
}

static TransformationMatrix<float, double, 3> ac_start_to_car(const TransformationMatrix<float, double, 3>& tm)
{
    return TransformationMatrix<float, double, 3>{ac_start_to_car(tm.R()), tm.t() + SPAWN_OFFSET};
}
 
// static TransformationMatrix<float, double, 3> ac_center(
//     const TransformationMatrix<float, double, 3>& left,
//     const TransformationMatrix<float, double, 3>& right)
// {
//     // Semetin
//     auto d = dot0d((right.t() - left.t()).casted<float>(), left.R().column(0));
//     return TransformationMatrix<float, double, 3>{
//         d > 0.f
//             ? left.R()
//             : dot2d(left.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, 180.f * degrees)),
//         (left.t() + right.t()) / 2.};
// }

static TransformationMatrix<float, double, 3> ac_portal(
    const TransformationMatrix<float, double, 3>& left,
    const TransformationMatrix<float, double, 3>& right)
{
    auto x = (right.t() - left.t()).casted<float>();
    auto y = FixedArray<float, 3>{0.f, 1.f, 0.f};
    x -= dot0d(x, y) * y;
    x /= std::sqrt(sum(squared(x)));
    auto z = cross(x, y);
    return TransformationMatrix<float, double, 3>{
        FixedArray<float, 3, 3>{
            x(0), y(0), z(0),
            x(1), y(1), z(1),
            x(2), y(2), z(2)},
        (left.t() + right.t()) / 2.};
}

static TransformationMatrix<float, double, 3> ac_start_to_car(
    const TransformationMatrix<float, double, 3>& left,
    const TransformationMatrix<float, double, 3>& right)
{
    return ac_portal(left, right);
}

static TransformationMatrix<float, double, 3> ac_waypoint(
    const TransformationMatrix<float, double, 3>& left,
    const TransformationMatrix<float, double, 3>& right)
{
    return ac_portal(left, right);
}

enum class MetaAttributes {
    NONE = 0,
    ATTR_VISIBLE = (1 << 0),
    ATTR_COLLIDABLE = (1 << 1),
    SURFACE_GRASS = (1 << 2),
    SURFACE_ROAD = (1 << 3),
    SURFACE_GRAVEL = (1 << 4),
    SURFACE_SIDE = (1 << 5),
    OBJ_GRASS = (1 << 6),
    OBJ_TREE = (1 << 7),
    ATTR_VERTICAL = (1 << 8)
};

MetaAttributes operator ~ (MetaAttributes a) {
    return (MetaAttributes)(~int(a));
}

MetaAttributes operator & (MetaAttributes a, MetaAttributes b) {
    return (MetaAttributes)(int(a) & int(b));
}

MetaAttributes operator | (MetaAttributes a, MetaAttributes b) {
    return (MetaAttributes)(int(a) | int(b));
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
    std::set<std::string> grass_materials;
    std::set<std::string> occluding_meshes;
    std::vector<unsigned int> texture_grid;
    float lit_mult = 1.f;
    float specular_mult = 1.f;
    bool raceway_is_circular = true;

    auto append_kn5 = [&](const std::string& kn5_filename) {
        auto kn5 = load_kn5(kn5_filename);
        if (dds_resources != nullptr) {
            for (auto& [name, content] : kn5.textures) {
                dds_resources->insert_texture(name, std::move(content.data), TextureAlreadyExistsBehavior::WARN);
            }
        }
        if (race_logic != nullptr) {
            std::map<std::string, const kn5Node*> nodes;
            for (const auto& [_, node] : kn5.nodes) {
                nodes[node.name] = &node;
            }
            // https://assettocorsamods.net/threads/build-your-first-track-basic-guide.12/
            // Read checkpoints
            std::list<TransformationMatrix<float, double, 3>> checkpoints;
            for (size_t i = 0; ; ++i) {
                auto time_l = nodes.find("AC_TIME_" + std::to_string(i) + "_L");
                auto time_r = nodes.find("AC_TIME_" + std::to_string(i) + "_R");
                if ((time_l == nodes.end()) || (time_r == nodes.end())) {
                    break;
                }
                checkpoints.push_back(ac_waypoint(
                    time_l->second->hmatrix.casted<float, double>(),
                    time_r->second->hmatrix.casted<float, double>()));
            }
            // No periodic extension by default.
            {
                auto it = nodes.find("AC_PIT_0");
                if (it != nodes.end()) {
                    race_logic->set_start_pose(ac_start_to_car(it->second->hmatrix.casted<float, double>()), 0);
                }
            }
            {
                auto start_l = nodes.find("AC_AB_START_L");
                auto start_r = nodes.find("AC_AB_START_R");
                if ((start_l != nodes.end()) && (start_r != nodes.end())) {
                    race_logic->set_start_pose(
                        ac_start_to_car(
                            start_l->second->hmatrix.casted<float, double>(),
                            start_r->second->hmatrix.casted<float, double>()),
                        0);
                    auto finish_l = nodes.find("AC_AB_FINISH_L");
                    auto finish_r = nodes.find("AC_AB_FINISH_R");
                    if ((finish_l != nodes.end()) && (finish_r != nodes.end())) {
                        checkpoints.push_back(ac_waypoint(
                            finish_l->second->hmatrix.casted<float, double>(),
                            finish_r->second->hmatrix.casted<float, double>()));
                    }
                    raceway_is_circular = false;
                }
            }
            {
                auto it = nodes.find("AC_START_0");
                if (it != nodes.end()) {
                    race_logic->set_start_pose(ac_start_to_car(it->second->hmatrix.casted<float, double>()), 0);
                }
            }
            // for (const auto& c : checkpoints) {
            //     linfo() << "Checkpoint: " << c.t();
            // }
            if (!checkpoints.empty()) {
                race_logic->set_checkpoints(std::vector(checkpoints.begin(), checkpoints.end()));
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
            auto attrs = MetaAttributes::ATTR_VISIBLE;
            static const DECLARE_REGEX(name_reg, "^(0+)?(\\d+)?(\\w+)");
            Mlib::re::smatch match;
            if (Mlib::re::regex_search(node.name, match, name_reg)) {
                static const DECLARE_REGEX(grass_reg, "^(?:GR|GRASS)(?:\\b|_|\\d)");
                static const DECLARE_REGEX(road_reg, "^ROAD(?:\\b|_|\\d)");
                static const DECLARE_REGEX(gravel_reg, "^GRAVEL(?:\\b|_|\\d)");
                static const DECLARE_REGEX(side_reg, "^SIDE(?:\\b|_|\\d)");
                static const DECLARE_REGEX(tree_reg, "^(?:tree|STREE|bush|bushes)(?:\\b|_|\\d)");
                static const DECLARE_REGEX(vertical_reg, "^(?:WALL|KERB|ROCKS)(?:\\b|_|\\d)");
                if (match[1].matched || match[3].str().starts_with("WALL_col")) {
                    attrs &= ~MetaAttributes::ATTR_VISIBLE;
                }
                if (match[2].matched) {
                    attrs |= MetaAttributes::ATTR_COLLIDABLE;
                }
                if (Mlib::re::regex_search(match[3].str(), grass_reg)) {
                    attrs |= MetaAttributes::SURFACE_GRASS;
                }
                if (Mlib::re::regex_search(match[3].str(), tree_reg)) {
                    attrs |= MetaAttributes::OBJ_TREE;
                }
                if (Mlib::re::regex_search(match[3].str(), road_reg)) {
                    attrs |= MetaAttributes::SURFACE_ROAD;
                }
                if (Mlib::re::regex_search(match[3].str(), gravel_reg)) {
                    attrs |= MetaAttributes::SURFACE_GRAVEL;
                }
                if (Mlib::re::regex_search(match[3].str(), side_reg)) {
                    attrs |= MetaAttributes::SURFACE_SIDE;
                }
                if (Mlib::re::regex_search(match[3].str(), vertical_reg)) {
                    attrs |= MetaAttributes::ATTR_VERTICAL;
                }
            }
            if (any(attrs & MetaAttributes::ATTR_COLLIDABLE)) {
                tl.physics_material |= PhysicsMaterial::ATTR_COLLIDE;
                tl.physics_material |= PhysicsMaterial::ATTR_CONCAVE;
            }
            if (any(attrs & MetaAttributes::SURFACE_ROAD)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_TARMAC;
            }
            if (any(attrs & MetaAttributes::SURFACE_GRAVEL)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_GRAVEL;
            }
            if (any(attrs & MetaAttributes::SURFACE_GRASS)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_GRASS;
            }
            if (any(attrs & MetaAttributes::SURFACE_SIDE)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_DIRT;
            }
            if (!node.isRenderable || !any(attrs & MetaAttributes::ATTR_VISIBLE)) {
                tl.physics_material &= ~PhysicsMaterial::ATTR_VISIBLE;
            }
            if (node.materialID.has_value()) {
                const auto& material = kn5.materials.at(node.materialID.value());
                // From: http://www.toms-sim-side.de/tutorials/dokumente/AC_convert.pdf
                //       https://assettocorsamods.net/threads/setting-up-trees.162/
                if (material.shader == "ksGrass") {
                    attrs |= MetaAttributes::OBJ_GRASS;
                }
                if (material.shader == "ksTree") {
                    attrs |= MetaAttributes::OBJ_TREE;
                }
                if (any(attrs & MetaAttributes::OBJ_GRASS)) {
                    tl.physics_material |= PhysicsMaterial::OBJ_GRASS;
                }
                if (grass_materials.contains(material.name) &&
                    !occluding_meshes.contains(node.name))
                {
                    tl.modifier_backlog.add_foliage = true;
                    // if (!texture_grid.empty()) {
                    //     tl.modifier_backlog.convert_to_terrain = true;
                    // }
                }
                if (any(attrs & (MetaAttributes::OBJ_GRASS | MetaAttributes::OBJ_TREE))) {
                    tl.modifier_backlog.merge_textures = true;
                    // tl.material.wrap_mode_s = WrapMode::CLAMP_TO_EDGE;
                    // tl.material.wrap_mode_t = WrapMode::CLAMP_TO_EDGE;
                    tl.material.occluded_pass = ExternalRenderPassType::NONE;
                    if (any(attrs & MetaAttributes::OBJ_GRASS)) {
                        tl.material.occluder_pass = ExternalRenderPassType::NONE;
                    } else {
                        tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    }
                    tl.material.blend_mode = cfg.blend_mode;
                } else {
                    if ((material.ksAlphaRef.value_or_default() != 0.f) && (material.ksAlphaRef.value_or_default() != 1.f)) {
                        tl.material.occluded_pass = ExternalRenderPassType::NONE;
                        tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    } else {
                        tl.material.occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
                        tl.material.occluder_pass = ExternalRenderPassType::NONE;
                    }
                    if ((material.shader == "ksPerPixel") ||                    // required for Akina track
                        (material.shader == "ksPerPixelAT") ||                  // required for Hondarribia and Akagi tracks
                        (material.shader == "ksPerPixelAT_NM") ||               // required for Semetin track
                        (material.shader == "ksPerPixelMultiMap_AT_NMDetail"))
                    {
                        tl.material.blend_mode = BlendMode::BINARY_05;
                    } else if (material.shader == "ksPerPixelAlpha")
                    {
                        tl.material.blend_mode = cfg.blend_mode;
                    } else if ((material.shader == "ksFlags") ||
                               (material.shader == "ksPerPixelNM") ||
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
                tl.material.emissivity = OrderableFixedArray{
                    cfg.emissivity_factor *
                    material.gain.value_or_default() *
                    material.ksEmissive.value_or_default()};
                tl.material.ambience = OrderableFixedArray{
                    cfg.ambience_factor *
                    material.gain.value_or_default() *
                    material.ksAmbient.value_or_default() *
                    lit_mult};
                tl.material.diffusivity = OrderableFixedArray{
                    cfg.diffusivity_factor *
                    material.gain.value_or_default() *
                    material.diffuseMult.value_or_default() *
                    material.ksDiffuse.value_or_default() *
                    lit_mult};
                tl.material.specularity = OrderableFixedArray{
                    cfg.specularity_factor *
                    material.gain.value_or_default() *
                    material.ksSpecular.value_or_default() *
                    lit_mult *
                    specular_mult};
                tl.material.specular_exponent = material.ksSpecularEXP.value_or_default();
                if ((material.useDetail.value_or_default() != 0.f) &&
                    (material.detailUVMultiplier.value_or_default() != 0.f) &&
                    !material.txDiffuse.empty() &&
                    !material.txDetail1.empty())
                {
                    tl.material.textures_color = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = {.filename = material.txDiffuse},
                            .normal = {.filename = material.txNormal},
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .role = BlendMapRole::DETAIL_BASE,
                        .reweight_mode = BlendMapReweightMode::DISABLED}};
                    tl.material.textures_color.push_back(BlendMapTexture{
                        .texture_descriptor = {
                            .color = {.filename = material.txDetail1},
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .scale = material.detailUVMultiplier.value_or_default(),
                        .role = BlendMapRole::DETAIL_COLOR,
                        .uv_source = BlendMapUvSource::VERTICAL});
                    tl.material.compute_color_mode();
                } else if (
                    !material.txDiffuse.empty() &&
                    !material.txMask.empty() &&
                    (material.detailUVMultiplier.value_or_default() != 0.f) &&
                    ((material.shader == "ksMultilayer") ||
                     (material.shader == "ksMultilayer_fresnel_nm")))
                {
                    tl.material.textures_color = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = {.filename = material.txDiffuse},
                            .normal = {.filename = material.txNormal},
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .weight = material.magicMult.value_or_default(),
                        .role = BlendMapRole::DETAIL_BASE,
                        .reweight_mode = BlendMapReweightMode::DISABLED}};
                    for (uint32_t i = 0; i < 4; ++i) {
                        if (material.txDetail4(i).empty() ||
                            (material.mult(i) == 0.f))
                        {
                            continue;
                        }
                        tl.material.textures_color.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = {.filename = material.txMask},
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                            .role = BlendMapRole::DETAIL_MASK_R + i});
                        tl.material.textures_color.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = {.filename = material.txDetail4(i)},
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                            .scale = material.mult(i),
                            .role = BlendMapRole::DETAIL_COLOR,
                            .uv_source = any(attrs & MetaAttributes::ATTR_VERTICAL)
                                ? BlendMapUvSource::VERTICAL
                                : BlendMapUvSource::HORIZONTAL});
                    }
                    tl.material.compute_color_mode();
                } else {
                    if (!material.txDiffuse.empty()) {
                        tl.material.textures_color = {BlendMapTexture{
                            .texture_descriptor = {
                                .color = {.filename = material.txDiffuse},
                                .normal = {.filename = material.txNormal},
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS}}};
                        tl.material.compute_color_mode();
                    }
                }
            }
            for (const auto& tri : node.triangles) {
                tl.draw_triangle_with_normals(
                    node.position(tri(0)) TEMPLATEV casted<TPos>(),
                    node.position(tri(1)) TEMPLATEV casted<TPos>(),
                    node.position(tri(2)) TEMPLATEV casted<TPos>(),
                    node.normal(tri(0)),
                    node.normal(tri(1)),
                    node.normal(tri(2)),
                    {1.f, 1.f, 1.f},                            // c00
                    {1.f, 1.f, 1.f},                            // c10
                    {1.f, 1.f, 1.f},                            // c01
                    node.uv(tri(0)),
                    node.uv(tri(1)),
                    node.uv(tri(2)),
                    {},                                         // b00
                    {},                                         // b10
                    {},                                         // b01
                    cfg.triangle_tangent_error_behavior);
            }
            result.push_back(tl.triangle_array());
        }
    };
    {
        auto ext_config_ini_filename =
            fs::path{filename}.parent_path() /
            fs::path{"extension"} /
            fs::path{"ext_config.ini"};
        if (path_exists(ext_config_ini_filename)) {
            IniParser ext_config{ext_config_ini_filename};
            static const DECLARE_REGEX(re, "\\, *");
            if (auto gm = ext_config.try_get("LIGHTING", "LIT_MULT");
                gm.has_value())
            {
                lit_mult = safe_stof(gm.value());
            }
            if (auto gm = ext_config.try_get("LIGHTING", "SPECULAR_MULT");
                gm.has_value())
            {
                specular_mult = safe_stof(gm.value());
            }
            if (auto gm = ext_config.try_get("GRASS_FX", "GRASS_MATERIALS");
                gm.has_value())
            {
                grass_materials = string_to_set(gm.value(), re);
            }
            if (auto gm = ext_config.try_get("GRASS_FX", "OCCLUDING_MESHES");
                gm.has_value())
            {
                occluding_meshes = string_to_set(gm.value(), re);
            }
            if (auto gm = ext_config.try_get("GRASS_FX", "TEXTURE_GRID");
                gm.has_value())
            {
                texture_grid = string_to_vector(gm.value(), re, safe_stou);
            }
        }
    }
    if (filename.ends_with(".ini")) {
        IniParser ini{filename};
        for (const auto& [name, section] : ini.sections()) {
            if (name.starts_with("MODEL_")) {
                auto it = section.find("FILE");
                if (it == section.end()) {
                    THROW_OR_ABORT("Could not find FILE variable in section of INI file: \"" + filename + '"');
                }
                append_kn5((fs::path{filename}.parent_path() / it->second).string());
            }
        }
    } else {
        append_kn5(filename);
    }
    race_logic->set_circularity(raceway_is_circular);
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
template std::list<std::shared_ptr<ColoredVertexArray<float>>> load_kn5_array<float>(
    const std::string& file_or_directory,
    const LoadMeshConfig<float>&,
    IDdsResources*,
    IRaceLogic*);
template std::list<std::shared_ptr<ColoredVertexArray<double>>> load_kn5_array<double>(
    const std::string& file_or_directory,
    const LoadMeshConfig<double>&,
    IDdsResources*,
    IRaceLogic*);
}
