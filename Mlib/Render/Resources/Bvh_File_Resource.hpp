#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <memory>

namespace Mlib {

class BvhLoader;
struct BvhConfig;

class BvhFileResource: public SceneNodeResource {
public:
    BvhFileResource(
        const std::string& filename,
        const BvhConfig& config);
    ~BvhFileResource();
    virtual std::map<std::string, OffsetAndQuaternion<float, float>> get_poses(float seconds) const override;
    virtual float get_animation_duration() const override;
private:
    std::unique_ptr<BvhLoader> bvh_loader;
};

}
