#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <memory>

namespace Mlib {

class Camera: public DanglingBaseClass {
    Camera(const Camera&) = delete;
    Camera& operator = (const Camera&) = delete;
public:
    Camera();
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
