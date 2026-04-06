#pragma once
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>

namespace Mlib {

class IGpuVertexData;
class IGpuInstanceBuffers;
template <class TData, size_t tndim>
class ExtremalAxisAlignedBoundingBox;
template <class TData, size_t tndim>
class ExtremalBoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IGpuVertexArray {
public:
    virtual ~IGpuVertexArray() = default;
    virtual void update_legacy() = 0;
    virtual void bind() const = 0;
    virtual bool copy_in_progress() const = 0;
    virtual bool initialized() const = 0;
    virtual void initialize() = 0;
    virtual void wait() const = 0;
    virtual std::shared_ptr<IGpuVertexData> vertices() = 0;
    virtual std::shared_ptr<IGpuInstanceBuffers> instances() = 0;
    virtual std::shared_ptr<const IGpuVertexData> vertices() const = 0;
    virtual std::shared_ptr<const IGpuInstanceBuffers> instances() const = 0;
    virtual const ExtremalAxisAlignedBoundingBox<float, 3>& aabb() const = 0;
    virtual const ExtremalBoundingSphere<float, 3>& bounding_sphere() const = 0;
    virtual void extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const = 0;
    virtual void extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const = 0;
    virtual void extend_aabb(
        const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const = 0;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const = 0;
    virtual std::string identifier() const = 0;
    virtual void print_stats(std::ostream& ostr) const = 0;
    inline void preload() {
        if (!initialized()) {
            initialize();
        }
        wait();
    }
};

}
