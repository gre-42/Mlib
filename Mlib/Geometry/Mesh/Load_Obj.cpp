#include "Load_Obj.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load_Material.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Static_Face_Lightning.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <filesystem>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

using namespace Mlib;

struct ColoredVertexX {
    FixedArray<float, 3> position;
    FixedArray<float, 3> color;
};

std::list<std::shared_ptr<ColoredVertexArray>> Mlib::load_obj(
    const std::string& filename,
    const LoadMeshConfig& cfg)
{
    std::map<std::string, ObjMaterial> mtllib;
    std::vector<ColoredVertexX> obj_vertices;
    std::vector<FixedArray<float, 2>> obj_uvs;
    std::vector<FixedArray<float, 3>> obj_normals;
    std::list<std::shared_ptr<ColoredVertexArray>> result;
    TriangleList tl{
        filename,
        Material{
            .occluded_type = cfg.occluded_type,
            .occluder_type = cfg.occluder_type,
            .occluded_by_black = cfg.occluded_by_black,
            .alpha_distances = OrderableFixedArray{cfg.alpha_distances},
            .aggregate_mode = cfg.aggregate_mode,
            .transformation_mode = cfg.transformation_mode,
            .is_small = cfg.is_small}};
    StaticFaceLightning sfl;

    std::ifstream ifs{filename};

    static const DECLARE_REGEX(vertex_reg, "^v +(\\S+) (\\S+) (\\S+)(?: (\\S+) (\\S+) (\\S+) (\\S+))?$");
    static const DECLARE_REGEX(vertex_normal_reg, "^vn +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(line_reg, "^l +"
                              "(\\d+) "
                              "(\\d+) *$");
    static const DECLARE_REGEX(face3_reg, "^f +"
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? *$");
    static const DECLARE_REGEX(face4_reg, "^f +"
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? "
                              "(\\d+)(?:/(\\d*)(?:/(\\d+))?)? *$");
    static const DECLARE_REGEX(vertex_uv_texture_reg, "^vt +(\\S+) (\\S+)$");
    static const DECLARE_REGEX(vertex_uvw_texture_reg, "^vt +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(comment_reg, "^#.*$");
    static const DECLARE_REGEX(mtllib_reg, "^mtllib (.+)$");
    static const DECLARE_REGEX(usemtl_reg, "^usemtl (.+)$");
    static const DECLARE_REGEX(object_reg, "^o .*$");
    static const DECLARE_REGEX(group_reg, "^g(?: (.*))?$");
    static const DECLARE_REGEX(smooth_shading_reg, "^s .*$");

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
            Mlib::re::smatch match;
            if (Mlib::re::regex_match(line, match, vertex_reg)) {
                float a = match[7].str().empty() ? 1 : safe_stof(match[7].str());
                if (a != 1) {
                    throw std::runtime_error("vertex a != 1");
                }
                obj_vertices.push_back({
                    .position = {
                        safe_stof(match[1].str()),
                        safe_stof(match[2].str()),
                        safe_stof(match[3].str())},
                    .color = {
                        match[4].str().empty() ? 1 : safe_stof(match[4].str()),
                        match[5].str().empty() ? 1 : safe_stof(match[5].str()),
                        match[6].str().empty() ? 1 : safe_stof(match[6].str())}});
            } else if (Mlib::re::regex_match(line, match, vertex_uv_texture_reg)) {
                FixedArray<float, 2> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str())};
                obj_uvs.push_back(n);
            } else if (Mlib::re::regex_match(line, match, vertex_uvw_texture_reg)) {
                FixedArray<float, 2> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str())};
                // assert_true(safe_stof(match[3].str()) == 0);
                obj_uvs.push_back(n);
            } else if (Mlib::re::regex_match(line, match, vertex_normal_reg)) {
                FixedArray<float, 3> n{
                    safe_stof(match[1].str()),
                    safe_stof(match[2].str()),
                    safe_stof(match[3].str())};
                obj_normals.push_back(n);
            } else if (Mlib::re::regex_match(line, match, line_reg)) {
                // FixedArray<size_t, 3> vertex_ids{
                //     safe_stoz(match[1].str()),
                //     safe_stoz(match[2].str())};
                // do nothing
            } else if (Mlib::re::regex_match(line, match, face3_reg)) {
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
                if (match[3].str() + match[6].str() + match[9].str() == "") {
                    auto n = triangle_normal({
                        obj_vertices.at(vertex_ids(0) - 1).position,
                        obj_vertices.at(vertex_ids(1) - 1).position,
                        obj_vertices.at(vertex_ids(2) - 1).position});
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
                const ColoredVertexX& v0 = obj_vertices.at(vertex_ids(0) - 1);
                const ColoredVertexX& v1 = obj_vertices.at(vertex_ids(1) - 1);
                const ColoredVertexX& v2 = obj_vertices.at(vertex_ids(2) - 1);
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
                    (uv_ids(2) != SIZE_MAX) ? obj_uvs.at(uv_ids(2) - 1) : FixedArray<float, 2>{0.f, 1.f});
            } else if (Mlib::re::regex_match(line, match, face4_reg)) {
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
                if (match[3].str() + match[6].str() + match[9].str() == "") {
                    auto n = triangle_normal({
                        obj_vertices.at(vertex_ids(0) - 1).position,
                        obj_vertices.at(vertex_ids(1) - 1).position,
                        obj_vertices.at(vertex_ids(2) - 1).position});
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
                const ColoredVertexX& v0 = obj_vertices.at(vertex_ids(0) - 1);
                const ColoredVertexX& v1 = obj_vertices.at(vertex_ids(1) - 1);
                const ColoredVertexX& v2 = obj_vertices.at(vertex_ids(2) - 1);
                const ColoredVertexX& v3 = obj_vertices.at(vertex_ids(3) - 1);
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
                    (uv_ids(3) != SIZE_MAX) ? obj_uvs.at(uv_ids(3) - 1) : FixedArray<float, 2>{0.f, 1.f});
            } else if (Mlib::re::regex_match(line, match, comment_reg)) {
                // do nothing
            } else if (Mlib::re::regex_match(line, match, object_reg)) {
                // do nothing
            } else if (Mlib::re::regex_match(line, match, group_reg)) {
                if (!tl.triangles_.empty()) {
                    result.push_back(tl.triangle_array());
                    tl.triangles_.clear();
                    tl.material_.textures.clear();
                }
                tl.name_ = match[1].str();
            } else if (Mlib::re::regex_match(line, match, mtllib_reg)) {
                std::string p = fs::path(filename).parent_path().string();
                mtllib = load_mtllib(p == "" ? match[1].str() : p + "/" + match[1].str(), cfg.werror);
            } else if (Mlib::re::regex_match(line, match, usemtl_reg)) {
                current_mtl = mtllib.at(match[1].str());
                TextureDescriptor td;
                if (!current_mtl.color_texture.empty()) {
                    fs::path p = fs::path(filename).parent_path();
                    td.color = p.empty() ? current_mtl.color_texture : fs::weakly_canonical(p / current_mtl.color_texture).string();
                }
                if (!current_mtl.bump_texture.empty()) {
                    fs::path p = fs::path(filename).parent_path();
                    td.normal = p.empty() ? current_mtl.bump_texture : fs::weakly_canonical(p / current_mtl.bump_texture).string();
                }
                if (!td.color.empty() || !td.normal.empty()) {
                    tl.material_.textures = { {.texture_descriptor = td } };
                }
                if (current_mtl.has_alpha_texture) {
                    tl.material_.blend_mode = cfg.blend_mode;
                    tl.material_.cull_faces = cfg.cull_faces;
                } else {
                    tl.material_.blend_mode = BlendMode::OFF;
                    tl.material_.cull_faces = true;
                }
                tl.material_.ambience = current_mtl.ambience;
                tl.material_.diffusivity = current_mtl.diffusivity;
                tl.material_.specularity = current_mtl.specularity;
                tl.material_.compute_color_mode();
            } else if (Mlib::re::regex_match(line, match, smooth_shading_reg)) {
                // do nothing
            } else {
                if (cfg.werror) {
                    throw std::runtime_error("Could not parse line");
                } else {
                    std::cerr << "WARNING: Could not parse line: " + line << std::endl;
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
    FixedArray<float, 3, 3> rotation_matrix{tait_bryan_angles_2_matrix(cfg.rotation)};
    for (auto& l : result) {
        for (auto& t : l->triangles) {
            for (auto& v : t.flat_iterable()) {
                v.position *= cfg.scale;
                v.position = dot1d(rotation_matrix, v.position);
                v.position += cfg.position;
                v.normal = dot1d(rotation_matrix, v.normal);
            }
        }
    }
    return result;
}
