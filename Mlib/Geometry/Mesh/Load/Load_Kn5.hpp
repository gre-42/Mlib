#pragma once
#include <Mlib/Geometry/Mesh/Load/Kn5_Elements.hpp>

namespace Mlib {

kn5Model load_kn5(
    std::istream& binStream,
    bool verbose = false,
    kn5LoadOptions opts = kn5LoadOptions::ALL);
kn5Model load_kn5(
    const std::string& filename,
    bool verbose = false,
    kn5LoadOptions opts = kn5LoadOptions::ALL);

}
