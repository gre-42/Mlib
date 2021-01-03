#include "Load_Mhx2.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace Mlib;

template <size_t tsize>
std::vector<FixedArray<float, tsize>> load_vector(const json& j) {
    std::list<FixedArray<float, tsize>> vertex_list;
    for (const auto& vertex : j) {
        auto v = vertex.get<std::vector<float>>();
        if (v.size() != tsize) {
            throw std::runtime_error("Unsupported dimensionality");
        }
        vertex_list.push_back(FixedArray<float, tsize>{v});
    }
    return std::vector<FixedArray<float, tsize>>{vertex_list.begin(), vertex_list.end()};
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
    TriangleList tl{
        filename,
        Material{
            .texture_descriptor = TextureDescriptor{color: ""},
            .occluded_type = occluded_type,
            .occluder_type = occluder_type,
            .occluded_by_black = occluded_by_black,
            .aggregate_mode = aggregate_mode,
            .transformation_mode = transformation_mode,
            .is_small = is_small}};
    std::list<std::shared_ptr<ColoredVertexArray>> result;
    
    for (const auto& geometry : j.at("geometries")) {
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
        result.push_back(tl.triangle_array());
        tl.triangles_.clear();
    }
    return result;
}
