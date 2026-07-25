#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <cstddef>

namespace Mlib {

template <class T>
class VariableAndHash;
struct LoadSceneJsonUserFunctionArgs;
enum class ExtremalBoundingVolume;

class SetParticleRenderer: public LoadPhysicsSceneInstanceFunction {
public:
    explicit SetParticleRenderer(PhysicsScene& physics_scene);
    void operator () (
        const VariableAndHash<std::string>& node,
        const VariableAndHash<std::string>& renderable,
        ExtremalBoundingVolume bounding_volume) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
