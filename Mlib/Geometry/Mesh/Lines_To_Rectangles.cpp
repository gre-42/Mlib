#include "Lines_To_Rectangles.hpp"
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

/* From: https://en.wikipedia.org/wiki/List_of_trigonometric_identities#Half-angle_formulae
 */
static double half_angle_cos(double cos) {
    return std::sqrt((1 + cos) / 2);
}

static FixedArray<CompressedScenePos, 2> intersect_street_lines(
    const FixedArray<CompressedScenePos, 2, 2>& l0,
    const FixedArray<CompressedScenePos, 2, 2>& l1,
    const CompressedScenePos& width0,
    const CompressedScenePos& width1)
{
    return intersect_lines(
        funpack(l0),
        funpack(l1),
        funpack(width0),
        funpack(width1),
        true // compute_center
    ).casted<CompressedScenePos>();
}

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool Mlib::lines_to_rectangles(
    FixedArray<CompressedScenePos, 2>& p00,
    FixedArray<CompressedScenePos, 2>& p01,
    FixedArray<CompressedScenePos, 2>& p10,
    FixedArray<CompressedScenePos, 2>& p11,
    const FixedArray<CompressedScenePos, 2>& aL,
    const FixedArray<CompressedScenePos, 2>& aR,
    const FixedArray<CompressedScenePos, 2>& b,
    const FixedArray<CompressedScenePos, 2>& c,
    const FixedArray<CompressedScenePos, 2>& dL,
    const FixedArray<CompressedScenePos, 2>& dR,
    CompressedScenePos width_aLb,
    CompressedScenePos width_aRb,
    CompressedScenePos width_bcL,
    CompressedScenePos width_bcR,
    CompressedScenePos width_cdL,
    CompressedScenePos width_cdR)
{
    // Calculate normals.
    FixedArray<double, 2> n_aLb = FixedArray<CompressedScenePos, 2>{b(1) - aL(1), aL(0) - b(0)}.casted<double>();
    FixedArray<double, 2> n_aRb = FixedArray<CompressedScenePos, 2>{b(1) - aR(1), aR(0) - b(0)}.casted<double>();
    FixedArray<double, 2> n_bc = FixedArray<CompressedScenePos, 2>{c(1) - b(1), b(0) - c(0)}.casted<double>();
    FixedArray<double, 2> n_cdL = FixedArray<CompressedScenePos, 2>{dL(1) - c(1), c(0) - dL(0)}.casted<double>();
    FixedArray<double, 2> n_cdR = FixedArray<CompressedScenePos, 2>{dR(1) - c(1), c(0) - dR(0)}.casted<double>();

    // Handle special case of line endings (a or d do not exist).
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
    FixedArray<double, 2> n_bL = (n_aLb + n_bc) / 2.;
    FixedArray<double, 2> n_bR = (n_aRb + n_bc) / 2.;
    FixedArray<double, 2> n_cL = (n_cdL + n_bc) / 2.;
    FixedArray<double, 2> n_cR = (n_cdR + n_bc) / 2.;

    // Rescale normals after averaging.

    // Good approximation, however using exact formula below instead.
    // n_bL *= (width_aLb + width_bcL) / 2 / sum(squared(n_bL));
    // n_bR *= (width_aRb + width_bcR) / 2 / sum(squared(n_bR));
    // n_cL *= (width_cdL + width_bcL) / 2 / sum(squared(n_cL));
    // n_cR *= (width_cdR + width_bcR) / 2 / sum(squared(n_cR));

    n_bL *= funpack(width_aLb + width_bcL) / 2 / std::sqrt(sum(squared(n_bL))) / half_angle_cos(std::abs(dot0d(n_aLb, n_bc)));
    n_bR *= funpack(width_aRb + width_bcR) / 2 / std::sqrt(sum(squared(n_bR))) / half_angle_cos(std::abs(dot0d(n_aRb, n_bc)));
    n_cL *= funpack(width_cdL + width_bcL) / 2 / std::sqrt(sum(squared(n_cL))) / half_angle_cos(std::abs(dot0d(n_cdL, n_bc)));
    n_cR *= funpack(width_cdR + width_bcR) / 2 / std::sqrt(sum(squared(n_cR))) / half_angle_cos(std::abs(dot0d(n_cdR, n_bc)));

    if ((sum(squared(n_bL)) > squared(20 * funpack(width_bcL))) ||
        (sum(squared(n_bR)) > squared(20 * funpack(width_bcR))) ||
        (sum(squared(n_cL)) > squared(20 * funpack(width_bcL))) ||
        (sum(squared(n_cR)) > squared(20 * funpack(width_bcR))))
    {
        return false;
    }

    // Set rectangle points.
    p00 = b - (n_bL / 2.).casted<CompressedScenePos>();
    p01 = b + (n_bR / 2.).casted<CompressedScenePos>();
    p10 = c - (n_cL / 2.).casted<CompressedScenePos>();
    p11 = c + (n_cR / 2.).casted<CompressedScenePos>();

    // return true;

    // lerr();
    // lerr() << aL;
    // lerr() << aR;
    // lerr() << b;
    // lerr() << c;
    // lerr() << dL;
    // lerr() << dR;
    auto ZERO = (CompressedScenePos)0.f;
    if (width_aLb == ZERO && width_bcL == ZERO) {
        p00 = b;
    } else if (std::abs(dot0d(n_aLb, n_bc)) < std::cos(M_PI / 8)) {
        p00 = intersect_street_lines({aL, b}, {b, c}, width_aLb, width_bcL);
    }
    if (width_aRb == ZERO && width_bcR == ZERO) {
        p01 = b;
    } else if (std::abs(dot0d(n_aRb, n_bc)) < std::cos(M_PI / 8)) {
        p01 = intersect_street_lines({aR, b}, {b, c}, -width_aRb, -width_bcR);
    }
    if (width_bcL == ZERO && width_cdL == ZERO) {
        p10 = c;
    } else if (std::abs(dot0d(n_cdL, n_bc)) < std::cos(M_PI / 8)) {
        p10 = intersect_street_lines({b, c}, {c, dL}, width_bcL, width_cdL);
    }
    if (width_bcR == ZERO && width_cdR == ZERO) {
        p11 = c;
    } else if (std::abs(dot0d(n_cdR, n_bc)) < std::cos(M_PI / 8)) {
        p11 = intersect_street_lines({b, c}, {c, dR}, -width_bcR, -width_cdR);
    }

    return true;
}
