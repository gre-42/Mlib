#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <memory>

namespace Mlib {

class BvhLoader;
struct BvhConfig;

class BvhFileResource: public ISceneNodeResource {
public:
    BvhFileResource(
        const std::string& filename,
        const BvhConfig& config);
    ~BvhFileResource();
    virtual void preload(const RenderableResourceFilter& filter) override;
    virtual UUVector<OffsetAndQuaternion<float, float>> get_relative_poses(
        float seconds,
        const StringWithHashUnorderedMap<uint32_t>& bone_indices) const override;
    virtual UUVector<OffsetAndQuaternion<float, float>> get_absolute_poses(
        float seconds,
        const StringWithHashUnorderedMap<uint32_t>& bone_indices) const override;
    virtual float get_animation_duration() const override;
private:
    std::unique_ptr<BvhLoader> bvh_loader;
};

}
