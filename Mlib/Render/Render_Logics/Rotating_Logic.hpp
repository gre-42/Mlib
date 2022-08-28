#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <atomic>
#include <vector>

namespace Mlib {

class Scene;

class RotatingLogicUserClass: public WindowPosition {
public:
    float scale = 1;
    float camera_z = 0;
    float angle_x = 0;
    float angle_y = 0;
    const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations;
    std::atomic_size_t beacon_index;
};

class RotatingLogic: public RenderLogic {
public:
    explicit RotatingLogic(
        ButtonStates& button_states,
        GLFWwindow* window,
        const Scene& scene,
        bool rotate,
        float scale,
        float camera_z,
        const FixedArray<float, 3>& background_color,
        const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<double, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, double, 3>& iv() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    ButtonPress button_press_;
    GLFWwindow* window_;
    const Scene& scene_;
    RotatingLogicUserClass user_object_;
    bool rotate_;
    FixedArray<float, 3> background_color_;
};

}
