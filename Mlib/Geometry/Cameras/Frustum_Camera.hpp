#pragma once
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera_Config.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>

namespace Mlib {

class FrustumCamera: public Camera {
public:
    enum class Postprocessing {
        DISABLED = 0,
        ENABLED = 1
    };
    explicit FrustumCamera(
        const FrustumCameraConfig& cfg,
        Postprocessing postprocessing);
    virtual ~FrustumCamera() override;
    virtual std::unique_ptr<Camera> copy() const override;
    virtual FixedArray<float, 4, 4> projection_matrix() const override;
    virtual float get_near_plane() const override;
    virtual float get_far_plane() const override;
    virtual void set_requires_postprocessing(bool value) override;
    virtual bool get_requires_postprocessing() const override;

    void set_near_plane(float near_plane);
    void set_far_plane(float far_plane);
    void set_left(float left_plane);
    void set_right(float right_plane);
    void set_bottom(float bottom_plane);
    void set_top(float top_plane);
private:
    FrustumCameraConfig cfg_;
    Postprocessing postprocessing_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
