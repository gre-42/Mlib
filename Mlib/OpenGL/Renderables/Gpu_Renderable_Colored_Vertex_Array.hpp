#pragma once
#include <Mlib/OpenGL/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>

namespace Mlib {

class IGpuVertexData;
class IGpuInstanceBuffers;

class GpuRenderableColoredVertexArray: public IGpuVertexArray {
public:
    explicit GpuRenderableColoredVertexArray(
        std::shared_ptr<IGpuVertexData> vertices,
        std::shared_ptr<IGpuInstanceBuffers> instances);
    virtual void update_legacy() override;
    virtual void bind() const override;
    virtual bool copy_in_progress() const override;
    virtual bool initialized() const override;
    virtual void initialize() override;
    virtual void wait() const override;
    virtual std::shared_ptr<IGpuVertexData> vertices() override;
    virtual std::shared_ptr<IGpuInstanceBuffers> instances() override;
    virtual std::shared_ptr<const IGpuVertexData> vertices() const override;
    virtual std::shared_ptr<const IGpuInstanceBuffers> instances() const override;
    virtual const ExtremalAxisAlignedBoundingBox<float, 3>& aabb() const override;
    virtual const ExtremalBoundingSphere<float, 3>& bounding_sphere() const override;
    virtual void extend_aabb(ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual void extend_bounding_sphere(ExtremalBoundingSphere<CompressedScenePos, 3>& bounding_sphere) const override;
    virtual void extend_aabb(
        const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const override;
    virtual std::string identifier() const override;
    virtual void print_stats(std::ostream& ostr) const override;
private:
    mutable VertexArray va_;
    std::shared_ptr<IGpuVertexData> vertices_;
    std::shared_ptr<IGpuInstanceBuffers> instances_;
};

}
