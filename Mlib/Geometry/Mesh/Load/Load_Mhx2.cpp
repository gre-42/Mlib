#include "Load_Mhx2.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <filesystem>

using namespace Mlib;

// template <class TValue>
// const auto& map_at(const std::map<std::string, TValue>& m, const std::string& n) {
//     auto it = m.find(n);
//     if (it == m.end()) {
//         THROW_OR_ABORT("Could not find key " + n + " in map");
//     }
//     return it->second;
// }

std::string gen_filename(const std::string& f, const std::string& texture_name) {
    if (!texture_name.empty()) {
        std::filesystem::path p = std::filesystem::path(f).parent_path();
        return p.empty() ? texture_name : std::filesystem::weakly_canonical(p / texture_name).string();
    } else {
        return "";
    }
}

struct ScaleAndOffset {
    explicit ScaleAndOffset(const nlohmann::json& j) {
        scale10 = 10 * j.at("scale").get<float>();
        offset = j.at("offset").get<UFixedArray<float, 3>>();
    }
    float scale10;
    FixedArray<float, 3> offset = uninitialized;
};

std::shared_ptr<AnimatedColoredVertexArrays> Mlib::load_mhx2(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg)
{
    if (cfg.apply_static_lighting) {
        THROW_OR_ABORT("Static lighting not supported for mhx2 files");
    }

    nlohmann::json j;
    {
        auto f = create_ifstream(filename);
        if (f->fail()) {
            THROW_OR_ABORT("Could not open file " + filename);
        }
        *f >> j;
        if (f->fail()) {
            THROW_OR_ABORT("Could not read from file " + filename);
        }
    }

    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    {
        ScaleAndOffset so_skelleton{ j.at("skeleton") };
        const auto& bones = j.at("skeleton").at("bones");
        StringWithHashUnorderedMap<Bone*> bone_names{ "Bone" };
        for (const auto& bone : bones) {
            FixedArray<float, 4, 4> initial_absolute_transformation = uninitialized;
            {
                auto initial_absolute_transformation_v = bone.at("matrix").get<std::vector<std::vector<float>>>();
                if (initial_absolute_transformation_v.size() != 4) {
                    THROW_OR_ABORT("wrong matrix size");
                }
                for (size_t r = 0; r < 4; ++r) {
                    if (initial_absolute_transformation_v[r].size() != 4) {
                        THROW_OR_ABORT("wrong matrix size");
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
            //         rodrigues2(
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
            auto new_bone = std::unique_ptr<Bone>(new Bone{
                .index = result->bone_indices.size(),
                .initial_absolute_transformation = OffsetAndQuaternion<float, float>{initial_absolute_transformation}});
            auto new_bone_name = bone.at("name").get<VariableAndHash<std::string>>();
            bone_names.add(new_bone_name, new_bone.get());
            result->bone_indices.add(new_bone_name, new_bone->index);
            if (parent != bone.end()) {
                auto parent_name = parent.value().get<VariableAndHash<std::string>>();
                auto& par = bone_names.get(parent_name);
                par->children.push_back(std::move(new_bone));
            } else {
                if (result->skeleton != nullptr) {
                    THROW_OR_ABORT("Found multiple root bones");
                }
                result->skeleton = std::move(new_bone);
            }
        }
        if (result->skeleton == nullptr) {
            THROW_OR_ABORT("Could not find root node");
        }
    }
    std::map<std::string, Material> materials;

    // auto contains_bone = [&result](
    //     const std::string& bone_name,
    //     const std::list<BoneWeight>& weights)
    // {
    //     auto bone_id = result->bone_indices.find(bone_name);
    //     if (bone_id == result->bone_indices.end()) {
    //         THROW_OR_ABORT("Could not find bone with name " + bone_name);
    //     }
    //     //lerr() << bone_id->second;
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
            .textures_color{
                BlendMapTexture{.texture_descriptor = {
                    .color = ColormapWithModifiers{
                        .filename = VariableAndHash{ gen_filename(filename, material.at("diffuse_texture")) },
                        .desaturate = cfg.desaturate,
                        .desaturation_exponent = cfg.desaturation_exponent,
                        .histogram = cfg.histogram,
                        .lighten = make_orderable(cfg.lighten),
                        .mipmap_mode = cfg.mipmap_mode,
                        .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level}.compute_hash()}}
            },
            .shading {
                .emissive = make_orderable(cfg.emissive_factor * material.at("emissive_color").get<UFixedArray<float, 3>>()),
                .ambient = make_orderable(cfg.ambient_factor * material.at("ambient_color").get<UFixedArray<float, 3>>()),
                .diffuse = make_orderable(cfg.diffuse_factor * material.at("diffuse_color").get<UFixedArray<float, 3>>()),
                .specular = make_orderable(cfg.specular_factor * material.at("specular_color").get<UFixedArray<float, 3>>()),
                .fog_distances = make_orderable(cfg.shading.fog_distances),
                .fog_ambient = make_orderable(cfg.shading.fog_ambient)
            },
            .dynamically_lighted = cfg.dynamically_lighted
        }}).second) {
            THROW_OR_ABORT("Could not insert material " + material.at("name").get<std::string>());
        }
    }

    for (const auto& geometry : j.at("geometries")) {
        ScaleAndOffset so_geometry{geometry};
        auto mit = materials.find(geometry.at("material"));
        if (mit == materials.end()) {
            THROW_OR_ABORT("Could not find material with name " + geometry.at("material").get<std::string>());
        }
        const Material& m = mit->second;
        TriangleList<float> tl{
            filename,
            Material{
                .textures_color = m.textures_color,
                .occluded_pass = cfg.occluded_pass,
                .occluder_pass = cfg.occluder_pass,
                .alpha_distances = make_orderable(cfg.alpha_distances),
                .aggregate_mode = cfg.aggregate_mode,
                .transformation_mode = cfg.transformation_mode,
                .shading = m.shading}.compute_color_mode(),
            Morphology{
                .physics_material = PhysicsMaterial::ATTR_VISIBLE,
                .center_distances2 = SquaredStepDistances::from_distances(cfg.center_distances),
                .max_triangle_distance = cfg.max_triangle_distance,
            }};
        const auto& mesh = geometry.at("mesh");
        UUVector<FixedArray<float, 3>> vertices = mesh.at("vertices").get<UUVector<FixedArray<float, 3>>>();
        UUVector<FixedArray<float, 2>> uv_coordinates = mesh.at("uv_coordinates").get<UUVector<FixedArray<float, 2>>>();
        std::vector<std::list<BoneWeight>> vertex_bone_weights;
        vertex_bone_weights.resize(vertices.size());
        for (const auto& bw : mesh.at("weights").items()) {
            auto bone_id = result->bone_indices.get(VariableAndHash<std::string>{ bw.key() });
            for (const auto& b : bw.value()) {
                if (b.size() != 2) {
                    THROW_OR_ABORT("Invalid weight length");
                }
                size_t vertex_id = b[0].get<size_t>();
                if (vertex_id >= vertex_bone_weights.size()) {
                    THROW_OR_ABORT("Vertex ID out of bounds");
                }
                vertex_bone_weights[vertex_id].push_back(BoneWeight{
                    .bone_index = bone_id,
                    .weight = b[1].get<float>()});
            }
        }
        for (const auto& w : vertex_bone_weights) {
            float s = 0;
            for (const auto& ww : w) {
                s += ww.weight;
            }
            if (std::abs(s - 1) > 1e-4) {
                THROW_OR_ABORT("Weights do not sum up to 1: " + std::to_string(s));
            }
        }
        const auto& uv_faces = mesh.at("uv_faces");
        auto uv_it = uv_faces.begin();
        for (const auto& face : mesh.at("faces")) {
            if (uv_it == uv_faces.end()) {
                THROW_OR_ABORT("uv_faces too short");
            }
            auto f = face.get<std::vector<size_t>>();
            auto u = uv_it->get<std::vector<size_t>>();
            if (f.size() != u.size()) {
                THROW_OR_ABORT("face and uv_face have unequal number of elements");
            }
            if (f.size() == 3) {
                tl.draw_triangle_wo_normals(
                    (vertices.at(f[0]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[1]) + so_geometry.offset) / so_geometry.scale10,
                    (vertices.at(f[2]) + so_geometry.offset) / so_geometry.scale10,
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[0])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[0])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[0]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[1])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[1])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[1]))},
                    // {1.f * contains_bone("thigh_l", vertex_bone_weights.at(f[2])), 1.f * contains_bone("calf_r", vertex_bone_weights.at(f[2])), 1.f * contains_bone("upperarm_r", vertex_bone_weights.at(f[2]))},
                    Colors::WHITE,
                    Colors::WHITE,
                    Colors::WHITE,
                    uv_coordinates.at(u[0]),
                    uv_coordinates.at(u[1]),
                    uv_coordinates.at(u[2]),
                    std::nullopt,
                    std::vector(vertex_bone_weights.at(f[0]).begin(), vertex_bone_weights.at(f[0]).end()),
                    std::vector(vertex_bone_weights.at(f[1]).begin(), vertex_bone_weights.at(f[1]).end()),
                    std::vector(vertex_bone_weights.at(f[2]).begin(), vertex_bone_weights.at(f[2]).end()),
                    NormalVectorErrorBehavior::WARN | NormalVectorErrorBehavior::SKIP,
                    cfg.triangle_tangent_error_behavior);
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
                    Colors::WHITE,
                    Colors::WHITE,
                    Colors::WHITE,
                    Colors::WHITE,
                    uv_coordinates.at(u[0]),
                    uv_coordinates.at(u[1]),
                    uv_coordinates.at(u[2]),
                    uv_coordinates.at(u[3]),
                    std::nullopt,
                    std::vector(vertex_bone_weights.at(f[0]).begin(), vertex_bone_weights.at(f[0]).end()),
                    std::vector(vertex_bone_weights.at(f[1]).begin(), vertex_bone_weights.at(f[1]).end()),
                    std::vector(vertex_bone_weights.at(f[2]).begin(), vertex_bone_weights.at(f[2]).end()),
                    std::vector(vertex_bone_weights.at(f[3]).begin(), vertex_bone_weights.at(f[3]).end()),
                    NormalVectorErrorBehavior::WARN | NormalVectorErrorBehavior::SKIP,
                    cfg.triangle_tangent_error_behavior);
            } else {
                THROW_OR_ABORT("Unsupported dimensionality");
            }
            ++uv_it;
        }
        if (uv_it != uv_faces.end()) {
            THROW_OR_ABORT("uv_faces too long");
        }
        if (tl.triangles.empty()) {
            THROW_OR_ABORT("Triangle array is empty in file " + filename);
        }
        tl.convert_triangle_to_vertex_normals();
        result->scvas.push_back(tl.triangle_array());
        tl.triangles.clear();
    }
    FixedArray<float, 3, 3> rotation_matrix_p{ tait_bryan_angles_2_matrix(cfg.rotation) };
    auto rotation_matrix_n = inv(rotation_matrix_p).value().T();
    for (auto& l : result->scvas) {
        for (auto& t : l->triangles) {
            for (auto& v : t.flat_iterable()) {
                v.position *= cfg.scale;
                v.position = dot1d(rotation_matrix_p, v.position);
                v.position += cfg.position;
                v.normal = dot1d(rotation_matrix_n, v.normal);
                v.tangent = dot1d(rotation_matrix_p, v.tangent);
            }
        }
    }
    return result;
}
