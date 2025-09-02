#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
class ColoredVertexArray;
class ColoredVertexArrayResource;
class RenderableColoredVertexArray;
class DynamicInstanceBuffers;
enum class TransformationMode;
struct BillboardSequence;
struct Light;
struct Skidmark;
struct SceneGraphConfig;
struct RenderConfig;
struct RenderedSceneDescriptor;
enum class ParticleType;
struct StaticWorld;

class ParticlesInstance {
    ParticlesInstance(const ParticlesInstance&) = delete;
    ParticlesInstance& operator=(const ParticlesInstance&) = delete;

public:
    explicit ParticlesInstance(
        const std::shared_ptr<ColoredVertexArray<float>>& triangles,
        size_t max_num_instances,
        const RenderableResourceFilter& filter,
        ParticleType particle_type);
    ~ParticlesInstance();

    ParticleType particle_type() const;

    size_t num_billboard_atlas_components() const;

    void add_particle(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
        const BillboardSequence& sequence,
        const FixedArray<float, 3>& velocity,
        float air_resistance);

    void move(float dt, const StaticWorld& world);

    void preload() const;

    void render(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderedSceneDescriptor& frame_id) const;

private:
    FixedArray<ScenePos, 3> offset_;
    std::shared_ptr<DynamicInstanceBuffers> dynamic_instance_buffers_;
    std::shared_ptr<ColoredVertexArrayResource> cvar_;
    std::unique_ptr<RenderableColoredVertexArray> rcva_;
    RenderableResourceFilter filter_;
    ParticleType particle_type_;
    mutable FastMutex mutex_;
};

}
