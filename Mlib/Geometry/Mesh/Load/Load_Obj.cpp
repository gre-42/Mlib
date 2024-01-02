#include "Load_Obj.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Mesh/Ambient_Occlusion_By_Curvature.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Material.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Static_Face_Lighting.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

using namespace Mlib;
using namespace Mlib::TemplateRegex;
template <class TPos>
struct ColoredVertexX {
    FixedArray<TPos, 3> position;
    FixedArray<float, 3> color;
};

bool contains_tag(const std::string& name, const std::string& tag) {
    static std::map<std::string, Mlib::regex> regexes;
    if (!regexes.contains(tag)) {
        regexes.insert({tag, Mlib::compile_regex("\\b" + tag + "(?:\\b|_)")});
    }
    return Mlib::re::regex_search(name, regexes.at(tag));
}

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> Mlib::load_obj(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg)
{
    std::map<std::string, ObjMaterial> mtllib;
    std::vector<ColoredVertexX<TPos>> obj_vertices;
    std::vector<FixedArray<float, 2>> obj_uvs;
    std::vector<FixedArray<float, 3>> obj_normals;
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    TriangleList<TPos> tl{
        filename,
        Material{
            .textures_color = cfg.textures,
            .period_world = cfg.period_world,
            .reflection_map = cfg.reflection_map,
            .occluded_pass = cfg.occluded_pass,
            .occluder_pass = cfg.occluder_pass,
            .alpha_distances = OrderableFixedArray{ cfg.alpha_distances },
            .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
            .aggregate_mode = cfg.aggregate_mode,
            .transformation_mode = cfg.transformation_mode,
            .center_distances = cfg.center_distances,
            .max_triangle_distance = cfg.max_triangle_distance,
            .cull_faces = cfg.cull_faces_default,
            .fresnel_ambience = OrderableFixedArray{ cfg.fresnel_ambience },
            .fresnel_ambience_exponent = cfg.fresnel_ambience_exponent},
        cfg.physics_material};
    tl.material.compute_color_mode();
    StaticFaceLighting sfl;

    auto ifs_p = create_ifstream(filename);
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        THROW_OR_ABORT("Could not open OBJ file \"" + filename + '"');
    }

    static const auto sl = str("/");
    static const auto ss = star(space);
    static const auto sp = plus(space);
    static const auto Sp = group(plus(no_space));
    static const auto ds = group(star(digit));
    static const auto dp = group(plus(digit));
    static const auto Dots = group(star(adot));
    static const auto Dotp = group(plus(adot));
    // static const DECLARE_REGEX(vertex_reg, "^v +(\\S+) (\\S+) (\\S+)(?: (\\S+) (\\S+) (\\S+) (\\S+))?$");
    // static const DECLARE_REGEX(vertex_normal_reg, "^vn +(\\S+) (\\S+) (\\S+)$");
    // static const DECLARE_REGEX(line_reg, "^l +"
    //                           "(\\d+) "
    //                           "(\\d+) *$");
    // static const DECLARE_REGEX(face3_reg, "^f +"
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? *$");
    // static const DECLARE_REGEX(face4_reg, "^f +"
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
    //                           "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? *$");
    // static const DECLARE_REGEX(vertex_uv_texture_reg, "^vt +(\\S+) (\\S+)$");
    // static const DECLARE_REGEX(vertex_uvw_texture_reg, "^vt +(\\S+) (\\S+) (\\S+)$");
    // static const DECLARE_REGEX(comment_reg, "^#.*$");
    // static const DECLARE_REGEX(mtllib_reg, "^mtllib (.+)$");
    // static const DECLARE_REGEX(usemtl_reg, "^usemtl (.+)$");
    // static const DECLARE_REGEX(object_reg, "^o (.*)$");
    // static const DECLARE_REGEX(group_reg, "^g (.*)$");
    // static const DECLARE_REGEX(smooth_shading_reg, "^s .*$");
    static const auto vertex_reg = seq(
        str("v"), sp,
        Sp, sp,
        Sp, sp,
        Sp, opt(seq(sp, Sp, sp, Sp, sp, Sp, sp, Sp)),
        eof);
    static const auto vertex_normal_reg = seq(
        str("vn"), sp,
        Sp, sp,
        Sp, sp,
        Sp, eof);
    static const auto line_reg = seq(
        str("l"), sp,
        dp, sp,
        dp, ss, eof);
    static const auto face3_reg = seq(
        str("f"), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), ss, eof);
    static const auto face4_reg = seq(
        str("f"), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), ss, eof);
    static const auto vertex_uv_texture_reg = seq(
        str("vt"), sp,
        Sp, sp,
        Sp, eof);
    static const auto vertex_uvw_texture_reg = seq(
        str("vt"), sp,
        Sp, sp,
        Sp, sp,
        Sp, eof);
    static const auto comment_reg = str("#");
    static const auto mtllib_reg = seq(str("mtllib"), sp, Dotp);
    static const auto usemtl_reg = seq(str("usemtl"), sp, Dotp);
    static const auto object_reg = seq(str("o"), sp, Dots);
    static const auto group_reg = seq(str("g"), sp, Dots);
    static const auto smooth_shading_reg = seq(str("s"), sp);

    ObjMaterial current_mtl;

    std::string line;
    while(std::getline(ifs, line)) {
        try {
            if (line.length() > 0 && line[line.length() - 1] == '\r') {
                line = line.substr(0, line.length() - 1);
            }
            if (line.length() == 0) {
                continue;
            }
            SMatch match;
            if (regex_match(line, match, vertex_reg)) {
                float a = match[7].matched ? safe_stof(match[7].str()) : 1;
                if (a != 1) {
                    THROW_OR_ABORT("vertex a != 1");
                }
                obj_vertices.push_back({
                    .position = {
                        safe_stox<TPos>(match[1].str()),
                        safe_stox<TPos>(match[2].str()),
                        safe_stox<TPos>(match[3].str())},
                    .color = {
                        match[4].matched ? safe_stof(match[4].str()): 1.f,
                        match[5].matched ? safe_stof(match[5].str()): 1.f,
                        match[6].matched ? safe_stof(match[6].str()): 1.f}});
            } else if (regex_match(line, match, vertex_uv_texture_reg)) {
                FixedArray<float, 2> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str())};
                obj_uvs.push_back(n);
            } else if (regex_match(line, match, vertex_uvw_texture_reg)) {
                FixedArray<float, 2> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str())};
                // assert_true(safe_stof(match[3].str()) == 0);
                obj_uvs.push_back(n);
            } else if (regex_match(line, match, vertex_normal_reg)) {
                FixedArray<float, 3> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())};
                obj_normals.push_back(n);
            } else if (regex_match(line, match, line_reg)) {
                // FixedArray<size_t, 3> vertex_ids{
                //     safe_stoz(match[1].str()),
                //     safe_stoz(match[2].str())};
                // do nothing
            } else if (regex_match(line, match, face3_reg)) {
                FixedArray<size_t, 3> vertex_ids{
                    safe_stoz(match[1].str()),
                    safe_stoz(match[4].str()),
                    safe_stoz(match[7].str())};
                FixedArray<size_t, 3> uv_ids;
                uv_ids(0) = (match[2].str() != "") ? safe_stoz(match[2].str()) : SIZE_MAX;
                uv_ids(1) = (match[5].str() != "") ? safe_stoz(match[5].str()) : SIZE_MAX;
                uv_ids(2) = (match[8].str() != "") ? safe_stoz(match[8].str()) : SIZE_MAX;
                assert_true(all(vertex_ids > size_t(0)));
                assert_true(all(uv_ids > size_t(0)));
                FixedArray<float, 3> n0;
                FixedArray<float, 3> n1;
                FixedArray<float, 3> n2;
                if (match[3].str().empty() && match[6].str().empty() && match[9].str().empty()) {
                    auto n = triangle_normal<TPos>({
                        obj_vertices.at(vertex_ids(0) - 1).position,
                        obj_vertices.at(vertex_ids(1) - 1).position,
                        obj_vertices.at(vertex_ids(2) - 1).position},
                        TriangleNormalErrorBehavior::WARN) TEMPLATEV casted<float>();
                    n0 = n;
                    n1 = n;
                    n2 = n;
                } else {
                    FixedArray<size_t, 3> normal_ids{
                        safe_stoz(match[3].str()),
                        safe_stoz(match[6].str()),
                        safe_stoz(match[9].str())};
                    assert_true(all(normal_ids > size_t(0)));
                    n0 = obj_normals.at(normal_ids(0) - 1);
                    n1 = obj_normals.at(normal_ids(1) - 1);
                    n2 = obj_normals.at(normal_ids(2) - 1);
                }
                const ColoredVertexX<TPos>& v0 = obj_vertices.at(vertex_ids(0) - 1);
                const ColoredVertexX<TPos>& v1 = obj_vertices.at(vertex_ids(1) - 1);
                const ColoredVertexX<TPos>& v2 = obj_vertices.at(vertex_ids(2) - 1);
                tl.draw_triangle_with_normals(
                    v0.position,
                    v1.position,
                    v2.position,
                    n0,
                    n1,
                    n2,
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v0.color : sfl.get_color(current_mtl.diffusivity, n0),
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v1.color : sfl.get_color(current_mtl.diffusivity, n1),
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v2.color : sfl.get_color(current_mtl.diffusivity, n2),
                    (uv_ids(0) != SIZE_MAX) ? obj_uvs.at(uv_ids(0) - 1) : FixedArray<float, 2>{0.f, 0.f},
                    (uv_ids(1) != SIZE_MAX) ? obj_uvs.at(uv_ids(1) - 1) : FixedArray<float, 2>{1.f, 0.f},
                    (uv_ids(2) != SIZE_MAX) ? obj_uvs.at(uv_ids(2) - 1) : FixedArray<float, 2>{0.f, 1.f},
                    {},
                    {},
                    {},
                    cfg.triangle_tangent_error_behavior);
            } else if (regex_match(line, match, face4_reg)) {
                FixedArray<size_t, 4> vertex_ids{
                    safe_stoz(match[1].str()),
                    safe_stoz(match[4].str()),
                    safe_stoz(match[7].str()),
                    safe_stoz(match[10].str())};
                FixedArray<size_t, 4> uv_ids;
                uv_ids(0) = (match[2].str() != "") ? safe_stoz(match[2].str()) : SIZE_MAX;
                uv_ids(1) = (match[5].str() != "") ? safe_stoz(match[5].str()) : SIZE_MAX;
                uv_ids(2) = (match[8].str() != "") ? safe_stoz(match[8].str()) : SIZE_MAX;
                uv_ids(3) = (match[11].str() != "") ? safe_stoz(match[11].str()) : SIZE_MAX;
                assert_true(all(vertex_ids > size_t(0)));
                assert_true(all(uv_ids > size_t(0)));
                FixedArray<float, 3> n0;
                FixedArray<float, 3> n1;
                FixedArray<float, 3> n2;
                FixedArray<float, 3> n3;
                if (match[3].str().empty() && match[6].str().empty() && match[9].str().empty()) {
                    auto n = triangle_normal<TPos>({
                        obj_vertices.at(vertex_ids(0) - 1).position,
                        obj_vertices.at(vertex_ids(1) - 1).position,
                        obj_vertices.at(vertex_ids(2) - 1).position},
                        TriangleNormalErrorBehavior::WARN) TEMPLATEV casted<float>();
                    n0 = n;
                    n1 = n;
                    n2 = n;
                    n3 = n;
                } else {
                    FixedArray<size_t, 4> normal_ids{
                        safe_stoz(match[3].str()),
                        safe_stoz(match[6].str()),
                        safe_stoz(match[9].str()),
                        safe_stoz(match[12].str())};
                    assert_true(all(normal_ids > size_t(0)));
                    n0 = obj_normals.at(normal_ids(0) - 1);
                    n1 = obj_normals.at(normal_ids(1) - 1);
                    n2 = obj_normals.at(normal_ids(2) - 1);
                    n3 = obj_normals.at(normal_ids(3) - 1);
                }
                const ColoredVertexX<TPos>& v0 = obj_vertices.at(vertex_ids(0) - 1);
                const ColoredVertexX<TPos>& v1 = obj_vertices.at(vertex_ids(1) - 1);
                const ColoredVertexX<TPos>& v2 = obj_vertices.at(vertex_ids(2) - 1);
                const ColoredVertexX<TPos>& v3 = obj_vertices.at(vertex_ids(3) - 1);
                tl.draw_rectangle_with_normals(
                    v0.position,
                    v1.position,
                    v2.position,
                    v3.position,
                    n0,
                    n1,
                    n2,
                    n3,
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v0.color : sfl.get_color(current_mtl.diffusivity, n0),
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v1.color : sfl.get_color(current_mtl.diffusivity, n1),
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v2.color : sfl.get_color(current_mtl.diffusivity, n2),
                    current_mtl.has_alpha_texture || !cfg.apply_static_lighting ? v3.color : sfl.get_color(current_mtl.diffusivity, n3),
                    (uv_ids(0) != SIZE_MAX) ? obj_uvs.at(uv_ids(0) - 1) : FixedArray<float, 2>{0.f, 0.f},
                    (uv_ids(1) != SIZE_MAX) ? obj_uvs.at(uv_ids(1) - 1) : FixedArray<float, 2>{1.f, 0.f},
                    (uv_ids(2) != SIZE_MAX) ? obj_uvs.at(uv_ids(2) - 1) : FixedArray<float, 2>{1.f, 1.f},
                    (uv_ids(3) != SIZE_MAX) ? obj_uvs.at(uv_ids(3) - 1) : FixedArray<float, 2>{0.f, 1.f},
                    {},
                    {},
                    {},
                    {},
                    cfg.triangle_tangent_error_behavior);
            } else if (regex_match(line, match, comment_reg)) {
                // do nothing
            } else if (regex_match(line, match, object_reg) ||
                       regex_match(line, match, group_reg))
            {
                if (!tl.triangles.empty()) {
                    result.push_back(tl.triangle_array());
                    tl.triangles.clear();
                }
                tl.name = match[1].str();
            } else if (regex_match(line, match, mtllib_reg)) {
                std::string p = fs::path(filename).parent_path().string();
                mtllib = load_mtllib(p == "" ? std::string{match[1].str()} : p + "/" + std::string{match[1].str()}, cfg.werror);
            } else if (regex_match(line, match, usemtl_reg)) {
                auto material_name = std::string{match[1].str()};
                current_mtl = mtllib.at(material_name);
                TextureDescriptor td;
                if (!current_mtl.color_texture.empty()) {
                    fs::path p = fs::path(filename).parent_path();
                    td.color = {
                        .filename = p.empty() ? current_mtl.color_texture : fs::weakly_canonical(p / current_mtl.color_texture).string(),
                        .desaturate = cfg.desaturate,
                        .histogram = cfg.histogram,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level };
                }
                if (!current_mtl.specular_texture.empty()) {
                    fs::path p = fs::path(filename).parent_path();
                    td.specular = {
                        .filename = p.empty() ? current_mtl.specular_texture : fs::weakly_canonical(p / current_mtl.specular_texture).string(),
                        .color_mode = ColorMode::RGB,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level};
                }
                if (!current_mtl.bump_texture.empty()) {
                    fs::path p = fs::path(filename).parent_path();
                    td.normal = {
                        .filename = p.empty() ? current_mtl.bump_texture : fs::weakly_canonical(p / current_mtl.bump_texture).string(),
                        .color_mode = ColorMode::RGB,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level};
                }
                if (!td.color.filename.empty() || !td.specular.filename.empty() || !td.normal.filename.empty()) {
                    tl.material.textures_color = { {.texture_descriptor = td } };
                } else {
                    tl.material.textures_color = cfg.textures;
                }
                if (current_mtl.has_alpha_texture || (current_mtl.alpha != 1.f)) {
                    tl.material.blend_mode = cfg.blend_mode;
                    tl.material.cull_faces = cfg.cull_faces_alpha;
                } else {
                    tl.material.blend_mode = BlendMode::OFF;
                    tl.material.cull_faces = cfg.cull_faces_default && !contains_tag(material_name, "NoCullFaces");
                }
                if (contains_tag(material_name, "OccludedTypeColor")) {
                    tl.material.occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
                } else {
                    tl.material.occluded_pass = cfg.occluded_pass;
                }
                if (contains_tag(material_name, "OccluderTypeWhite")) {
                    tl.material.occluder_pass = ExternalRenderPassType::NONE;
                } else {
                    tl.material.occluder_pass = cfg.occluder_pass;
                }
                tl.material.emissivity = cfg.emissivity_factor * current_mtl.emissivity;
                tl.material.ambience = cfg.ambience_factor * current_mtl.ambience;
                tl.material.diffusivity = cfg.diffusivity_factor * current_mtl.diffusivity;
                tl.material.specularity = cfg.specularity_factor * current_mtl.specularity;
                tl.material.specular_exponent = current_mtl.specular_exponent;
                tl.material.alpha = current_mtl.alpha;
                tl.material.compute_color_mode();
            } else if (regex_match(line, match, smooth_shading_reg)) {
                // do nothing
            } else {
                if (cfg.werror) {
                    THROW_OR_ABORT("Could not parse line");
                } else {
                    std::cerr << "WARNING: Could not parse line: " + line << std::endl;
                }
            }
        } catch (const std::runtime_error& e) {
            THROW_OR_ABORT("Error in line: \"" + line + "\", " + e.what());
        } catch (const std::out_of_range& e) {
            THROW_OR_ABORT("Error in line: \"" + line + "\", " + e.what());
        }
    }
    if (!ifs.eof() && ifs.fail()) {
        THROW_OR_ABORT("Error reading from file " + filename);
    }
    result.push_back(tl.triangle_array());
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
template std::list<std::shared_ptr<ColoredVertexArray<float>>> load_obj<float>(const std::string& filename, const LoadMeshConfig<float>& cfg);
template std::list<std::shared_ptr<ColoredVertexArray<double>>> load_obj<double>(const std::string& filename, const LoadMeshConfig<double>& cfg);
}
