#pragma once
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Geometry/Mesh/Save_Mtllib.hpp>
#include <Mlib/Regex_Select.hpp>
#include <filesystem>
#include <fstream>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
struct Material;

template <class TDir, class TPos, class TIndex>
void save_obj(
    const std::string& filename,
    const IndexedFaceSet<TDir, TPos, TIndex>& data,
    const std::map<std::string, ObjMaterial>* materials)
{
    namespace fs = std::filesystem;
    static const DECLARE_REGEX(filename_re, "^(.*)\\.obj$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(filename, match, filename_re)) {
        throw std::runtime_error("OBJ filename does not have \".obj\" extension: " + filename);
    }
    std::ofstream ostr{ filename };
    ostr.precision(15);
    ostr << std::scientific;
    if (materials != nullptr) {
        std::string mtl_filename = match[1].str() + ".mtl";
        save_mtllib(mtl_filename, *materials);
        ostr << "mtllib " << fs::path(mtl_filename).filename().string()  << '\n';
    }
    for (const auto v : data.vertices) {
        ostr << "v " << v << '\n';
    }
    for (const auto uv : data.uvs) {
        ostr << "vt " << uv << '\n';
    }
    for (const auto n : data.normals) {
        ostr << "vn " << n << '\n';
    }
    for (const NamedObjTriangles<TIndex>& named_triangles : data.named_obj_triangles) {
        if (!named_triangles.name.empty()) {
            ostr << "g " << named_triangles.name << '\n';
            ostr << "usemtl " << named_triangles.material_name << '\n';
        }
        for (const auto& t : named_triangles.triangles) {
            ostr << "f ";
            size_t i = 0;
            for (const auto& v : t.flat_iterable()) {
                if (i++ != 0) {
                    ostr << ' ';
                }
                ostr << (v.position + 1) << '/' << (v.uv + 1) << '/' << (v.normal + 1);
            }
            ostr << '\n';
        }
    }
    ostr.flush();
    if (ostr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}

void save_obj(
    const std::string& filename,
    const std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
    const std::function<ObjMaterial(const Material&)>& convert_material);

}
