#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool lines_to_rectangles(
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
    CompressedScenePos width_cdR);

}
