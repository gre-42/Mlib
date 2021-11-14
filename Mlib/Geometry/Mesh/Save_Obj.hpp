#pragma once
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <fstream>
#include <string>

namespace Mlib {

template <class TData, class TIndex>
void save_obj(const std::string& filename, const IndexedFaceSet<TData, TIndex>& data) {
    std::ofstream ostr(filename);
    ostr.precision(15);
    ostr << std::scientific;
    for (const auto v : data.vertices) {
        ostr << "v " << v << '\n';
    }
    for (const auto n : data.normals) {
        ostr << "vn " << n << '\n';
    }
    for (const NamedObjTriangles<TIndex>& named_triangles : data.named_obj_triangles) {
        if (!named_triangles.name.empty()) {
            ostr << "g " << named_triangles.name << '\n';
        }
        for (const auto& t : named_triangles.triangles) {
            ostr << "f ";
            size_t i = 0;
            for (const auto& v : t.flat_iterable()) {
                if (i++ != 0) {
                    ostr << ' ';
                }
                ostr << (v.position + 1) << '/' << '/' << (v.normal + 1);
            }
            ostr << '\n';
        }
    }
    ostr.flush();
    if (ostr.fail()) {
        throw std::runtime_error("Could not write to file " + filename);
    }
}

}
