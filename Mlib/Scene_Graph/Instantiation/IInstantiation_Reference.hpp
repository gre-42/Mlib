#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class RenderingDynamics;
class SceneNode;
template <class T>
class DanglingUniquePtr;
class Renderable;

class IInstantiationReference {
public:
    virtual void add_renderable(
        const std::string& name,
        const std::shared_ptr<const Renderable>& renderable) = 0;
    virtual TransformationMatrix<float, ScenePos, 3> absolute_model_matrix() const = 0;
    virtual void auto_add_root_node(
        const std::string& name,
        std::unique_ptr<SceneNode>&& scene_node,
        RenderingDynamics rendering_dynamics) = 0;
};

}
