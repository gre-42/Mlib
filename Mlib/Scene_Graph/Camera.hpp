#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <memory>

namespace Mlib {

class Camera {
public:
    virtual ~Camera() {}
    virtual std::shared_ptr<Camera> copy() const = 0;
    virtual void set_y_fov(float y_fov) {}
    virtual void set_aspect_ratio(float aspect_ratio) {}
    virtual void set_near_plane(float near_plane) {}
    virtual void set_far_plane(float far_plane) {}
    virtual void set_left_plane(float left_plane) {}
    virtual void set_right_plane(float right_plane) {}
    virtual void set_bottom_plane(float bottom_plane) {}
    virtual void set_top_plane(float top_plane) {}
    virtual FixedArray<float, 4, 4> projection_matrix() = 0;
    virtual float get_near_plane() const = 0;
    virtual float get_far_plane() const = 0;
    virtual void set_requires_postprocessing(bool value) = 0;
    virtual bool get_requires_postprocessing() const = 0;
};

}
