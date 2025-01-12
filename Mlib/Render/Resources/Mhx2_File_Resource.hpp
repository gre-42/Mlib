#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>

namespace Mlib {

template <class TPos>
struct LoadMeshConfig;
struct Bone;
struct AnimatedColoredVertexArrays;
class RenderingResources;
class ColoredVertexArrayResource;
template <typename TData, size_t... tshape>
class FixedArray;

template <class TDir, class TPos>
class OffsetAndQuaternion;

class Mhx2FileResource: public ISceneNodeResource {
public:
    Mhx2FileResource(
        const std::string& filename,
        const LoadMeshConfig<float>& cfg);
    ~Mhx2FileResource();

    // Misc
    const Bone& skeleton() const;
    UUVector<OffsetAndQuaternion<float, float>> vectorize_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) const;

    // ISceneNodeResource, Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual AggregateMode get_aggregate_mode() const override;

    // ISceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
    virtual void set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) override;

    // ISceneNodeResource, Modifiers
    virtual void downsample(size_t n) override;
private:
    std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
    std::shared_ptr<ColoredVertexArrayResource> rva_;
};

}
