#include "Load_Mhx2.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

using namespace Mlib;

template <class TData, size_t tsize>
FixedArray<TData, tsize> get_fixed_array(const json& j) {
    auto v = j.get<std::vector<float>>();
    if (v.size() != tsize) {
        throw std::runtime_error("Unsupported dimensionality");
    }
    return FixedArray<float, tsize>{v};
}

template <size_t tsize>
std::vector<FixedArray<float, tsize>> load_vector(const json& j) {
    std::list<FixedArray<float, tsize>> vertex_list;
    for (const auto& vertex : j) {
        vertex_list.push_back(get_fixed_array<float, tsize>(vertex));
    }
    return std::vector<FixedArray<float, tsize>>{vertex_list.begin(), vertex_list.end()};
}

// template <class TValue>
// const auto& map_at(const std::map<std::string, TValue>& m, const std::string& n) {
//     auto it = m.find(n);
//     if (it == m.end()) {
//         throw std::runtime_error("Could not find key " + n + " in map");
//     }
//     return it->second;
// }

std::string gen_filename(const std::string& f, const std::string& texture_name) {
    if (!texture_name.empty()) {
        fs::path p = fs::path(f).parent_path();
        return p.empty() ? texture_name : fs::weakly_canonical(p / texture_name).string();
    } else {
        return "";
    }
}

std::list<std::shared_ptr<ColoredVertexArray>> Mlib::load_mhx2(
    const std::string& filename,
    bool is_small,
    BlendMode blend_mode,
    bool cull_faces,
    OccludedType occluded_type,
    OccluderType occluder_type,
    bool occluded_by_black,
    AggregateMode aggregate_mode,
    TransformationMode transformation_mode,
    bool werror)
{
    json j;
    std::ifstream f{filename};
    if (f.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    f >> j;
    if (f.fail()) {
        throw std::runtime_error("Could not read from file " + filename);
    }

    auto bones = j.at("skeleton").at("bones");
    for (const auto& bone : bones) {
        std::cerr << bone.at("name") << std::endl;
    }
    std::map<std::string, Material> materials;
    std::list<std::shared_ptr<ColoredVertexArray>> result;
    
    for (const auto& material : j.at("materials")) {
        /*
        "diffuse_color" : [1,1,1],
        "diffuse_map_intensity" : 1,
        "specular_color" : [0.062745,0.05098,0.05098],
        "specular_map_intensity" : 1,
        "shininess" : 0.5,
        "opacity" : 1,
        "translucency" : 0,
        "emissive_color" : [0.19608,0.10196,0.10196],
        "ambient_color" : [0.30196,0.27843,0.27843],
        "transparency_map_intensity" : 1,
        "shadeless" : false,
        "wireframe" : false,
        "transparent" : false,
        "alphaToCoverage" : true,
        "backfaceCull" : true,
        "depthless" : false,
        "castShadows" : true,
        "receiveShadows" : true,
        "sssEnabled" : true,
        "sssRScale" : 5,
        "sssGScale" : 2.5,
        "sssBScale" : 1,
        "diffuse_texture" : "textures/middleage_lightskinned_female_diffuse.png",
        "viewPortColor" : [0.75089,0.57281,0.45185],
        "viewPortAlpha" : 1
        */
        if (!materials.insert({material.at("name"), Material{
            .texture_descriptor{
                .color = gen_filename(filename, material.at("diffuse_texture"))
            },
            .ambience = OrderableFixedArray{get_fixed_array<float, 3>(material.at("ambient_color"))},
            .diffusivity = OrderableFixedArray{get_fixed_array<float, 3>(material.at("diffuse_color"))},
            .specularity = OrderableFixedArray{get_fixed_array<float, 3>(material.at("specular_color"))}
        }}).second) {
            throw std::runtime_error("Could not insert material " + material.at("name").get<std::string>());
        }
    }

    for (const auto& geometry : j.at("geometries")) {
        auto mit = materials.find(geometry.at("material"));
        if (mit == materials.end()) {
            throw std::runtime_error("Could not find material with name " + geometry.at("material").get<std::string>());
        }
        const Material& m = mit->second;
        TriangleList tl{
            filename,
            Material{
                .texture_descriptor = m.texture_descriptor,
                .occluded_type = occluded_type,
                .occluder_type = occluder_type,
                .occluded_by_black = occluded_by_black,
                .aggregate_mode = aggregate_mode,
                .transformation_mode = transformation_mode,
                .is_small = is_small,
                .ambience = m.ambience,
                .diffusivity = m.diffusivity,
                .specularity = m.specularity}.compute_color_mode()};
        auto mesh = geometry.at("mesh");
        std::vector<FixedArray<float, 3>> vertices = load_vector<3>(mesh.at("vertices"));
        std::vector<FixedArray<float, 2>> uv_coordinates = load_vector<2>(mesh.at("uv_coordinates"));
        auto uv_faces = mesh.at("uv_faces");
        auto uv_it = uv_faces.begin();
        for (const auto& face : mesh.at("faces")) {
            if (uv_it == uv_faces.end()) {
                throw std::runtime_error("uv_faces too short");
            }
            auto f = face.get<std::vector<size_t>>();
            auto u = uv_it->get<std::vector<size_t>>();
            if (f.size() != u.size()) {
                throw std::runtime_error("face and uv_face have unequal number of elements");
            }
            if (f.size() == 4) {
                tl.draw_rectangle_wo_normals(
                    vertices.at(f[0]),
                    vertices.at(f[1]),
                    vertices.at(f[2]),
                    vertices.at(f[3]),
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    uv_coordinates.at(u[0]),
                    uv_coordinates.at(u[1]),
                    uv_coordinates.at(u[2]),
                    uv_coordinates.at(u[3]));
            } else {
                throw std::runtime_error("Unsupported dimensionality");
            }
            ++uv_it;
        }
        if (uv_it != uv_faces.end()) {
            throw std::runtime_error("uv_faces too long");
        }
        if (tl.triangles_.empty()) {
            throw std::runtime_error("Triangle array is empty in file " + filename);
        }
        tl.convert_triangle_to_vertex_normals();
        result.push_back(tl.triangle_array());
        tl.triangles_.clear();
    }
    return result;
}
