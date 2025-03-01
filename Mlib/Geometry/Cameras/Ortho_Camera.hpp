#pragma once
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Cameras/Ortho_Camera_Config.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>

namespace Mlib {

class OrthoCamera final: public Camera {
public:
    enum class Postprocessing {
        DISABLED = 0,
        ENABLED = 1
    };
    explicit OrthoCamera(
        const OrthoCameraConfig& cfg,
        Postprocessing postprocessing);
    virtual ~OrthoCamera() override;
    virtual std::unique_ptr<Camera> copy() const override;
    virtual FixedArray<float, 4, 4> projection_matrix() const override;
    virtual float get_near_plane() const override;
    virtual float get_far_plane() const override;
    virtual void set_requires_postprocessing(bool value) override;
    virtual bool get_requires_postprocessing() const override;

    float get_left_plane() const;
    float get_right_plane() const;
    float get_bottom_plane() const;
    float get_top_plane() const;

    void set_near_plane(float near_plane);
    void set_far_plane(float far_plane);
    void set_left_plane(float left_plane);
    void set_right_plane(float right_plane);
    void set_bottom_plane(float bottom_plane);
    void set_top_plane(float top_plane);

    FixedArray<float, 2> dpi(float texture_width, float texture_height) const;
    FixedArray<float, 2> grid(float texture_width, float texture_height) const;
private:
    OrthoCameraConfig cfg_;
    Postprocessing postprocessing_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
