#include "Load_Dff_Array.hpp"
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Draw_Distance_Db.hpp>
#include <Mlib/Geometry/Mesh/Load/Ide_Flags.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Nan_To_Num.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <algorithm>

using namespace Mlib;

template <class TPosition>
DffArrays<TPosition> Mlib::load_dff(
    const std::string& filename,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb,
    FrameTransformation frame_transformation)
{
    auto ifs = create_ifstream(filename, std::ios::binary);
    if (ifs->fail()) {
        THROW_OR_ABORT("Could not open file for read: \"" + filename + '"');
    }
    try {
        return load_dff(
            *ifs,
            std::filesystem::path{ filename }.filename().string() + '_',
            cfg,
            dddb,
            frame_transformation);
    } catch (std::runtime_error& e) {
        THROW_OR_ABORT("Could not read file \"" + filename + "\": "+ e.what());
    }
}

template <class TPosition>
DffArrays<TPosition> Mlib::load_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb,
    FrameTransformation frame_transformation)
{
    using I = funpack_t<TPosition>;
    DffArrays<TPosition> result;
    auto clump = Mlib::Dff::read_dff(istr, IoVerbosity::SILENT);
    for (const auto& a : clump.atomics) {
        if (a.frame == nullptr) {
            THROW_OR_ABORT("Atomic has no frame");
        }
        if (a.geometry->morph_targets.empty()) {
            THROW_OR_ABORT("Morph targets empty");
        }
        auto trafo = a.frame->matrix.casted<float, I>();
        for (uint32_t p = a.frame->parent; p != UINT32_MAX;) {
            if (p >= clump.frames.size()) {
                THROW_OR_ABORT("Parent frame index out of bounds");
            }
            const auto& parent = clump.frames[p];
            trafo = parent.matrix.casted<float, I>() * trafo;
            p = parent.parent;
        }
        if (any(frame_transformation & FrameTransformation::ZERO_POSITION)) {
            trafo.t = (I)0.f;
        }
        if (any(frame_transformation & FrameTransformation::IDENTITY_ROTATION)) {
            trafo.R = fixed_identity_array<float, 3>();
        }
        const auto& morph_target = a.geometry->morph_targets[0];
        const auto& materials = a.geometry->mat_list.materials;
        NonCopyingVector<TriangleList<TPosition>> tls(materials.size());
        for (const auto& material : materials) {
            auto col3 = FixedArray<float, 3>{
                material.color(0) / 255.f,
                material.color(1) / 255.f,
                material.color(2) / 255.f };
            const auto& ide = dddb.get_item(a.frame->name);
            auto center_distances = ide.center_distances(morph_target.bounding_sphere.radius);
            auto& tl = tls.emplace_back(
                name + "_" + a.frame->name,
                Material{
                    .blend_mode = any(ide.flags & IdeFlags::ADDITIVE)
                        ? BlendMode::CONTINUOUS_ADD
                        : any(ide.flags & (IdeFlags::NO_ZBUFFER_WRITE | IdeFlags::DRAW_LAST))
                            ? BlendMode::CONTINUOUS
                            : BlendMode::BINARY_05,
                    .textures_color = cfg.textures,
                    .period_world = cfg.period_world,
                    .reflection_map = cfg.reflection_map,
                    .occluded_pass = cfg.occluded_pass,
                    .occluder_pass = cfg.occluder_pass,
                    .alpha_distances = OrderableFixedArray{ cfg.alpha_distances },
                    .magnifying_interpolation_mode = cfg.magnifying_interpolation_mode,
                    .aggregate_mode = cfg.aggregate_mode,
                    .transformation_mode = cfg.transformation_mode,
                    .cull_faces = any(ide.flags & IdeFlags::CULL) ? cfg.cull_faces_default : false,
                    .shading = {
                        .ambient = OrderableFixedArray<float, 3>((cfg.ambient_factor * col3) * material.surface_properties.ambient),
                        .diffuse = OrderableFixedArray<float, 3>((cfg.diffuse_factor * col3) * material.surface_properties.diffuse),
                        .specular = OrderableFixedArray<float, 3>(cfg.specular_factor * material.surface_properties.specular),
                        .reflectance = OrderableFixedArray{ cfg.reflectance },
                        .fresnel = cfg.fresnel,
                        .fog_distances = OrderableFixedArray{ cfg.fog_distances },
                        .fog_ambient = OrderableFixedArray{ cfg.fog_ambient }
                    },
                    .dynamically_lighted = cfg.dynamically_lighted
                },
                Morphology{
                    .physics_material = (center_distances(0) == 0.f)
                        ? cfg.physics_material
                        : (cfg.physics_material & ~PhysicsMaterial::ATTR_COLLIDE),
                    .center_distances = OrderableFixedArray{ center_distances },
                    .max_triangle_distance = cfg.max_triangle_distance });
            if ((material.texture != nullptr) && cfg.textures.empty()) {
                auto filename_lower = ide.texture_dictionary + ".txd_" + *material.texture->name;
                std::transform(filename_lower.begin(), filename_lower.end(), filename_lower.begin(), ::tolower);
                tl.material.textures_color = { {.texture_descriptor = TextureDescriptor{
                    .color = ColormapWithModifiers{
                        .filename = VariableAndHash{ filename_lower },
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
            auto material_color = materials[v.matId].color;
            if (normals.empty()) {
                tls[v.matId].draw_triangle_wo_normals(
                    vertices[v.v(0)].template casted<TPosition>(),
                    vertices[v.v(1)].template casted<TPosition>(),
                    vertices[v.v(2)].template casted<TPosition>(),
                    colors.empty() ? material_color : Colors::multiply(material_color, colors[v.v(0)]),
                    colors.empty() ? material_color : Colors::multiply(material_color, colors[v.v(1)]),
                    colors.empty() ? material_color : Colors::multiply(material_color, colors[v.v(2)]),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : nan_to_num((*uvs)[v.v(0)].base(), 0.f),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : nan_to_num((*uvs)[v.v(1)].base(), 0.f),
                    (uvs == nullptr) ? fixed_zeros<float, 2>() : nan_to_num((*uvs)[v.v(2)].base(), 0.f),
                    {},
                    {},
                    {},
                    NormalVectorErrorBehavior::WARN | NormalVectorErrorBehavior::SKIP,
                    TriangleTangentErrorBehavior::ZERO);
            } else {
                tls[v.matId].draw_triangle_with_normals(
                    vertices[v.v(0)].template casted<TPosition>(),
                    vertices[v.v(1)].template casted<TPosition>(),
                    vertices[v.v(2)].template casted<TPosition>(),
                    normals[v.v(0)],
                    normals[v.v(1)],
                    normals[v.v(2)],
                    colors.empty() ? material_color : Colors::multiply(material_color, colors[v.v(0)]),
                    colors.empty() ? material_color : Colors::multiply(material_color, colors[v.v(1)]),
                    colors.empty() ? material_color : Colors::multiply(material_color, colors[v.v(2)]),
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
            if (tl.triangles.empty()) {
                continue;
            }
            result.renderables.push_back(tl.triangle_array()->template transformed<TPosition>(trafo, "_root_frame"));
        }
    }
    return result;
}

namespace Mlib {

template DffArrays<float> load_dff<float>(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    const DrawDistanceDb& dddb,
    FrameTransformation frame_transformation);

template DffArrays<CompressedScenePos> load_dff<CompressedScenePos>(
    const std::string& filename,
    const LoadMeshConfig<CompressedScenePos>& cfg,
    const DrawDistanceDb& dddb,
    FrameTransformation frame_transformation);

}
