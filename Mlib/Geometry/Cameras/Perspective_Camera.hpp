#pragma once
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera_Config.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>

namespace Mlib {

class PerspectiveCamera final: public Camera {
public:
    enum class Postprocessing {
        DISABLED = 0,
        ENABLED = 1
    };
    explicit PerspectiveCamera(
        const PerspectiveCameraConfig& cfg,
        Postprocessing postprocessing);
    virtual ~PerspectiveCamera() override;
    virtual std::unique_ptr<Camera> copy() const override;
    virtual void set_aspect_ratio(float aspect_ratio) override;
    virtual FixedArray<float, 4, 4> projection_matrix() const override;
    virtual float get_near_plane() const override;
    virtual float get_far_plane() const override;
    virtual void set_requires_postprocessing(bool value) override;
    virtual bool get_requires_postprocessing() const override;

    void set_y_fov(float y_fov);
    void set_near_plane(float near_plane);
    void set_far_plane(float far_plane);
private:
    PerspectiveCameraConfig cfg_;
    Postprocessing postprocessing_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
