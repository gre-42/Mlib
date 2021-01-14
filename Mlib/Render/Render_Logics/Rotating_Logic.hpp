#pragma once
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;

class RotatingLocigUserClass: public BaseUserObject {
public:
    float scale = 1;
    float camera_z = 5;
    float angle_x = 0;
    float angle_y = 0;
};

class RotatingLogic: public RenderLogic {
public:
    explicit RotatingLogic(GLFWwindow* window, const Scene& scene, bool rotate, float scale);

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
private:
    const Scene& scene_;
    RotatingLocigUserClass user_object_;
    bool rotate_;
    float scale_;
};

}
