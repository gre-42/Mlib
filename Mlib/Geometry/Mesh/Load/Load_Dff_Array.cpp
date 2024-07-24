#include "Load_Dff_Array.hpp"
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Draw_Distance_Db.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

template <class TPosition>
DffArrays<TPosition> Mlib::load_dff(
    const std::string& filename,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb)
{
    auto ifs = create_ifstream(filename, std::ios::binary);
    if (ifs->fail()) {
        THROW_OR_ABORT("Could not open file for read: \"" + filename + '"');
    }
    try {
        return load_dff(*ifs, std::filesystem::path{ filename }.filename().string() + '_', cfg, dddb);
    } catch (std::runtime_error& e) {
        THROW_OR_ABORT("Could not read file \"" + filename + "\": "+ e.what());
    }
}

template <class TPosition>
DffArrays<TPosition> Mlib::load_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb)
{
    DffArrays<TPosition> result;
    auto clump = Mlib::Dff::read_dff(istr, IoVerbosity::SILENT);
    for (const auto& a : clump.atomics) {
        if (a.frame == nullptr) {
            THROW_OR_ABORT("Atomic has no frame");
        }
        if (a.geometry->morph_targets.empty()) {
            THROW_OR_ABORT("Morph targets empty");
        }
        auto m = a.frame->matrix.casted<float, TPosition>();
        for (size_t p = a.frame->parent; p != -1;) {
            if (p >= clump.frames.size()) {
                THROW_OR_ABORT("Parent frame index out of bounds");
            }
            const auto& parent = clump.frames[p];
            m = parent.matrix.casted<float, TPosition>() * m;
            p = parent.parent;
        }
        const auto& morph_target = a.geometry->morph_targets[0];
        const auto& materials = a.geometry->mat_list.materials;
        NonCopyingVector<TriangleList<TPosition>> tls(materials.size());
        for (const auto& m : materials) {
            auto col3 = FixedArray<float, 3>{ m.color(0) / 255.f, m.color(1) / 255.f, m.color(2) / 255.f };
            auto& tl = tls.emplace_back(
                name + "_" + a.frame->name,
                Material{
                    .blend_mode = BlendMode::BINARY_05,
                    .textures_color = cfg.textures,
                    .period_world = cfg.period_world,
                    .reflection_map = cfg.reflection_map,
                    .occluded_pass = cfg.occluded_pass,
                    .occluder_pass = cfg.occluder_pass,
                    .alpha_distances = OrderableFixedArray{ cfg.alpha_distances },
                    .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                    .aggregate_mode = cfg.aggregate_mode,
                    .transformation_mode = cfg.transformation_mode,
                    .cull_faces = cfg.cull_faces_default,
                    .shading = {
                        .ambient = OrderableFixedArray<float, 3>(col3 * m.surface_properties.ambient),
                        .diffuse = OrderableFixedArray<float, 3>(col3 * m.surface_properties.diffuse),
                        .specular = m.surface_properties.specular,
                        .fresnel = cfg.fresnel
                    },
                    .dynamically_lighted = cfg.dynamically_lighted
                },
                Morphology{
                    .physics_material = cfg.physics_material,
                    .center_distances = OrderableFixedArray{
                        dddb.get_center_distances(a.frame->name, morph_target.bounding_sphere.radius()) },
                    .max_triangle_distance = cfg.max_triangle_distance });
            if (m.texture != nullptr) {
                tl.material.textures_color = { {.texture_descriptor = TextureDescriptor{
                    .color = ColormapWithModifiers{
                        .filename = m.texture->name,
                        .color_mode = ColorMode::RGBA,
                        .mipmap_mode = MipmapMode::WITH_MIPMAPS
                    }.compute_hash()}} };
                // linfo() << "Texture: " << tex->name;
            }
        }
        const auto& vertices = morph_target.vertices;
        const auto& normals = morph_target.normals;
        const auto& colors = a.geometry->colors;
        const auto* uvs = a.geometry->tex_coords.empty() ? nullptr : &a.geometry->tex_coords[0];

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
            if (!normals.empty() && any(v.v >= integral_cast<uint16_t>(normals.size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            if (!colors.empty() && any(v.v >= integral_cast<uint16_t>(colors.size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            if ((uvs != nullptr) && any(v.v >= integral_cast<uint16_t>(uvs->size()))) {
                THROW_OR_ABORT("Vertex ID too large");
            }
            auto material_color = materials[v.matId].color.row_range<0, 3>().casted<float>() / 255.f;
            if (normals.empty()) {
                tls[v.matId].draw_triangle_wo_normals(
                    vertices[v.v(0)].template casted<TPosition>(),
                    vertices[v.v(1)].template casted<TPosition>(),
                    vertices[v.v(2)].template casted<TPosition>(),
                    colors.empty() ? material_color : material_color * (colors[v.v(0)].template row_range<0, 3>().template casted<float>() / 255.f),
                    colors.empty() ? material_color : material_color * (colors[v.v(1)].template row_range<0, 3>().template casted<float>() / 255.f),
                    colors.empty() ? material_color : material_color * (colors[v.v(2)].template row_range<0, 3>().template casted<float>() / 255.f),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : (*uvs)[v.v(0)].base(),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : (*uvs)[v.v(1)].base(),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : (*uvs)[v.v(2)].base(),
                    {},
                    {},
                    {},
                    TriangleNormalErrorBehavior::ZERO,
                    TriangleTangentErrorBehavior::ZERO);
            } else {
                tls[v.matId].draw_triangle_with_normals(
                    vertices[v.v(0)].template casted<TPosition>(),
                    vertices[v.v(1)].template casted<TPosition>(),
                    vertices[v.v(2)].template casted<TPosition>(),
                    normals[v.v(0)],
                    normals[v.v(1)],
                    normals[v.v(2)],
                    colors.empty() ? material_color : material_color * (colors[v.v(0)].template row_range<0, 3>().template casted<float>() / 255.f),
                    colors.empty() ? material_color : material_color * (colors[v.v(1)].template row_range<0, 3>().template casted<float>() / 255.f),
                    colors.empty() ? material_color : material_color * (colors[v.v(2)].template row_range<0, 3>().template casted<float>() / 255.f),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : (*uvs)[v.v(0)].base(),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : (*uvs)[v.v(1)].base(),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : (*uvs)[v.v(2)].base(),
                    {},
                    {},
                    {},
                    TriangleTangentErrorBehavior::ZERO);
            }
        }
        for (auto& tl : tls) {
            // if (normals.empty()) {
            //     tl.convert_triangle_to_vertex_normals();
            // }
            result.renderables.push_back(tl.triangle_array()->transformed<TPosition>(m, "_root_frame"));
        }
    }
    return result;
}

namespace Mlib {

template DffArrays<float> load_dff<float>(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const DrawDistanceDb& dddb);

template DffArrays<double> load_dff<double>(
    const std::string& filename,
    const LoadMeshConfig<double>& cfg,
    const DrawDistanceDb& dddb);

}
