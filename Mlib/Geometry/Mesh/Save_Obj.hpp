#pragma once
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Geometry/Mesh/Save_Mtllib.hpp>
#include <Mlib/Math/Funpack.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
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
        THROW_OR_ABORT("OBJ filename does not have \".obj\" extension: " + filename);
    }
    std::ofstream ostr{ filename };
    if constexpr (std::is_same_v<funpack_t<TPos>, double>) {
        ostr.precision(15);
    } else if constexpr (std::is_same_v<funpack_t<TPos>, float>) {
        ostr.precision(7);
    } else {
        THROW_OR_ABORT("Unsupported floating point type");
    }
    ostr << std::scientific;
    if (materials != nullptr) {
        std::string mtl_filename = match[1].str() + ".mtl";
        save_mtllib(mtl_filename, *materials);
        ostr << "mtllib " << fs::path(mtl_filename).filename().string()  << '\n';
    }
    for (const auto& v : data.vertices) {
        ostr << "v " << v << '\n';
    }
    for (const auto& uv : data.uvs) {
        ostr << "vt " << uv << '\n';
    }
    for (const auto& n : data.normals) {
        ostr << "vn " << n << '\n';
    }
    for (const NamedObjPolygons<TIndex>& named_polygons : data.named_obj_polygons) {
        if (!named_polygons.name.empty()) {
            ostr << "g " << named_polygons.name << '\n';
            ostr << "usemtl " << named_polygons.material_name << '\n';
        }
        auto add_polygons = [&ostr]<size_t tnvertices>(
            const UUVector<FixedArray<IndexVertex<TIndex>, tnvertices>>& polygons)
        {
            for (const auto& t : polygons) {
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
        };
        add_polygons(named_polygons.triangles);
        add_polygons(named_polygons.quads);
    }
    ostr.flush();
    if (ostr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

template <class TPos>
void save_obj(
    const std::string& filename,
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::function<std::string(const ColoredVertexArray<TPos>&)>& material_name = {},
    const std::function<ObjMaterial(const Material&)>& convert_material = {});

}
