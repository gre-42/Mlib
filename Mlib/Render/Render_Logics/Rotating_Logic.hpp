#pragma once
#ifndef __ANDROID__
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <atomic>
#include <vector>

namespace Mlib {

class Scene;
class RotatingLogicKeys;

class RotatingLogicUserClass: public WindowPosition {
public:
    float scale = 1;
    float camera_z = 0;
    float angle_x = 0;
    float angle_y = 0;
    const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations;
    std::atomic_size_t beacon_index;
};

class RotatingLogic: public RenderLogic {
public:
    explicit RotatingLogic(
        ButtonStates& button_states,
        GLFWwindow& window,
        const Scene& scene,
        bool rotate,
        float scale,
        float camera_z,
        const FixedArray<float, 3>& background_color,
        const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations);
    ~RotatingLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    GLFWwindow& window_;
    const Scene& scene_;
    ButtonStates& button_states_;
    RotatingLogicUserClass user_object_;
    bool rotate_;
    FixedArray<float, 3> background_color_;
    std::unique_ptr<RotatingLogicKeys> keys_;
};

}

#endif
