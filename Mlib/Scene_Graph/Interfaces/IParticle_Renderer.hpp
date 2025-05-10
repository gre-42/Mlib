#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct Skidmark;
struct RenderConfig;
struct SceneGraphConfig;
struct ExternalRenderPass;
class IParticleCreator;
struct StaticWorld;

class IParticleRenderer: public Renderable, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    virtual ~IParticleRenderer() = default;
    virtual void preload(const VariableAndHash<std::string>& name) = 0;
    virtual IParticleCreator& get_instantiator(const VariableAndHash<std::string>& name) = 0;
};

}
