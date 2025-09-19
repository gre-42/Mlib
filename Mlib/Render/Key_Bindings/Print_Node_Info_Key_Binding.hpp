#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <cstddef>
#include <functional>

namespace Mlib {

class SceneNode;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct PrintNodeInfoKeyBinding {
    std::function<DanglingBaseClassPtr<SceneNode>()> dynamic_node;
    ButtonPress button_press;
    const TransformationMatrix<double, double, 3>* geographic_mapping;
    DestructionFunctionsRemovalTokens on_destroy_key_bindings;
};

}
