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
    return TransformationMatrix<float, double, 3>{ac_start_to_car(tm.R()), tm.t()};
}

static TransformationMatrix<float, double, 3> ac_center(
    const TransformationMatrix<float, double, 3>& left,
    const TransformationMatrix<float, double, 3>& right)
{
    // Semetin
    auto d = dot0d((right.t() - left.t()).casted<float>(), left.R().column(0));
    return TransformationMatrix<float, double, 3>{
        d > 0.f
            ? left.R()
            : dot2d(left.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, 180.f * degrees)),
        (left.t() + right.t()) / 2.};
}

static TransformationMatrix<float, double, 3> ac_waypoint(
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

enum class MetaAttributes {
    NONE = 0,
    VISIBLE = (1 << 0),
    COLLIDABLE = (1 << 1),
    GRASS = (1 << 2),
    ROAD = (1 << 3),
    GRAVEL = (1 << 4),
    SIDE = (1 << 5),
    TREE = (1 << 6),
    HORIZONTAL = (1 << 7),
    VERTICAL = (1 << 8)
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
    std::set<std::string> grass_materials;
    std::set<std::string> occluding_meshes;
    std::vector<unsigned int> texture_grid;

    auto append_kn5 = [&](const std::string& kn5_filename) {
        auto kn5 = load_kn5(kn5_filename);
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
                auto start_l = nodes.find("AC_AB_START_L");
                auto start_r = nodes.find("AC_AB_START_R");
                if ((start_l != nodes.end()) && (start_r != nodes.end())) {
                    race_logic->set_start_pose(
                        ac_center(
                            start_l->second->hmatrix.casted<float, double>(),
                            start_r->second->hmatrix.casted<float, double>()),
                        0);
                    auto finish_l = nodes.find("AC_AB_FINISH_L");
                    auto finish_r = nodes.find("AC_AB_FINISH_R");
                    if ((finish_l != nodes.end()) && (finish_r != nodes.end())) {
                        race_logic->set_checkpoints({
                            ac_waypoint(
                                finish_l->second->hmatrix.casted<float, double>(),
                                finish_r->second->hmatrix.casted<float, double>())});
                    }
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
            static const DECLARE_REGEX(name_reg, "^(\\d+)?(\\w+)");
            Mlib::re::smatch match;
            if (Mlib::re::regex_search(node.name, match, name_reg)) {
                static const DECLARE_REGEX(grass_reg, "^(?:GR|GRASS)(?:\\b|_)");
                static const DECLARE_REGEX(road_reg, "^ROAD(?:\\b|_)");
                static const DECLARE_REGEX(gravel_reg, "^GRAVEL(?:\\b|_)");
                static const DECLARE_REGEX(side_reg, "^SIDE(?:\\b|_)");
                static const DECLARE_REGEX(tree_reg, "^(?:tree|STREE|bush)(?:\\b|_)");
                static const DECLARE_REGEX(horizontal_reg, "^(?:GRAVEL|ROAD|PITS|VISIBLE_SURFACE|GR\\b|GRASS|Terrain|SIDE|far_ter)(?:\\b|_)");
                static const DECLARE_REGEX(vertical_reg, "^(?:WALL|KERB|ROCKS)(?:\\b|_)");
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
                if (Mlib::re::regex_search(match[2].str(), tree_reg)) {
                    attrs |= MetaAttributes::TREE;
                }
                if (Mlib::re::regex_search(match[2].str(), road_reg)) {
                    attrs |= MetaAttributes::ROAD;
                }
                if (Mlib::re::regex_search(match[2].str(), gravel_reg)) {
                    attrs |= MetaAttributes::GRAVEL;
                }
                if (Mlib::re::regex_search(match[2].str(), side_reg)) {
                    attrs |= MetaAttributes::SIDE;
                }
                if (Mlib::re::regex_search(match[2].str(), horizontal_reg)) {
                    attrs |= MetaAttributes::HORIZONTAL;
                }
                if (Mlib::re::regex_search(match[2].str(), vertical_reg)) {
                    attrs |= MetaAttributes::VERTICAL;
                }
            }
            if (any(attrs & MetaAttributes::COLLIDABLE)) {
                tl.physics_material |= PhysicsMaterial::ATTR_COLLIDE;
                tl.physics_material |= PhysicsMaterial::ATTR_CONCAVE;
            }
            if (any(attrs & MetaAttributes::ROAD)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_TARMAC;
            }
            if (any(attrs & MetaAttributes::GRAVEL)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_GRAVEL;
            }
            if (any(attrs & MetaAttributes::GRASS)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_GRASS;
            }
            if (any(attrs & MetaAttributes::SIDE)) {
                if (any(tl.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.physics_material |= PhysicsMaterial::SURFACE_BASE_DIRT;
            }
            if (!node.isRenderable || !any(attrs & MetaAttributes::VISIBLE)) {
                tl.physics_material &= ~PhysicsMaterial::ATTR_VISIBLE;
            }
            if (node.materialID.has_value()) {
                const auto& material = kn5.materials.at(node.materialID.value());
                // From: http://www.toms-sim-side.de/tutorials/dokumente/AC_convert.pdf
                //       https://assettocorsamods.net/threads/setting-up-trees.162/
                if ((material.shader == "ksGrass") ||
                    (material.shader == "ksTree"))
                {
                    attrs |= MetaAttributes::TREE;
                }
                if (grass_materials.contains(material.name) &&
                    !occluding_meshes.contains(node.name))
                {
                    tl.modifier_backlog.add_foliage = true;
                    // if (!texture_grid.empty()) {
                    //     tl.modifier_backlog.convert_to_terrain = true;
                    // }
                }
                if (any(attrs & MetaAttributes::TREE)) {
                    tl.modifier_backlog.merge_textures = true;
                    tl.material.wrap_mode_s = WrapMode::CLAMP_TO_EDGE;
                    tl.material.wrap_mode_t = WrapMode::CLAMP_TO_EDGE;
                    tl.material.occluded_pass = ExternalRenderPassType::NONE;
                    tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    tl.material.blend_mode = cfg.blend_mode;
                } else {
                    if ((material.ksAlphaRef != 0.f) && (material.ksAlphaRef != 1.f)) {
                        tl.material.occluded_pass = ExternalRenderPassType::NONE;
                        tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    } else {
                        tl.material.occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
                        tl.material.occluder_pass = ExternalRenderPassType::NONE;
                    }
                    if ((material.shader == "ksPerPixel") ||                // required for Akina track
                        (material.shader == "ksPerPixelAT_NM"))             // required for Semetin track
                    {
                        tl.material.blend_mode = BlendMode::BINARY_05;
                    } else if ((material.shader == "ksPerPixelAlpha") ||
                               (material.shader == "ksPerPixelAT") ||       // required for Hondarribia track
                               (material.shader == "ksPerPixelMultiMap_AT_NMDetail"))
                    {
                        tl.material.blend_mode = cfg.blend_mode;
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
                tl.material.emissivity = OrderableFixedArray{cfg.emissivity_factor * material.ksEmissive};
                tl.material.ambience = OrderableFixedArray{cfg.ambience_factor * material.ksAmbient};
                tl.material.diffusivity = OrderableFixedArray{cfg.diffusivity_factor * material.ksDiffuse};
                tl.material.specularity = OrderableFixedArray{cfg.specularity_factor * material.ksSpecular};
                tl.material.specular_exponent = material.ksSpecularEXP;
                if ((material.useDetail != 0.f) &&
                    (material.detailUVMultiplier != 0.f) &&
                    !material.txDiffuse.empty() &&
                    !material.txDetail1.empty())
                {
                    tl.material.detail_multiplier = 1.f;
                    tl.material.textures = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = material.txDiffuse,
                            .normal = material.txNormal,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .role = BlendMapRole::DETAIL_BASE}};
                    tl.material.textures.push_back(BlendMapTexture{
                        .texture_descriptor = {
                            .color = material.txDetail1,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .scale = material.detailUVMultiplier,
                        .role = BlendMapRole::DETAIL_COLOR_VERTICAL});
                    tl.material.compute_color_mode();
                } else if (
                    !material.txDiffuse.empty() &&
                    !material.txMask.empty() &&
                    (material.detailUVMultiplier != 0.f) &&
                    ((material.shader == "ksMultilayer") ||
                     (material.shader == "ksMultilayer_fresnel_nm")))
                {
                    tl.material.detail_multiplier = material.magicMult;
                    tl.material.textures = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = material.txDiffuse,
                            .normal = material.txNormal,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                        .role = BlendMapRole::DETAIL_BASE}};
                    for (uint32_t i = 0; i < 4; ++i) {
                        if (material.txDetail4(i).empty() ||
                            (material.mult(i) == 0.f))
                        {
                            continue;
                        }
                        if (any(attrs & MetaAttributes::HORIZONTAL) == any(attrs & MetaAttributes::VERTICAL)) {
                            THROW_OR_ABORT("Could not determine horizontal/vertical UV-coordinates for node \"" + node.name + '"');
                        }
                        tl.material.textures.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = material.txMask,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                            .role = BlendMapRole::DETAIL_MASK_R + i});
                        tl.material.textures.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = material.txDetail4(i),
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS},
                            .scale = material.mult(i),
                            .role = any(attrs & MetaAttributes::VERTICAL)
                                ? BlendMapRole::DETAIL_COLOR_VERTICAL
                                : BlendMapRole::DETAIL_COLOR_HORIZONTAL});
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
    {
        auto ext_config_ini_filename =
            fs::path{filename}.parent_path() /
            fs::path{"extension"} /
            fs::path{"ext_config.ini"};
        if (path_exists(ext_config_ini_filename)) {
            IniParser ext_config{ext_config_ini_filename};
            static const DECLARE_REGEX(re, "\\, *");
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
