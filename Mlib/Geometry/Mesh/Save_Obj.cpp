#include "Save_Obj.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <set>

using namespace Mlib;

template <class TPos, class TContainer>
void save_obj_with_materials(
    const std::string& filename,
    const std::list<std::shared_ptr<TContainer>>& cvas,
    const std::function<std::string(const TContainer&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material)
{
    std::set<std::string> names;
    std::map<Material, size_t> material_indices;
    std::map<std::string, ObjMaterial> obj_materials;
    std::vector<NamedInputPolygons<
        decltype(cvas.front()->triangles),
        decltype(cvas.front()->quads)>> ipolys;
    ipolys.reserve(cvas.size());
    for (const std::shared_ptr<TContainer>& cva : cvas) {
        if (cva->name.empty()) {
            if (!cva->material.textures_color.empty()) {
                THROW_OR_ABORT("Empty name, material: \"" + *cva->material.textures_color.front().texture_descriptor.color.filename);
            } else {
                THROW_OR_ABORT("Empty name, no material color texture");
            }
        }
        if (!names.insert(cva->name.full_name()).second) {
            THROW_OR_ABORT("Duplicate name: \"" + cva->name.full_name() + '"');
        }
        std::string mname;
        if (material_name) {
            mname = material_name(*cva);
        } else {
            material_indices.insert({ cva->material, material_indices.size() });
            mname = cva->name.full_name() + "_material_" + std::to_string(material_indices.at(cva->material));
        }
        ObjMaterial obj_material;
        if (convert_material) {
            obj_material = convert_material(cva->material);
        } else {
            obj_material = ObjMaterial{
                .ambient = cva->material.shading.ambient,
                .diffuse = cva->material.shading.diffuse,
                .specular = cva->material.shading.specular
            };
        }
        ipolys.push_back({ cva->name.full_name(), mname, cva->triangles, cva->quads });
        obj_materials.insert({ mname, obj_material });
    }
    save_obj(filename, IndexedFaceSet<float, TPos, size_t>{ ipolys }, &obj_materials);
}

template <class TPos>
void Mlib::save_obj(
    const std::string& filename,
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<std::string(const ColoredVertexArray<TPos>&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material)
{
    save_obj_with_materials<TPos>(filename, cvas, material_name, convert_material);
}

template <class TPos>
void Mlib::save_obj(
    const std::string& filename,
    const std::list<std::shared_ptr<TriangleList<TPos>>>& cvas,
    const std::function<std::string(const TriangleList<TPos>&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material)
{
    save_obj_with_materials<TPos>(filename, cvas, material_name, convert_material);
}

template void Mlib::save_obj<float>(
    const std::string& filename,
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
    const std::function<std::string(const ColoredVertexArray<float>&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material);

template void Mlib::save_obj<CompressedScenePos>(
    const std::string& filename,
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const std::function<std::string(const ColoredVertexArray<CompressedScenePos>&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material);

template void Mlib::save_obj<float>(
    const std::string& filename,
    const std::list<std::shared_ptr<TriangleList<float>>>& cvas,
    const std::function<std::string(const TriangleList<float>&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material);

template void Mlib::save_obj<CompressedScenePos>(
    const std::string& filename,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& cvas,
    const std::function<std::string(const TriangleList<CompressedScenePos>&)>& material_name,
    const std::function<ObjMaterial(const Material&)>& convert_material);
