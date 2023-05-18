#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticles_Instance.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
class ColoredVertexArrayResource;
class RenderableColoredVertexArray;
class DynamicInstanceBuffers;
enum class TransformationMode;

class ParticlesInstance: public IParticlesInstance {
public:
    explicit ParticlesInstance(
        const std::shared_ptr<ColoredVertexArray<float>>& triangles,
        size_t max_num_instances);
    ~ParticlesInstance();
    virtual void add_particle(
        const TransformationMatrix<float, double, 3>& transformation_matrix,
        const BillboardSequence& sequence) override;
    virtual void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;
private:
    FixedArray<double, 3> offset_;
    std::shared_ptr<DynamicInstanceBuffers> dynamic_instance_buffers_;
    std::shared_ptr<ColoredVertexArrayResource> cvar_;
    std::unique_ptr<RenderableColoredVertexArray> rcva_;
};

}
