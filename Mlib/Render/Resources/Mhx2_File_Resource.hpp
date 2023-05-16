#pragma once
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resource.hpp>
#include <vector>

namespace Mlib {

struct LoadMeshConfig;
struct Bone;
struct AnimatedColoredVertexArrays;
class RenderingResources;
class ColoredVertexArrayResource;
template <typename TData, size_t... tshape>
class FixedArray;

template <class TDir, class TPos>
class OffsetAndQuaternion;

class Mhx2FileResource: public SceneNodeResource {
public:
    Mhx2FileResource(
        const std::string& filename,
        const LoadMeshConfig& cfg);
    ~Mhx2FileResource();

    // Misc
    const Bone& skeleton() const;
    std::vector<OffsetAndQuaternion<float, float>> vectorize_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) const;

    // SceneNodeResource, Misc
    void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual AggregateMode aggregate_mode() const override;

    // SceneNodeResource, Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;
    virtual void set_relative_joint_poses(const std::map<std::string, OffsetAndQuaternion<float, float>>& poses) override;

    // SceneNodeResource, Modifiers
    virtual void downsample(size_t n) override;
private:
    std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
    std::shared_ptr<ColoredVertexArrayResource> rva_;
};

}
