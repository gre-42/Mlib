#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class T>
class VariableAndHash;
struct LoadSceneJsonUserFunctionArgs;

class EmitChildParticle: public LoadPhysicsSceneInstanceFunction {
public:
    explicit EmitChildParticle(PhysicsScene& physics_scene);
    void operator () (
        const VariableAndHash<std::string>& node,
        const VariableAndHash<std::string>& resource,
        const TransformationMatrix<SceneDir, ScenePos, 3>& location) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
