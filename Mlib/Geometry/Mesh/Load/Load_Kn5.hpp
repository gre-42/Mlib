#pragma once
#include <Mlib/Geometry/Mesh/Load/Kn5_Elements.hpp>
#include <Mlib/Os/Utf8_Path.hpp>

namespace Mlib {

kn5Model load_kn5(
    std::istream& binStream,
    bool verbose = false,
    kn5LoadOptions opts = kn5LoadOptions::ALL);
kn5Model load_kn5(
    const Utf8Path& filename,
    bool verbose = false,
    kn5LoadOptions opts = kn5LoadOptions::ALL);

}
