#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

void colorize_triangles_by_physics_material(
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas);

}
