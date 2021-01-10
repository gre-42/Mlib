#include "Lines_To_Rectangles.hpp"
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool Mlib::lines_to_rectangles(
    FixedArray<float, 2>& p00,
    FixedArray<float, 2>& p01,
    FixedArray<float, 2>& p10,
    FixedArray<float, 2>& p11,
    const FixedArray<float, 2>& aL,
    const FixedArray<float, 2>& aR,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    const FixedArray<float, 2>& dL,
    const FixedArray<float, 2>& dR,
    float width_aLb,
    float width_aRb,
    float width_bc,
    float width_cdL,
    float width_cdR)
{
    // Calculate normals.
    FixedArray<float, 2> n_aLb = FixedArray<float, 2>{b(1) - aL(1), aL(0) - b(0)};
    FixedArray<float, 2> n_aRb = FixedArray<float, 2>{b(1) - aR(1), aR(0) - b(0)};
    FixedArray<float, 2> n_bc = FixedArray<float, 2>{c(1) - b(1), b(0) - c(0)};
    FixedArray<float, 2> n_cdL = FixedArray<float, 2>{dL(1) - c(1), c(0) - dL(0)};
    FixedArray<float, 2> n_cdR = FixedArray<float, 2>{dR(1) - c(1), c(0) - dR(0)};

    // Handle special case of line endings (a or d dont exist).
    if (all(n_aLb == -n_bc)) n_aLb = n_bc;
    if (all(n_aRb == -n_bc)) n_aRb = n_bc;
    if (all(n_cdL == -n_bc)) n_cdL = n_bc;
    if (all(n_cdR == -n_bc)) n_cdR = n_bc;

    // Normalize normals.
    n_aLb /= std::sqrt(sum(squared(n_aLb)));
    n_aRb /= std::sqrt(sum(squared(n_aRb)));
    n_bc /= std::sqrt(sum(squared(n_bc)));
    n_cdL /= std::sqrt(sum(squared(n_cdL)));
    n_cdR /= std::sqrt(sum(squared(n_cdR)));

    // Average normals.
    FixedArray<float, 2> n_bL = (n_aLb + n_bc) / 2.f;
    FixedArray<float, 2> n_bR = (n_aRb + n_bc) / 2.f;
    FixedArray<float, 2> n_cL = (n_cdL + n_bc) / 2.f;
    FixedArray<float, 2> n_cR = (n_cdR + n_bc) / 2.f;

    // Rescale normals after averaging.
    n_bL *= (width_aLb + width_bc) / 2 / sum(squared(n_bL));
    n_bR *= (width_aRb + width_bc) / 2 / sum(squared(n_bR));
    n_cL *= (width_cdL + width_bc) / 2 / sum(squared(n_cL));
    n_cR *= (width_cdR + width_bc) / 2 / sum(squared(n_cR));

    if ((sum(squared(n_bL)) > 3 * width_bc) ||
        (sum(squared(n_bR)) > 3 * width_bc) ||
        (sum(squared(n_cL)) > 3 * width_bc) ||
        (sum(squared(n_cR)) > 3 * width_bc))
    {
        return false;
    }

    // Set rectangle points.
    p00 = FixedArray<float, 2>{b(0) - n_bL(0) / 2, b(1) - n_bL(1) / 2};
    p01 = FixedArray<float, 2>{b(0) + n_bR(0) / 2, b(1) + n_bR(1) / 2};
    p10 = FixedArray<float, 2>{c(0) - n_cL(0) / 2, c(1) - n_cL(1) / 2};
    p11 = FixedArray<float, 2>{c(0) + n_cR(0) / 2, c(1) + n_cR(1) / 2};

    // return true;

    // std::cerr << std::endl;
    // std::cerr << aL << std::endl;
    // std::cerr << aR << std::endl;
    // std::cerr << b << std::endl;
    // std::cerr << c << std::endl;
    // std::cerr << dL << std::endl;
    // std::cerr << dR << std::endl;
    if (std::abs(dot0d(n_aLb, n_bc)) < std::cos(M_PI / 8)) {
        p00 = intersect_lines({aL, b}, {b, c}, width_aLb, width_bc, true);  // true = compute_center
    }
    if (std::abs(dot0d(n_aRb, n_bc)) < std::cos(M_PI / 8)) {
        p01 = intersect_lines({aR, b}, {b, c}, -width_aRb, -width_bc, true);  // true = compute_center
    }
    if (std::abs(dot0d(n_cdL, n_bc)) < std::cos(M_PI / 8)) {
        p10 = intersect_lines({b, c}, {c, dL}, width_bc, width_cdL, true);  // true = compute_center
    }
    if (std::abs(dot0d(n_cdR, n_bc)) < std::cos(M_PI / 8)) {
        p11 = intersect_lines({b, c}, {c, dR}, -width_bc, -width_cdR, true);  // true = compute_center
    }

    return true;
}
