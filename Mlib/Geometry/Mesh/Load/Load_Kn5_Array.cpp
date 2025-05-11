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
#include <Mlib/Geometry/Vertex_Transformation.hpp>
#include <Mlib/Images/Flip_Mode.hpp>
#include <Mlib/Io/Ini_Parser.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <filesystem>
#include <map>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

using namespace Mlib;

static const FixedArray<ScenePos, 3> SPAWN_OFFSET = {0.f, 2.f, 0.f};

static const auto M = FixedArray<float, 3, 3>::init(
    -1.f, 0.f, 0.f,
    0.f, 1.f, 0.f,
    0.f, 0.f, -1.f);

static FixedArray<float, 3, 3> trafo(const FixedArray<float, 3, 3>& R) {
    return dot2d(dot2d(M.T(), R), M);
}

static FixedArray<float, 3, 3> ac_start_to_car(const FixedArray<float, 3, 3>& R)
{
    if (R(1, 1) > 0.f) {
        // Akagi
        return dot2d(R, tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, 180.f * degrees, 0.f}));
    } else {
        // Hondarribia, Irohazaka
        return trafo(dot2d(R, tait_bryan_angles_2_matrix(FixedArray<float, 3>{180.f * degrees, 0.f, 0.f})));
    }
}

static TransformationMatrix<float, ScenePos, 3> ac_start_to_car(const TransformationMatrix<float, ScenePos, 3>& tm)
{
    return TransformationMatrix<float, ScenePos, 3>{ac_start_to_car(tm.R), tm.t + SPAWN_OFFSET};
}

// static TransformationMatrix<float, ScenePos, 3> ac_center(
//     const TransformationMatrix<float, ScenePos, 3>& left,
//     const TransformationMatrix<float, ScenePos, 3>& right)
// {
//     // Semetin
//     auto d = dot0d((right.t() - left.t()).casted<float>(), left.R().column(0));
//     return TransformationMatrix<float, ScenePos, 3>{
//         d > 0.f
//             ? left.R()
//             : dot2d(left.R(), rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, 180.f * degrees)),
//         (left.t() + right.t()) / 2.};
// }

static TransformationMatrix<float, ScenePos, 3> ac_portal(
    const TransformationMatrix<float, ScenePos, 3>& left,
    const TransformationMatrix<float, ScenePos, 3>& right)
{
    auto x = (right.t - left.t).casted<float>();
    auto y = FixedArray<float, 3>{ 0.f, 1.f, 0.f };
    x -= dot0d(x, y) * y;
    x /= std::sqrt(sum(squared(x)));
    auto z = cross(x, y);
    return TransformationMatrix<float, ScenePos, 3>{
        FixedArray<float, 3, 3>::init(
            x(0), y(0), z(0),
            x(1), y(1), z(1),
            x(2), y(2), z(2)),
        (left.t + right.t) / ScenePos(2)};
}

static TransformationMatrix<float, ScenePos, 3> ac_start_to_car(
    const TransformationMatrix<float, ScenePos, 3>& left,
    const TransformationMatrix<float, ScenePos, 3>& right)
{
    return ac_portal(left, right);
}

static TransformationMatrix<float, ScenePos, 3> ac_waypoint(
    const TransformationMatrix<float, ScenePos, 3>& left,
    const TransformationMatrix<float, ScenePos, 3>& right)
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
    SURFACE_SKIDS = (1 << 6),
    SURFACE_ANY = SURFACE_GRASS | SURFACE_ROAD | SURFACE_GRAVEL | SURFACE_SIDE | SURFACE_SKIDS,
    OBJ_GRASS = (1 << 7),
    OBJ_TREE = (1 << 8),
    ATTR_VERTICAL = (1 << 9)
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

struct MaterialProperties {
    std::optional<float> ksDiffuse;
    std::optional<float> ksAmbient;
    std::optional<float> ksSpecular;
    std::optional<float> ksSpecularEXP;
    std::optional<float> ksEmissive;
    std::optional<float> ksAlphaRef;
};

struct MaterialSettings {
    std::string shaderName;
    std::string alphaBlendMode;
    bool alphaTested = false;
    std::string depthMode;
    MaterialProperties properties;
};

struct SettingsJson {
    std::map<std::string, MaterialSettings> materials;
};

void from_json(const nlohmann::json& j, MaterialProperties& v) {
    JsonView jv{ j };
    if (j.contains("ksDiffuse")) {
        v.ksDiffuse = JsonView{ jv.at("ksDiffuse") }.at<float>("valueA");
    }
    if (j.contains("ksAmbient")) {
        v.ksAmbient = JsonView{ jv.at("ksAmbient") }.at<float>("valueA");
    }
    if (j.contains("ksSpecular")) {
        v.ksSpecular = JsonView{ jv.at("ksSpecular") }.at<float>("valueA");
    }
    if (j.contains("ksSpecularEXP")) {
        v.ksSpecularEXP = JsonView{ jv.at("ksSpecularEXP") }.at<float>("valueA");
    }
    if (j.contains("ksEmissive")) {
        v.ksEmissive = JsonView{ jv.at("ksEmissive") }.at<float>("valueA");
    }
    if (j.contains("ksAlphaRef")) {
        v.ksAlphaRef = JsonView{ jv.at("ksAlphaRef") }.at<float>("valueA");
    }
}

void from_json(const nlohmann::json& j, MaterialSettings& v) {
    JsonView jv{ j };
    v.shaderName = jv.at<std::string>("shaderName", "");
    v.alphaBlendMode = jv.at<std::string>("alphaBlendMode", "");
    v.alphaTested = jv.at<bool>("alphaTested", false);
    v.depthMode = jv.at<std::string>("depthMode", "");
    v.properties = jv.at<MaterialProperties>("properties");
}

void from_json(const nlohmann::json& j, SettingsJson& v) {
    JsonView jv{ j };
    if (jv.contains("materials")) {
        v.materials = jv.at<std::map<std::string, MaterialSettings>>("materials");
    }
}

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> Mlib::load_kn5_array(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources,
    IRaceLogic* race_logic)
{
    const bool show_only_collidables = false;
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    std::unordered_map<VariableAndHash<std::string>, kn5Texture> textures;
    std::set<std::string> grass_materials;
    std::set<std::string> occluding_meshes;
    std::vector<unsigned int> texture_grid;
    float lit_mult = 1.f;
    float specular_mult = 1.f;
    // bool raceway_is_circular = true;
    SettingsJson settings_json;

    auto append_kn5 = [&](const std::string& kn5_filename) {
        auto kn5 = load_kn5(kn5_filename);
        while (!kn5.textures.empty()) {
            textures.insert(kn5.textures.extract(kn5.textures.begin()));
        }
        bool is_physics_file = fs::path{ kn5_filename }.filename().string().starts_with("physics_");
        for (auto& [_, m] : kn5.materials) {
            if (settings_json.materials.contains(m.name)) {
                const auto& ms = settings_json.materials.at(m.name);
                if (ms.properties.ksDiffuse.has_value()) {
                    m.ksDiffuse = *ms.properties.ksDiffuse;
                }
                if (ms.properties.ksAmbient.has_value()) {
                    m.ksAmbient = *ms.properties.ksAmbient;
                }
                if (ms.properties.ksSpecular.has_value()) {
                    m.ksSpecular = *ms.properties.ksSpecular;
                }
                if (ms.properties.ksSpecularEXP.has_value()) {
                    m.ksSpecularEXP = *ms.properties.ksSpecularEXP;
                }
                if (ms.properties.ksEmissive.has_value()) {
                    m.ksEmissive = *ms.properties.ksEmissive;
                }
                if (ms.properties.ksAlphaRef.has_value()) {
                    m.ksAlphaRef = *ms.properties.ksAlphaRef;
                }
            }
        }
        if (race_logic != nullptr) {
            std::map<std::string, const kn5Node*> nodes;
            for (const auto& [_, node] : kn5.nodes) {
                nodes[node.name] = &node;
            }
            // https://assettocorsamods.net/threads/build-your-first-track-basic-guide.12/
            // Read checkpoints
            std::list<TransformationMatrix<float, ScenePos, 3>> checkpoints;
            for (size_t i = 0; ; ++i) {
                auto time_l = nodes.find("AC_TIME_" + std::to_string(i) + "_L");
                auto time_r = nodes.find("AC_TIME_" + std::to_string(i) + "_R");
                if ((time_l == nodes.end()) || (time_r == nodes.end())) {
                    break;
                }
                checkpoints.push_back(ac_waypoint(
                    time_l->second->hmatrix.casted<float, ScenePos>(),
                    time_r->second->hmatrix.casted<float, ScenePos>()));
            }
            // No periodic extension by default.
            if (auto it = nodes.find("AC_PIT_0"); it != nodes.end()) {
                race_logic->set_start_pose(
                    ac_start_to_car(it->second->hmatrix.casted<float, ScenePos>()),
                    fixed_zeros<float, 3>() / kph,  // velocity
                    fixed_zeros<float, 3>() / rpm,  // angular_velocity
                    0);                             // rank
            }
            auto find_ab = [&](
                const std::string& name_start_l,
                const std::string& name_start_r,
                const std::string& name_finish_l,
                const std::string& name_finish_r)
            {
                auto start_l = nodes.find(name_start_l);
                auto start_r = nodes.find(name_start_r);
                if ((start_l != nodes.end()) && (start_r != nodes.end())) {
                    race_logic->set_start_pose(
                        ac_start_to_car(
                            start_l->second->hmatrix.casted<float, ScenePos>(),
                            start_r->second->hmatrix.casted<float, ScenePos>()),
                        fixed_zeros<float, 3>() / kph,  // velocity
                        fixed_zeros<float, 3>() / rpm,  // angular_velocity
                        0);                             // rank
                    auto finish_l = nodes.find(name_finish_l);
                    auto finish_r = nodes.find(name_finish_r);
                    if ((finish_l != nodes.end()) && (finish_r != nodes.end())) {
                        checkpoints.push_back(ac_waypoint(
                            finish_l->second->hmatrix.casted<float, ScenePos>(),
                            finish_r->second->hmatrix.casted<float, ScenePos>()));
                    }
                    // raceway_is_circular = false;
                }
            };
            find_ab(
                "AC_AB_START_L",
                "AC_AB_START_R",
                "AC_AB_FINISH_L",
                "AC_AB_FINISH_R");
            find_ab(
                "AC_OPEN_START_L",
                "AC_OPEN_START_R",
                "AC_OPEN_FINISH_L",
                "AC_OPEN_FINISH_R");
            if (auto it = nodes.find("AC_START_0"); it != nodes.end()) {
                race_logic->set_start_pose(
                    ac_start_to_car(it->second->hmatrix.casted<float, ScenePos>()),
                    fixed_zeros<float, 3>() / kph,  // velocity
                    fixed_zeros<float, 3>() / rpm,  // angular_velocity
                    0);                             // rank
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
            if (node.name.starts_with("AC_")) {
                continue;
            }
            auto material = node.materialID.has_value()
                ? &kn5.materials.at(*node.materialID)
                : nullptr;

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
                    .cull_faces = cfg.cull_faces_default,
                    .shading = {
                        .fog_distances = OrderableFixedArray{ cfg.shading.fog_distances },
                        .fog_ambient = OrderableFixedArray{ cfg.shading.fog_ambient }},
                    .dynamically_lighted = cfg.dynamically_lighted},
                Morphology{
                    .physics_material = cfg.physics_material,
                    .center_distances2 = SquaredStepDistances::from_distances(cfg.center_distances),
                    .max_triangle_distance = cfg.max_triangle_distance
                }};
            auto attrs = MetaAttributes::ATTR_VISIBLE;
            static const DECLARE_REGEX(name_reg, "^(\\d+)?(\\w+)");
            static const size_t NUMBER = 1;
            static const size_t NAME = 2;
            Mlib::re::cmatch match;
            if (Mlib::re::regex_search(node.name, match, name_reg)) {
                static const DECLARE_REGEX(grass_reg, "^(?:grass|GR|GRASS)(?:\\b|_|\\d)");
                static const DECLARE_REGEX(road_reg, "^(?:road|ROAD)");
                static const DECLARE_REGEX(gravel_reg, "^(?:sandgravel|SAND|GRA|GRAV|GRAVEL)(?:\\b|_|\\d)");
                static const DECLARE_REGEX(side_reg, "^SIDE(?:\\b|_|\\d)");
                static const DECLARE_REGEX(skids_reg, "^SKIDS(?:\\b|_|\\d)");
                static const DECLARE_REGEX(tree_reg, "^(?:tree|STREE|bush|bushes)(?:\\b|_|\\d)");
                static const DECLARE_REGEX(vertical_reg, "^(?:wall|WALL|KERB|ROCKS)(?:\\b|_|\\d)");
                auto number = match[NUMBER].matched
                    ? std::optional{ safe_stou(match[NUMBER].str()) }
                    : std::nullopt;
                if (match[NAME].str().starts_with("WALL_col") ||
                    match[NAME].str().starts_with("INVISIBLE") ||
                    is_physics_file)
                {
                    attrs &= ~MetaAttributes::ATTR_VISIBLE;
                }
                if (number.has_value() && (*number > 0)) {
                    attrs |= MetaAttributes::ATTR_COLLIDABLE;
                }
                if (any(attrs & MetaAttributes::ATTR_COLLIDABLE) &&
                    Mlib::re::regex_search(match[NAME].str(), grass_reg))
                {
                    attrs |= MetaAttributes::SURFACE_GRASS;
                }
                if (Mlib::re::regex_search(match[NAME].str(), tree_reg)) {
                    attrs |= MetaAttributes::OBJ_TREE;
                }
                if (Mlib::re::regex_search(match[NAME].str(), road_reg)) {
                    attrs |= MetaAttributes::SURFACE_ROAD;
                }
                if (Mlib::re::regex_search(match[NAME].str(), gravel_reg)) {
                    attrs |= MetaAttributes::SURFACE_GRAVEL;
                }
                if (Mlib::re::regex_search(match[NAME].str(), side_reg)) {
                    attrs |= MetaAttributes::SURFACE_SIDE;
                }
                if (Mlib::re::regex_search(match[NAME].str(), skids_reg)) {
                    attrs |= MetaAttributes::SURFACE_SKIDS;
                }
                if (Mlib::re::regex_search(match[NAME].str(), vertical_reg)) {
                    attrs |= MetaAttributes::ATTR_VERTICAL;
                }
                if (any(attrs & MetaAttributes::SURFACE_ANY) &&
                    (material != nullptr) &&
                    (material->shader == "ksGrass"))
                {
                    attrs &= ~MetaAttributes::SURFACE_ANY;
                    attrs |= MetaAttributes::SURFACE_GRASS;
                }
            }
            if (any(attrs & MetaAttributes::ATTR_COLLIDABLE)) {
                tl.morphology.physics_material |= PhysicsMaterial::ATTR_COLLIDE;
                tl.morphology.physics_material |= PhysicsMaterial::ATTR_CONCAVE;
            }
            if (any(attrs & MetaAttributes::SURFACE_ROAD)) {
                if (any(tl.morphology.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.morphology.physics_material |= PhysicsMaterial::SURFACE_BASE_TARMAC;
                tl.material.contains_skidmarks = true;
            }
            if (any(attrs & MetaAttributes::SURFACE_GRAVEL)) {
                if (any(tl.morphology.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.morphology.physics_material |= PhysicsMaterial::SURFACE_BASE_GRAVEL;
                tl.material.contains_skidmarks = true;
            }
            if (any(attrs & MetaAttributes::SURFACE_GRASS)) {
                if (any(tl.morphology.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.morphology.physics_material |= PhysicsMaterial::SURFACE_BASE_GRASS;
            }
            if (any(attrs & MetaAttributes::SURFACE_SIDE)) {
                if (any(tl.morphology.physics_material & PhysicsMaterial::SURFACE_BASE_MASK)) {
                    THROW_OR_ABORT("Surface material already set");
                }
                tl.morphology.physics_material |= PhysicsMaterial::SURFACE_BASE_DIRT;
            }
            if (node.isRenderable && any(attrs & MetaAttributes::ATTR_VISIBLE)) {
                tl.morphology.physics_material |= PhysicsMaterial::ATTR_VISIBLE;
            }
            if (material != nullptr) {
                // From: http://www.toms-sim-side.de/tutorials/dokumente/AC_convert.pdf
                //       https://assettocorsamods.net/threads/setting-up-trees.162/
                if (!any(attrs & MetaAttributes::SURFACE_ANY) &&
                    (material->shader == "ksGrass"))
                {
                    attrs |= MetaAttributes::OBJ_GRASS;
                }
                if (material->shader == "ksTree") {
                    attrs |= MetaAttributes::OBJ_TREE;
                }
                if (any(attrs & MetaAttributes::OBJ_GRASS)) {
                    tl.morphology.physics_material |= PhysicsMaterial::OBJ_GRASS;
                }
                if (grass_materials.contains(material->name) &&
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
                    if ((material->ksAlphaRef.value_or_default() != 0.f) && (material->ksAlphaRef.value_or_default() != 1.f)) {
                        tl.material.occluded_pass = ExternalRenderPassType::NONE;
                        tl.material.occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_AND_LOCAL;
                    } else {
                        tl.material.occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
                        tl.material.occluder_pass = ExternalRenderPassType::NONE;
                    }
                    if ((material->shader == "ksPerPixel") ||                       // required for Akina track
                        (material->shader == "ksPerPixelAT") ||                     // required for Hondarribia and Akagi tracks
                        (material->shader == "ksPerPixelAT_NM") ||                  // required for Semetin track
                        (material->shader == "ksPerPixelAT_NS") ||                  // required for Gunsai Touge (Enhanced)
                        (material->shader == "ksPerPixelMultiMap_AT") ||
                        (material->shader == "ksPerPixelMultiMap_AT_NMDetail"))
                    {
                        if (any(attrs & MetaAttributes::SURFACE_SKIDS)) {
                            tl.material.blend_mode = cfg.blend_mode;
                        } else {
                            tl.material.blend_mode = BlendMode::BINARY_05;
                        }
                    } else if (material->shader == "ksPerPixelAlpha")
                    {
                        tl.material.blend_mode = cfg.blend_mode;
                    } else if ((material->shader == "ksGrass") ||
                               (material->shader == "ksFlags") ||
                               (material->shader == "ksPerPixelNM") ||
                               (material->shader == "ksPerPixelMultiMap") ||
                               (material->shader == "ksPerPixelMultiMap_NMDetail") ||
                               (material->shader == "ksPerPixelReflection") ||
                               (material->shader == "ksPerPixelSimpleRefl") ||
                               (material->shader == "ksMultilayer") ||
                               (material->shader == "ksMultilayer_fresnel_nm") ||
                               (material->shader == "ksMultilayer_objsp"))
                    {
                        tl.material.blend_mode = BlendMode::OFF;
                    } else {
                        THROW_OR_ABORT("Unknown shader: \"" + material->shader + '"');
                    }
                }
                tl.material.shading.emissive = OrderableFixedArray{
                    cfg.emissive_factor *
                    material->ksEmissive.value_or_default()};
                tl.material.shading.ambient = OrderableFixedArray{
                    cfg.ambient_factor *
                    material->ksAmbient.value_or_default() *
                    lit_mult};
                tl.material.shading.diffuse = OrderableFixedArray{
                    cfg.diffuse_factor *
                    material->diffuseMult.value_or_default() *
                    material->ksDiffuse.value_or_default() *
                    lit_mult};
                tl.material.shading.specular = OrderableFixedArray{
                    cfg.specular_factor *
                    material->ksSpecular.value_or_default() *
                    lit_mult *
                    specular_mult};
                tl.material.shading.specular_exponent = material->ksSpecularEXP.value_or_default();
                if (tl.material.shading.specular_exponent == 0.f) {
                    tl.material.shading.specular = 0.f;
                }
                if ((material->fresnelEXP.value_or_default() != 0.f) &&
                    (material->fresnelMaxLevel.value_or_default() > material->fresnelC.value_or_default() + 1e-3))
                {
                    auto fac = std::min(1.f, 0.3f / material->fresnelMaxLevel.value_or_default());
                    tl.material.shading.fresnel.reflectance.min = fac * material->fresnelC.value_or_default();
                    tl.material.shading.fresnel.reflectance.max = fac * material->fresnelMaxLevel.value_or_default();
                    tl.material.shading.fresnel.reflectance.exponent = material->fresnelEXP.value_or_default();
                    tl.material.shading.fresnel.ambient = cfg.shading.fresnel.ambient;
                }
                if (any(attrs & MetaAttributes::SURFACE_GRASS) &&
                    (material->shader == "ksGrass") &&
                    !material->txDiffuse->empty() &&
                    !material->txVariation->empty())
                {
                    tl.material.textures_color = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = {
                                .filename = material->txDiffuse,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                            .specular = {},
                            .normal = ColormapWithModifiers{
                                .filename = material->txNormal,
                                .color_mode = ColorMode::RGB,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level}},
                        .role = BlendMapRole::DETAIL_BASE,
                        .reweight_mode = BlendMapReweightMode::DISABLED}};
                    tl.material.textures_color.push_back(BlendMapTexture{
                        .texture_descriptor = {
                            .color = {
                                .filename = material->txVariation,
                                .color_mode = ColorMode::RGB,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                            .specular = {},
                            .normal = {}},
                        .scale = 0.5f,
                        .role = BlendMapRole::DETAIL_COLOR,
                        .uv_source = BlendMapUvSource::HORIZONTAL});
                } else if (
                    (material->useDetail.value_or_default() != 0.f) &&
                    (material->detailUVMultiplier.value_or_default() != 0.f) &&
                    !material->txDiffuse->empty() &&
                    !material->txDetail1->empty())
                {
                    tl.material.textures_color = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = {
                                .filename = material->txDiffuse,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                            .specular = {},
                            .normal = {
                                .filename = material->txNormal,
                                .color_mode = ColorMode::RGB,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level}},
                        .role = BlendMapRole::DETAIL_BASE,
                        .reweight_mode = BlendMapReweightMode::DISABLED}};
                    tl.material.textures_color.push_back(BlendMapTexture{
                        .texture_descriptor = {
                            .color = {
                                .filename = material->txDetail1,
                                .color_mode = ColorMode::RGB,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                            .specular = {},
                            .normal = {}},
                        .scale = material->detailUVMultiplier.value_or_default(),
                        .role = BlendMapRole::DETAIL_COLOR,
                        .uv_source = BlendMapUvSource::VERTICAL0});
                } else if (
                    !material->txDiffuse->empty() &&
                    !material->txMask->empty() &&
                    any(material->mult.template applied<bool>([](const auto& v) { return v.value_or_default() != 0.f; })) &&
                    (material->detailUVMultiplier.value_or_default() != 0.f) &&
                    ((material->shader == "ksMultilayer") ||
                     (material->shader == "ksMultilayer_fresnel_nm") ||
                     (material->shader == "ksMultilayer_objsp")))
                {
                    tl.material.textures_color = {BlendMapTexture{
                        .texture_descriptor = {
                            .color = {
                                .filename = material->txDiffuse,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                            .specular = {},
                            .normal = {
                                .filename = material->txNormal,
                                .color_mode = ColorMode::RGB,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level}},
                        .weight = material->magicMult.value_or_default(),
                        .role = BlendMapRole::DETAIL_BASE,
                        .reweight_mode = BlendMapReweightMode::DISABLED}};
                    for (uint32_t i = 0; i < 4; ++i) {
                        if (material->txDetail4(i)->empty()) // ||
                            // (material->mult(i)->value_or_default() == 0.f))
                        {
                            continue;
                        }
                        tl.material.textures_color.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = {
                                    .filename = material->txMask,
                                    .color_mode = ColorMode::RGBA,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                                .specular = {},
                                .normal = {}},
                            .min_detail_weight = 0.01f,
                            .role = BlendMapRole::DETAIL_MASK_R + i,
                            .reduction = BlendMapReductionOperation::TIMES});
                        tl.material.textures_color.push_back(BlendMapTexture{
                            .texture_descriptor = {
                                .color = {
                                    .filename = material->txDetail4(i),
                                    .color_mode = ColorMode::RGB,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                                .specular = {},
                                .normal = {}},
                            .scale = material->mult(i).value_or_default(),
                            .weight = 0.f,
                            .role = BlendMapRole::DETAIL_COLOR,
                            .uv_source = any(attrs & MetaAttributes::ATTR_VERTICAL)
                                ? BlendMapUvSource::VERTICAL0
                                : BlendMapUvSource::HORIZONTAL});
                    }
                } else {
                    if (!material->txDiffuse->empty()) {
                        tl.material.textures_color = {BlendMapTexture{
                            .texture_descriptor = {
                                .color = {
                                    .filename = material->txDiffuse,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level},
                                .specular = {},
                                .normal = {
                                    .filename = material->txNormal,
                                    .color_mode = ColorMode::RGB,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level}}}};
                    }
                }
            }
            for (const auto& tri : node.triangles) {
                tl.draw_triangle_with_normals(
                    node.position(tri(0)).template casted<TPos>(),
                    node.position(tri(1)).template casted<TPos>(),
                    node.position(tri(2)).template casted<TPos>(),
                    node.normal(tri(0)),
                    node.normal(tri(1)),
                    node.normal(tri(2)),
                    Colors::WHITE,                              // c00
                    Colors::WHITE,                              // c10
                    Colors::WHITE,                              // c01
                    node.uv(tri(0)),
                    node.uv(tri(1)),
                    node.uv(tri(2)),
                    std::nullopt,                               // interiormap_uvmap
                    {},                                         // b00
                    {},                                         // b10
                    {},                                         // b01
                    cfg.triangle_tangent_error_behavior);
            }
            if (!any(tl.morphology.physics_material & (PhysicsMaterial::ATTR_VISIBLE | PhysicsMaterial::ATTR_COLLIDE))) {
                lwarn() << "Skipping node \"" << node.name << "\" because it is neither visible nor collidable";
            } else if (show_only_collidables) {
                if (any(tl.morphology.physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
                    tl.morphology.physics_material |= PhysicsMaterial::ATTR_VISIBLE;
                    tl.material.textures_color.clear();
                    tl.material.textures_alpha.clear();
                    tl.material.shading.ambient *= 0.3f;
                    tl.material.shading.specular *= 0.3f;
                    tl.material.shading.diffuse *= 0.3f;
                    result.push_back(tl.triangle_array());
                }
            } else {
                result.push_back(tl.triangle_array());
            }
        }
    };
    {
        auto settings_json_filename =
            fs::path{filename}.parent_path() /
            fs::path{"settings.json"};
        if (path_exists(settings_json_filename)) {
            auto f = create_ifstream(settings_json_filename);
            if (f->fail()) {
                THROW_OR_ABORT("Could not open \"" + settings_json_filename.string() + '"');
            }
            nlohmann::json j;
            *f >> j;
            if (f->fail()) {
                THROW_OR_ABORT("Could not read from file \"" + settings_json_filename.string() + '"');
            }
            settings_json = j.get<SettingsJson>();
        }
    }
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
                lit_mult = safe_stof(*gm);
            }
            if (auto gm = ext_config.try_get("LIGHTING", "SPECULAR_MULT");
                gm.has_value())
            {
                specular_mult = safe_stof(*gm);
            }
            if (auto gm = ext_config.try_get("GRASS_FX", "GRASS_MATERIALS");
                gm.has_value())
            {
                grass_materials = string_to_set(*gm, re);
            }
            if (auto gm = ext_config.try_get("GRASS_FX", "OCCLUDING_MESHES");
                gm.has_value())
            {
                occluding_meshes = string_to_set(*gm, re);
            }
            if (auto gm = ext_config.try_get("GRASS_FX", "TEXTURE_GRID");
                gm.has_value())
            {
                texture_grid = string_to_vector(*gm, re, safe_stou);
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
    {
        std::unordered_map<VariableAndHash<std::string>, ColorMode> color_modes;
        for (const auto& cva : result) {
            cva->material.compute_color_mode();
            for (const auto& t : cva->material.textures_color) {
                auto cmax = [&color_modes](auto& colormap){
                    color_modes[colormap.filename] |= colormap.color_mode;
                    };
                cmax(t.texture_descriptor.color);
                cmax(t.texture_descriptor.specular);
                cmax(t.texture_descriptor.normal);
            }
        }
        for (const auto& cva : result) {
            for (auto& t : cva->material.textures_color) {
                t.texture_descriptor.color.color_mode = color_modes.at(t.texture_descriptor.color.filename);
                t.texture_descriptor.specular.color_mode = color_modes.at(t.texture_descriptor.specular.filename);
                t.texture_descriptor.normal.color_mode = color_modes.at(t.texture_descriptor.normal.filename);
            }
        }
        for (const auto& cva : result) {
            auto register_colormap = [&](const ColormapWithModifiers& cm) {
                if (dds_resources != nullptr) {
                    auto n = textures.extract(cm.filename);
                    if (!n.empty()) {
                        dds_resources->add_texture(cm, std::move(n.mapped().data), FlipMode::VERTICAL, TextureAlreadyExistsBehavior::WARN);
                    }
                }
                };
            for (auto& t : cva->material.textures_color) {
                t.texture_descriptor.color.compute_hash();
                t.texture_descriptor.specular.compute_hash();
                t.texture_descriptor.normal.compute_hash();
                if (!t.texture_descriptor.color.filename->empty()) {
                    register_colormap(t.texture_descriptor.color);
                }
                if (!t.texture_descriptor.specular.filename->empty()) {
                    register_colormap(t.texture_descriptor.specular);
                }
                if (!t.texture_descriptor.normal.filename->empty()) {
                    register_colormap(t.texture_descriptor.normal);
                }
            }
        }
    }
    // if (race_logic != nullptr) {
    //     race_logic->set_circularity(raceway_is_circular);
    // }
    auto trafo = VertexTransformation<TPos>{
        cfg.position,
        cfg.rotation,
        cfg.scale };
    for (auto& l : result) {
        for (auto& t : l->triangles) {
            for (auto& v : t.flat_iterable()) {
                trafo.transform_inplace(
                    v,
                    NormalVectorErrorBehavior::ZERO,
                    TriangleTangentErrorBehavior::ZERO);
            }
        }
    }
    ambient_occlusion_by_curvature(result, cfg.laplace_ao_strength);
    return result;
}

template std::list<std::shared_ptr<ColoredVertexArray<float>>> Mlib::load_kn5_array<float>(
    const std::string& file_or_directory,
    const LoadMeshConfig<float>&,
    IDdsResources*,
    IRaceLogic*);
template std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> Mlib::load_kn5_array<CompressedScenePos>(
    const std::string& file_or_directory,
    const LoadMeshConfig<CompressedScenePos>&,
    IDdsResources*,
    IRaceLogic*);
