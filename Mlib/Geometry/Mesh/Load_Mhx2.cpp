#include "Load_Mhx2.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
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

struct ScaleAndOffset {
    explicit ScaleAndOffset(const json& j) {
        scale10 = 10 * j.at("scale").get<float>();
        offset = FixedArray<float, 3>(j.at("offset").get<std::vector<float>>());
    }
    float scale10;
    FixedArray<float, 3> offset;
};

std::shared_ptr<AnimatedColoredVertexArrays> Mlib::load_mhx2(
    const std::string& filename,
    const LoadMeshConfig& cfg)
{
    if (cfg.apply_static_lighting) {
        throw std::runtime_error("Static lighting not supported for mhx2 files");
    }

    json j;
    std::ifstream f{filename};
    if (f.fail()) {
        throw std::runtime_error("Could not open file " + filename);
    }
    f >> j;
    if (f.fail()) {
        throw std::runtime_error("Could not read from file " + filename);
    }

    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    {
        ScaleAndOffset so_skelleton{j.at("skeleton")};
        auto bones = j.at("skeleton").at("bones");
        std::map<std::string, Bone*> bone_names;
        for (const auto& bone : bones) {
            FixedArray<float, 4, 4> initial_absolute_transformation;
            {
                auto initial_absolute_transformation_v = bone.at("matrix").get<std::vector<std::vector<float>>>();
                if (initial_absolute_transformation_v.size() != 4) {
                    throw std::runtime_error("wrong matrix size");
                }
                for (size_t r = 0; r < 4; ++r) {
                    if (initial_absolute_transformation_v[r].size() != 4) {
                        throw std::runtime_error("wrong matrix size");
                    }
                    for (size_t c = 0; c < 4; ++c) {
                        initial_absolute_transformation(r, c) = initial_absolute_transformation_v[r][c];
                    }
                }
            }
            auto parent = bone.find("parent");
            if (parent == bone.end()) {
                initial_absolute_transformation(0, 3) += so_skelleton.offset(0);
                initial_absolute_transformation(1, 3) += so_skelleton.offset(1);
                initial_absolute_transformation(2, 3) += so_skelleton.offset(2);
            }
            initial_absolute_transformation(0, 3) /= so_skelleton.scale10;
            initial_absolute_transformation(1, 3) /= so_skelleton.scale10;
            initial_absolute_transformation(2, 3) /= so_skelleton.scale10;
            // ~/.config/blender/2.90/scripts/addons/import_runtime_mhx2/importer.py
            // if "matrix" in mhBone.keys():
            //     mat = Matrix(mhBone["matrix"])
            //     nmat = Matrix((mat[0], -mat[2], mat[1])).to_3x3().to_4x4()
            //     nmat.col[3] = eb.matrix.col[3]
            //     eb.matrix = nmat
            // else:
            //     eb.roll = mhBone["roll"]
            // {
            //     FixedArray<float, 4, 4> roll = assemble_homogeneous_4x4(
            //         rodrigues(
            //             FixedArray<float, 3>{0, 0, 1},
            //             bone.at("roll").get<float>()),
            //         FixedArray<float, 3>{0, 0, 0});
            //     initial_absolute_transformation = dot2d(initial_absolute_transformation, roll);
            // }
            // {
            //     auto R = R3_from_4x4(initial_absolute_transformation);
            //     auto t = t3_from_4x4(initial_absolute_transformation);
            //     std::swap(R[1], R[2]);
            //     R[1] *= -1;
            //     initial_absolute_transformation = assemble_homogeneous_4x4(R, t);
            // }
            {
                auto t = t3_from_4x4(initial_absolute_transformation);
                auto R = fixed_identity_array<float, 3>();
                initial_absolute_transformation = assemble_homogeneous_4x4(R, t);
            }
            Bone* new_bone = new Bone{
                .index = result->bone_indices.size(),
                .initial_absolute_transformation = initial_absolute_transformation};
            std::string new_bone_name = bone.at("name").get<std::string>();
            result->bone_indices.insert({new_bone_name, new_bone->index});
            if (parent != bone.end()) {
                std::string parent_name = parent.value().get<std::string>();
                auto par = bone_names.find(parent_name);
                if (par == bone_names.end()) {
                    throw std::runtime_error("Unknown bone " + parent_name);
                }
                par->second->children.push_back(std::unique_ptr<Bone>(new_bone));
            } else {
                if (result->skeleton != nullptr) {
                    throw std::runtime_error("Found multiple root bones");
                }
                result->skeleton.reset(new_bone);
            }
            if (!bone_names.insert({new_bone_name, new_bone}).second) {
                throw std::runtime_error("Could not insert bone " + new_bone_name);
            }
        }
        if (result->skeleton == nullptr) {
            throw std::runtime_error("Could not find root node");
        }
    }
    std::map<std::string, Material> materials;

    // auto contains_bone = [&result](
    //     const std::string& bone_name,
    //     const std::list<BoneWeight>& weights)
    // {
    //     auto bone_id = result->bone_indices.find(bone_name);
    //     if (bone_id == result->bone_indices.end()) {
    //         throw std::runtime_error("Could not find bone with name " + bone_name);
    //     }
    //     //std::cerr << bone_id->second << std::endl;
    //     //assert_true(false);
    //     auto it = std::find_if(
    //         weights.begin(),
    //         weights.end(),
    //         [&bone_id](const auto& w){return w.bone_index == bone_id->second;});
    //     return (it != weights.end());
    // };
    
    for (const auto& material : j.at("materials")) {
        // "diffuse_color" : [1,1,1],
        // "diffuse_map_intensity" : 1,
        // "specular_color" : [0.062745,0.05098,0.05098],
        // "specular_map_intensity" : 1,
        // "shininess" : 0.5,
        // "opacity" : 1,
        // "translucency" : 0,
        // "emissive_color" : [0.19608,0.10196,0.10196],
        // "ambient_color" : [0.30196,0.27843,0.27843],
        // "transparency_map_intensity" : 1,
        // "shadeless" : false,
        // "wireframe" : false,
        // "transparent" : false,
        // "alphaToCoverage" : true,
        // "backfaceCull" : true,
        // "depthless" : false,
        // "castShadows" : true,
        // "receiveShadows" : true,
        // "sssEnabled" : true,
        // "sssRScale" : 5,
        // "sssGScale" : 2.5,
        // "sssBScale" : 1,
        // "diffuse_texture" : "textures/middleage_lightskinned_female_diffuse.png",
        // "viewPortColor" : [0.75089,0.57281,0.45185],
        // "viewPortAlpha" : 1
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
        ScaleAndOffset so_geometry{geometry};
        auto mit = materials.find(geometry.at("material"));
        if (mit == materials.end()) {
            throw std::runtime_error("Could not find material with name " + geometry.at("material").get<std::string>());
        }
        const Material& m = mit->second;
        TriangleList tl{
            filename,
            Material{
                .texture_descriptor = m.texture_descriptor,
                .occluded_type = cfg.occluded_type,
                .occluder_type = cfg.occluder_type,
                .occluded_by_black = cfg.occluded_by_black,
                .aggregate_mode = cfg.aggregate_mode,
                .transformation_mode = cfg.transformation_mode,
                .is_small = cfg.is_small,
                .ambience = m.ambience,
                .diffusivity = m.diffusivity,
                .specularity = m.specularity}.compute_color_mode()};
        auto mesh = geometry.at("mesh");
        std::vector<FixedArray<float, 3>> vertices = load_vector<3>(mesh.at("vertices"));
        std::vector<FixedArray<float, 2>> uv_coordinates = load_vector<2>(mesh.at("uv_coordinates"));
        std::vector<std::list<BoneWeight>> vertex_bone_weights;
        vertex_bone_weights.resize(vertices.size());
        for (const auto& bw : mesh.at("weights").items()) {
            auto bone_id = result->bone_indices.find(bw.key());
            if (bone_id == result->bone_indices.end()) {
                throw std::runtime_error("Could not find bone id for " + bw.key());
            }
            for (const auto& b : bw.value()) {
                if (b.size() != 2) {
                    throw std::runtime_error("Invalid weight length");
                }
                size_t vertex_id = b[0].get<size_t>();
                if (vertex_id >= vertex_bone_weights.size()) {
                    throw std::runtime_error("Vertex ID out of bounds");
                }
                vertex_bone_weights[vertex_id].push_back(BoneWeight{
                    .bone_index = bone_id->second,
                    .weight = b[1].get<float>()});
            }
        }
        for (const auto& w : vertex_bone_weights) {
            float s = 0;
            for (const auto& ww : w) {
                s += ww.weight;
            }
            if (std::abs(s - 1) > 1e-4) {
                throw std::runtime_error("Weights do not sum up to 1: " + std::to_string(s));
            }
        }
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
            if (f.size() == 3) {
                tl.draw_triangle_wo_normals(
                    (vertices.at(f[0]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[1]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[2]) + so_geometry.offset) / so_geometry.scale10,
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[0])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[0])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[0]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[1])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[1])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[1]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[2])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[2])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[2]))},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    uv_coordinates.at(u[0]),
                    uv_coordinates.at(u[1]),
                    uv_coordinates.at(u[2]),
                    std::vector(vertex_bone_weights.at(f[0]).begin(), vertex_bone_weights.at(f[0]).end()),
                    std::vector(vertex_bone_weights.at(f[1]).begin(), vertex_bone_weights.at(f[1]).end()),
                    std::vector(vertex_bone_weights.at(f[2]).begin(), vertex_bone_weights.at(f[2]).end()));
            } else if (f.size() == 4) {
                tl.draw_rectangle_wo_normals(
                    (vertices.at(f[0]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[1]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[2]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[3]) + so_geometry.offset) / so_geometry.scale10,
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[0])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[0])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[0]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[1])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[1])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[1]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[2])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[2])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[2]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[3])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[3])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[3]))},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f},
                    uv_coordinates.at(u[0]),
                    uv_coordinates.at(u[1]),
                    uv_coordinates.at(u[2]),
                    uv_coordinates.at(u[3]),
                    std::vector(vertex_bone_weights.at(f[0]).begin(), vertex_bone_weights.at(f[0]).end()),
                    std::vector(vertex_bone_weights.at(f[1]).begin(), vertex_bone_weights.at(f[1]).end()),
                    std::vector(vertex_bone_weights.at(f[2]).begin(), vertex_bone_weights.at(f[2]).end()),
                    std::vector(vertex_bone_weights.at(f[3]).begin(), vertex_bone_weights.at(f[3]).end()));
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
        result->cvas.push_back(tl.triangle_array());
        tl.triangles_.clear();
    }
    FixedArray<float, 3, 3> rotation_matrix{tait_bryan_angles_2_matrix(cfg.rotation)};
    for (auto& l : result->cvas) {
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
