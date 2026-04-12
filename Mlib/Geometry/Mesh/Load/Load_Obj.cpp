#include "Load_Obj.hpp"
#include <Mlib/Compression/Compressed_File.hpp>
#include <Mlib/Geometry/Mesh/Ambient_Occlusion_By_Curvature.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Material.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Static_Face_Lighting.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Geometry/Vertex_Transformation.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Os/Utf8_Path.hpp>
#include <Mlib/Os/Weakly_Canonical_Preserve_Symlinks.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Strings/String_View_To_Scene_Pos.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <vector>

using namespace Mlib;
using namespace Mlib::TemplateRegex;
template <class TPos>
struct ColoredVertexX {
    FixedArray<TPos, 3> position;
    FixedArray<float, 3> color;
};

bool contains_tag(const std::string& name, const std::string& tag) {
    static std::map<std::string, Mlib::re::cregex> regexes;
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
    using Triangle = FixedArray<TPos, 3, 3>;

    std::string prefix = Utf8Path{ filename }.stem().string() + "/";
    std::map<std::string, ObjMaterial> mtllib;
    Utf8Path mtllib_path;
    std::vector<ColoredVertexX<TPos>> obj_vertices;
    UUVector<FixedArray<float, 2>> obj_uvs;
    UUVector<FixedArray<float, 3>> obj_normals;
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    TriangleList<TPos> tl{
        filename,
        Material{
            .blend_mode = cfg.blend_mode,
            .textures_color = cfg.textures,
            .period_world = cfg.period_world,
            .reflection_map = cfg.reflection_map,
            .occluded_pass = cfg.occluded_pass,
            .occluder_pass = cfg.occluder_pass,
            .alpha_distances = make_orderable(cfg.alpha_distances),
            .aggregate_mode = cfg.aggregate_mode,
            .transformation_mode = cfg.transformation_mode,
            .billboard_atlas_instances = cfg.billboard_atlas_instances,
            .cull_faces = cfg.cull_faces_default,
            .shading = cfg.shading,
            .dynamically_lighted = cfg.dynamically_lighted},
        Morphology{
            .physics_material = cfg.physics_material,
            .center_distances2 = SquaredStepDistances::from_distances(cfg.center_distances),
            .max_triangle_distance = cfg.max_triangle_distance,
            },
        ModifierBacklog{}};
    tl.meta.material.compute_color_mode();
    StaticFaceLighting sfl;

    auto compressed_file = CompressedFile{filename};
    auto ifs_p = compressed_file.decompressed_ifstream();
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        throw std::runtime_error("Could not open OBJ file \"" + filename + '"');
    }

    static const auto sl = chr('/');
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
        chr('v'), sp,
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
        chr('l'), sp,
        dp, sp,
        dp, ss, eof);
    static const auto face3_reg = seq(
        chr('f'), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), sp,
        dp, opt(seq(sl, ds, opt(seq(sl, dp)))), ss, eof);
    static const auto face4_reg = seq(
        chr('f'), sp,
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
    static const auto comment_reg = chr('#');
    static const auto mtllib_reg = seq(str("mtllib"), sp, Dotp);
    static const auto usemtl_reg = seq(str("usemtl"), sp, Dotp);
    static const auto object_reg = seq(chr('o'), sp, Dots);
    static const auto group_reg = seq(chr('g'), sp, Dots);
    static const auto smooth_shading_reg = seq(chr('s'), sp);

    ObjMaterial current_mtl;
    bool cfg_has_alpha_texture = false;
    for (const auto& t : cfg.textures) {
        if (any(t.texture_descriptor.color.color_mode & ColorMode::RGBA)) {
            cfg_has_alpha_texture = true;
            break;
        }
    }
    bool has_alpha_texture = cfg_has_alpha_texture;

    std::string line;
    while(std::getline(ifs, line)) {
        try {
            if (line.length() > 0 && line[line.length() - 1] == '\r') {
                line = line.substr(0, line.length() - 1);
            }
            if (line.length() == 0) {
                continue;
            }
            if (SMatch<8> match; regex_match(line, match, vertex_reg)) {
                float a = match[7].matched() ? safe_stof(match[7].str()) : 1;
                if (a != 1) {
                    throw std::runtime_error("vertex a != 1");
                }
                obj_vertices.push_back({
                    .position = {
                        safe_sto<TPos>(match[1].str()),
                        safe_sto<TPos>(match[2].str()),
                        safe_sto<TPos>(match[3].str())},
                    .color = {
                        match[4].matched() ? safe_stof(match[4].str()) : 1.f,
                        match[5].matched() ? safe_stof(match[5].str()) : 1.f,
                        match[6].matched() ? safe_stof(match[6].str()) : 1.f}});
            } else if (SMatch<3> match; regex_match(line, match, vertex_uv_texture_reg)) {
                FixedArray<float, 2> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str())};
                obj_uvs.emplace_back(n);
            } else if (SMatch<4> match; regex_match(line, match, vertex_uvw_texture_reg)) {
                FixedArray<float, 2> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str())};
                // assert_true(safe_stof(match[3].str()) == 0);
                obj_uvs.emplace_back(n);
            } else if (SMatch<4> match; regex_match(line, match, vertex_normal_reg)) {
                FixedArray<float, 3> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())};
                obj_normals.emplace_back(n);
            } else if (SMatch<3> match; regex_match(line, match, line_reg)) {
                // FixedArray<size_t, 3> vertex_ids{
                //     safe_stoz(match[1].str()),
                //     safe_stoz(match[2].str())};
                // do nothing
            } else if (SMatch<10> match; regex_match(line, match, face3_reg)) {
                FixedArray<size_t, 3> vertex_ids{
                    safe_stoz(match[1].str()),
                    safe_stoz(match[4].str()),
                    safe_stoz(match[7].str())};
                FixedArray<size_t, 3> uv_ids{
                    (match[2].str() != "") ? safe_stoz(match[2].str()) : SIZE_MAX,
                    (match[5].str() != "") ? safe_stoz(match[5].str()) : SIZE_MAX,
                    (match[8].str() != "") ? safe_stoz(match[8].str()) : SIZE_MAX };
                assert_true(all(vertex_ids > size_t(0)));
                assert_true(all(uv_ids > size_t(0)));
                FixedArray<float, 3> n0 = uninitialized;
                FixedArray<float, 3> n1 = uninitialized;
                FixedArray<float, 3> n2 = uninitialized;
                if (match[3].str().empty() && match[6].str().empty() && match[9].str().empty()) {
                    auto n = triangle_normal(funpack(Triangle{
                        obj_vertices.at(vertex_ids(0) - 1).position,
                        obj_vertices.at(vertex_ids(1) - 1).position,
                        obj_vertices.at(vertex_ids(2) - 1).position}),
                        NormalVectorErrorBehavior::WARN).template casted<float>();
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
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v0.color : sfl.get_color(current_mtl.diffuse, n0)),
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v1.color : sfl.get_color(current_mtl.diffuse, n1)),
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v2.color : sfl.get_color(current_mtl.diffuse, n2)),
                    (uv_ids(0) != SIZE_MAX) ? obj_uvs.at(uv_ids(0) - 1) : FixedArray<float, 2>{0.f, 0.f},
                    (uv_ids(1) != SIZE_MAX) ? obj_uvs.at(uv_ids(1) - 1) : FixedArray<float, 2>{1.f, 0.f},
                    (uv_ids(2) != SIZE_MAX) ? obj_uvs.at(uv_ids(2) - 1) : FixedArray<float, 2>{0.f, 1.f},
                    std::nullopt,
                    {},
                    {},
                    {},
                    cfg.triangle_tangent_error_behavior);
            } else if (SMatch<13> match; regex_match(line, match, face4_reg)) {
                FixedArray<size_t, 4> vertex_ids{
                    safe_stoz(match[1].str()),
                    safe_stoz(match[4].str()),
                    safe_stoz(match[7].str()),
                    safe_stoz(match[10].str())};
                FixedArray<size_t, 4> uv_ids{
                    (match[2].str() != "") ? safe_stoz(match[2].str()) : SIZE_MAX,
                    (match[5].str() != "") ? safe_stoz(match[5].str()) : SIZE_MAX,
                    (match[8].str() != "") ? safe_stoz(match[8].str()) : SIZE_MAX,
                    (match[11].str() != "") ? safe_stoz(match[11].str()) : SIZE_MAX };
                assert_true(all(vertex_ids > size_t(0)));
                assert_true(all(uv_ids > size_t(0)));
                FixedArray<float, 3> n0 = uninitialized;
                FixedArray<float, 3> n1 = uninitialized;
                FixedArray<float, 3> n2 = uninitialized;
                FixedArray<float, 3> n3 = uninitialized;
                if (match[3].str().empty() && match[6].str().empty() && match[9].str().empty()) {
                    auto n = triangle_normal(funpack(Triangle{
                        obj_vertices.at(vertex_ids(0) - 1).position,
                        obj_vertices.at(vertex_ids(1) - 1).position,
                        obj_vertices.at(vertex_ids(2) - 1).position}),
                        NormalVectorErrorBehavior::WARN).template casted<float>();
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
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v0.color : sfl.get_color(current_mtl.diffuse, n0)),
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v1.color : sfl.get_color(current_mtl.diffuse, n1)),
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v2.color : sfl.get_color(current_mtl.diffuse, n2)),
                    Colors::from_rgb(has_alpha_texture || !cfg.apply_static_lighting ? v3.color : sfl.get_color(current_mtl.diffuse, n3)),
                    (uv_ids(0) != SIZE_MAX) ? obj_uvs.at(uv_ids(0) - 1) : FixedArray<float, 2>{0.f, 0.f},
                    (uv_ids(1) != SIZE_MAX) ? obj_uvs.at(uv_ids(1) - 1) : FixedArray<float, 2>{1.f, 0.f},
                    (uv_ids(2) != SIZE_MAX) ? obj_uvs.at(uv_ids(2) - 1) : FixedArray<float, 2>{1.f, 1.f},
                    (uv_ids(3) != SIZE_MAX) ? obj_uvs.at(uv_ids(3) - 1) : FixedArray<float, 2>{0.f, 1.f},
                    std::nullopt,
                    {},
                    {},
                    {},
                    {},
                    cfg.triangle_tangent_error_behavior,
                    cfg.rectangle_triangulation_mode,
                    cfg.delaunay_error_behavior);
            } else if (SMatch<1> match; regex_match(line, match, comment_reg)) {
                // do nothing
            } else if (SMatch<2> match; regex_match(line, match, object_reg) ||
                                        regex_match(line, match, group_reg))
            {
                if (!tl.triangles.empty()) {
                    result.push_back(tl.triangle_array());
                    tl.triangles.clear();
                }
                tl.meta.name = { prefix, std::string{ match[1].str() } };
            } else if (SMatch<2> match; regex_match(line, match, mtllib_reg)) {
                mtllib_path = compressed_file.sibling(match[1].str()).path();
                mtllib = load_mtllib(mtllib_path, cfg.werror);
            } else if (SMatch<2> match; regex_match(line, match, usemtl_reg)) {
                auto material_name = std::string{ match[1].str() };
                current_mtl = mtllib.at(material_name);
                TextureDescriptor td;
                auto gen_texture_path = [&mtllib_path](const Utf8Path& child){
                    if (child.empty()) {
                        return FPath{};
                    }
                    auto p = mtllib_path.parent_path();
                    return FPath::from_local_path(p.empty() ? child : weakly_canonical_preserve_symlinks(p / child));
                };
                if (!current_mtl.diffuse_texture.empty()) {
                    td.color = ColormapWithModifiers{
                        .filename = gen_texture_path(current_mtl.diffuse_texture),
                        .chrominance = gen_texture_path(current_mtl.diffuse_chrominance_texture),
                        .desaturate = cfg.desaturate,
                        .desaturation_exponent = cfg.desaturation_exponent,
                        .histogram = cfg.histogram,
                        .lighten = make_orderable(cfg.lighten),
                        .mipmap_mode = cfg.mipmap_mode,
                        .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level }.compute_hash();
                }
                if (!current_mtl.specular_texture.empty()) {
                    td.specular = ColormapWithModifiers{
                        .filename = gen_texture_path(current_mtl.specular_texture),
                        .color_mode = ColorMode::RGB,
                        .mipmap_mode = cfg.mipmap_mode,
                        .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level}.compute_hash();
                }
                if (!current_mtl.bump_texture.empty()) {
                    td.normal = ColormapWithModifiers{
                        .filename = gen_texture_path(current_mtl.bump_texture),
                        .color_mode = ColorMode::RGB,
                        .mipmap_mode = cfg.mipmap_mode,
                        .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level}.compute_hash();
                }
                if (!td.color.filename.empty() || !td.specular.filename.empty() || !td.normal.filename.empty()) {
                    tl.meta.material.textures_color = { {.texture_descriptor = td } };
                    has_alpha_texture = current_mtl.has_alpha_texture;
                } else {
                    tl.meta.material.textures_color = cfg.textures;
                    has_alpha_texture = cfg_has_alpha_texture;
                }
                if (has_alpha_texture || (current_mtl.alpha != 1.f)) {
                    tl.meta.material.blend_mode = cfg.blend_mode;
                    tl.meta.material.cull_faces = cfg.cull_faces_alpha;
                } else {
                    tl.meta.material.blend_mode = BlendMode::OFF;
                    tl.meta.material.cull_faces = cfg.cull_faces_default && !contains_tag(material_name, "NoCullFaces");
                }
                if (contains_tag(material_name, "OccludedTypeColor")) {
                    tl.meta.material.occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE;
                } else {
                    tl.meta.material.occluded_pass = cfg.occluded_pass;
                }
                if (contains_tag(material_name, "OccluderTypeWhite")) {
                    tl.meta.material.occluder_pass = ExternalRenderPassType::NONE;
                } else {
                    tl.meta.material.occluder_pass = cfg.occluder_pass;
                }
                tl.meta.material.shading.emissive = current_mtl.emissive;
                tl.meta.material.shading.ambient = current_mtl.ambient;
                tl.meta.material.shading.diffuse = current_mtl.diffuse;
                tl.meta.material.shading.specular = current_mtl.specular;
                tl.meta.material.shading.specular_exponent = current_mtl.specular_exponent;
                tl.meta.material.alpha = current_mtl.alpha;
                tl.meta.material.compute_color_mode();
            } else if (SMatch<1> match; regex_match(line, match, smooth_shading_reg)) {
                // do nothing
            } else {
                if (cfg.werror) {
                    throw std::runtime_error("Could not parse line");
                } else {
                    lerr() << "WARNING: Could not parse line: " + line;
                }
            }
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error in line: \"" + line + "\", " + e.what());
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Error in line: \"" + line + "\", " + e.what());
        }
    }
    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file " + filename);
    }
    result.push_back(tl.triangle_array());
    VertexTransformation<TPos> vtrafo{
        cfg.position,
        cfg.rotation,
        cfg.scale };
    for (auto& l : result) {
        for (auto& t : l->triangles) {
            for (auto& v : t.flat_iterable()) {
                vtrafo.transform_inplace(
                    v,
                    NormalVectorErrorBehavior::ZERO,
                    TriangleTangentErrorBehavior::ZERO);
            }
        }
        for (auto& t : l->quads) {
            for (auto& v : t.flat_iterable()) {
                vtrafo.transform_inplace(
                    v,
                    NormalVectorErrorBehavior::ZERO,
                    TriangleTangentErrorBehavior::ZERO);
            }
        }
        l->meta.material.shading.emissive *= cfg.emissive_factor;
        l->meta.material.shading.ambient *= cfg.ambient_factor;
        l->meta.material.shading.diffuse *= cfg.diffuse_factor;
        l->meta.material.shading.specular *= cfg.specular_factor;
    }
    ambient_occlusion_by_curvature(result, cfg.laplace_ao_strength);
    return result;
}

template std::list<std::shared_ptr<ColoredVertexArray<float>>> Mlib::load_obj<float>(
    const std::string& filename, const LoadMeshConfig<float>& cfg);
template std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> Mlib::load_obj<CompressedScenePos>(
    const std::string& filename, const LoadMeshConfig<CompressedScenePos>& cfg);
