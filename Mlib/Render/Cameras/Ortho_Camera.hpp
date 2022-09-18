#pragma once
#include <Mlib/Geometry/Cameras/Ortho_Camera_Config.hpp>
#include <Mlib/Scene_Graph/Elements/Camera.hpp>
#include <shared_mutex>

namespace Mlib {

class OrthoCamera: public Camera {
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
    virtual FixedArray<float, 4, 4> projection_matrix() override;
    virtual float get_near_plane() const override;
    virtual float get_far_plane() const override;
    virtual void set_requires_postprocessing(bool value) override;
    virtual bool get_requires_postprocessing() const override;

    void set_near_plane(float near_plane);
    void set_far_plane(float far_plane);
    void set_left_plane(float left_plane);
    void set_right_plane(float right_plane);
    void set_bottom_plane(float bottom_plane);
    void set_top_plane(float top_plane);
private:
    OrthoCameraConfig cfg_;
    Postprocessing postprocessing_;
    mutable std::shared_mutex mutex_;
};

}
