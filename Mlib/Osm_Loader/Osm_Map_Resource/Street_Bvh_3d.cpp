#include "Street_Bvh_3d.hpp"

using namespace Mlib;

StreetBvh3d::StreetBvh3d()
    : bvh{ {(CompressedScenePos)100.f, (CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10 }
{}
