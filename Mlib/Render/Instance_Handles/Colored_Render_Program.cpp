#include "Colored_Render_Program.hpp"
#include <Mlib/Compare_Vectors.hpp>

using namespace Mlib;

std::strong_ordering operator <=> (const std::vector<size_t>& a, const std::vector<size_t>& b) {
    return compare_vectors(a, b);
}

std::partial_ordering RenderProgramIdentifier::operator <=> (const RenderProgramIdentifier&) const = default;
