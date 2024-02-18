#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <memory>

namespace Mlib {

class Camera {
protected:
    Camera();
    Camera(const Camera&);
    Camera& operator = (const Camera&);
public:
    virtual ~Camera();
    virtual std::unique_ptr<Camera> copy() const = 0;
    virtual void set_aspect_ratio(float aspect_ratio);
    virtual FixedArray<float, 4, 4> projection_matrix() const = 0;
    virtual float get_near_plane() const;
    virtual float get_far_plane() const;
    virtual void set_requires_postprocessing(bool value);
    virtual bool get_requires_postprocessing() const;
};

}
