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
    for (const auto& t : data.triangles) {
        ostr << "f " << (t + TIndex(1)) << '\n';
    }
    ostr.flush();
    if (ostr.fail()) {
        throw std::runtime_error("Could not write to file " + filename);
    }
}

}
