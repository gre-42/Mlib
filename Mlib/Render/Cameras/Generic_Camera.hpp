#pragma once
#include <Mlib/Scene_Graph/Camera.hpp>
#include <Mlib/Scene_Graph/Camera_Config.hpp>

namespace Mlib {

class GenericCamera: public Camera {
public:
    enum class Mode {
        ORTHO,
        PERSPECTIVE
    };
    explicit GenericCamera(const CameraConfig& cfg, const Mode& mode);
    void set_mode(const Mode& mode);
    virtual ~GenericCamera() override;
    virtual std::shared_ptr<Camera> copy() const override;
    virtual void set_y_fov(float y_fov) override;
    virtual void set_aspect_ratio(float aspect_ratio) override;
    virtual void set_near_plane(float near_plane) override;
    virtual void set_far_plane(float far_plane) override;
    virtual void set_left_plane(float left_plane) override;
    virtual void set_right_plane(float right_plane) override;
    virtual void set_bottom_plane(float bottom_plane) override;
    virtual void set_top_plane(float top_plane) override;
    virtual FixedArray<float, 4, 4> projection_matrix() override;
    virtual float get_near_plane() const override;
    virtual float get_far_plane() const override;
    virtual void set_requires_postprocessing(bool value) override;
    virtual bool get_requires_postprocessing() const override;
private:
    CameraConfig cfg_;
    bool requires_postprocessing_;
    Mode mode_;
};

}
