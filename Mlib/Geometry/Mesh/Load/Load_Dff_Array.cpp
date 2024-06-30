#include "Load_Dff_Array.hpp"
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

std::list<std::shared_ptr<ColoredVertexArray<float>>> Mlib::load_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg)
{
    auto ifs = create_ifstream(filename, std::ios::binary);
    if (ifs->fail()) {
        THROW_OR_ABORT("Could not open file for read: \"" + filename + '"');
    }
    try {
        return load_dff(*ifs, std::filesystem::path{ filename }.filename().string() + '_', cfg);
    } catch (std::runtime_error& e) {
        THROW_OR_ABORT("Could not read file \"" + filename + "\": "+ e.what());
    }
}

std::list<std::shared_ptr<ColoredVertexArray<float>>> Mlib::load_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg)
{
    std::list<std::shared_ptr<ColoredVertexArray<float>>> result;
    auto clump = Mlib::Dff::read_dff(istr);
    for (const auto& a : clump.atomics) {
        if (a.frame == nullptr) {
            THROW_OR_ABORT("Atomic has no frame");
        }
        const auto& materials = a.geometry->matList.materials;
        NonCopyingVector<TriangleList<float>> tls(materials.size());
        for (const auto& m : materials) {
            tls.emplace_back(
                name + a.frame->name,
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
                    .shading = {
                        .ambient = m.surfaceProps.ambient,
                        .diffuse = m.surfaceProps.diffuse,
                        .specular = m.surfaceProps.specular,
                        .fresnel = cfg.fresnel
                    },
                    .dynamically_lighted = cfg.dynamically_lighted
                },
                cfg.physics_material);
        }
        if (a.geometry->morphTargets.empty()) {
            THROW_OR_ABORT("Morph targets empty");
        }
        if (a.geometry->texCoords.empty()) {
            THROW_OR_ABORT("Texture coordinates empty");
        }
        const auto& vertices = a.geometry->morphTargets[0].vertices;
        const auto& normals = a.geometry->morphTargets[0].normals;
        const auto& colors = a.geometry->colors;
        const auto& uvs = a.geometry->texCoords[0];

        for (const auto& v : a.geometry->triangles) {
            if (v.matId >= tls.size()) {
                THROW_OR_ABORT("Material ID too large (0)");
            }
            if (v.matId >= materials.size()) {
                THROW_OR_ABORT("Material ID too large (1)");
            }
            if (any(v.v >= integral_cast<uint16_t>(vertices.size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            if (any(v.v >= integral_cast<uint16_t>(normals.size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            if (!colors.empty() && any(v.v >= integral_cast<uint16_t>(colors.size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            if (!uvs.empty() && any(v.v >= integral_cast<uint16_t>(uvs.size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            auto material_color = materials[v.matId].color.row_range<0, 3>().casted<float>() / 255.f;
            tls[v.matId].draw_triangle_with_normals(
                vertices[v.v(0)],
                vertices[v.v(1)],
                vertices[v.v(2)],
                normals[v.v(0)],
                normals[v.v(1)],
                normals[v.v(2)],
                colors.empty() ? material_color : material_color * (colors[v.v(0)].row_range<0, 3>().casted<float>() / 255.f),
                colors.empty() ? material_color : material_color * (colors[v.v(1)].row_range<0, 3>().casted<float>() / 255.f),
                colors.empty() ? material_color : material_color * (colors[v.v(2)].row_range<0, 3>().casted<float>() / 255.f),
                uvs.empty() ? fixed_zeros<float, 2>() : uvs[v.v(0)].base(),
                uvs.empty() ? fixed_zeros<float, 2>() : uvs[v.v(1)].base(),
                uvs.empty() ? fixed_zeros<float, 2>() : uvs[v.v(2)].base(),
                {},
                {},
                {},
                TriangleTangentErrorBehavior::ZERO);
        }
        for (const auto& tl : tls) {
            result.push_back(tl.triangle_array());
        }
    }
    return result;
}
