#pragma once
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <atomic>
#include <vector>

namespace Mlib {

class Scene;

class RotatingLogicUserClass: public BaseUserObject {
public:
    float scale = 1;
    float camera_z = 0;
    float angle_x = 0;
    float angle_y = 0;
    const std::vector<TransformationMatrix<float, 3>>* beacon_locations;
    std::atomic_size_t beacon_index;
};

class RotatingLogic: public RenderLogic {
public:
    explicit RotatingLogic(
        GLFWwindow* window,
        const Scene& scene,
        bool rotate,
        float scale,
        float camera_z,
        const std::vector<TransformationMatrix<float, 3>>* beacon_locations);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, 3>& iv() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Scene& scene_;
    RotatingLogicUserClass user_object_;
    bool rotate_;
};

}
