#include "Save_Obj.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <set>

using namespace Mlib;

void Mlib::save_obj(
    const std::string& filename,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
    const std::function<ObjMaterial(const Material&)>& convert_material)
{
    std::set<std::string> names;
    std::map<Material, size_t> material_indices;
    std::map<std::string, ObjMaterial> obj_materials;
    std::vector<NamedInputTriangles<std::vector<FixedArray<ColoredVertex<double>, 3>>>> itris;
    itris.reserve(cvas.size());
    for (const std::shared_ptr<ColoredVertexArray<double>>& cva : cvas) {
        if (cva->name.empty()) {
            if (!cva->material.textures.empty()) {
                THROW_OR_ABORT("Empty name, material: \"" + cva->material.textures.front().texture_descriptor.color.filename);
            } else {
                THROW_OR_ABORT("Empty name, no material color texture");
            }
        }
        if (!names.insert(cva->name).second) {
            THROW_OR_ABORT("Duplicate name: \"" + cva->name + '"');
        }
        material_indices.insert({ cva->material, material_indices.size() });
        std::string material_name = "material_" + std::to_string(material_indices.at(cva->material));
        itris.push_back({ cva->name, material_name, cva->triangles });
        obj_materials.insert({ material_name, convert_material(cva->material) });
    }
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ itris }, &obj_materials);
}
