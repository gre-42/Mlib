#pragma once
#include <list>
#include <memory>

namespace Proctree {

class Tree;

}

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
struct Material;

std::list<std::shared_ptr<ColoredVertexArray<float>>> proctree_to_cvas(
    const Proctree::Tree& tree,
    const Material& tree_material,
    const Material& twig_material);

}
