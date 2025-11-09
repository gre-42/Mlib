#pragma once
#include <Mlib/Images/Ppm_Image.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Usage_Counter.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Scene.hpp>
#include <Mlib/Scene_Graph/Remote_User_Filter.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace Mlib {

class PhysicsScene;
class DirtmapLogic;
class PostProcessingLogic;
class FxaaLogic;
class BloomLogic;
class SkyBloomLogic;
class BloomSelectorLogic;
class MotionInterpolationLogic;
class FlyingCameraUserClass;
class StandardRenderLogic;
class AggregateRenderLogic;
class AudioListenerUpdater;

class ButtonStates;
class CursorStates;

class UiFocus;

enum class ThreadAffinity;
enum class ClearMode;
enum class BloomMode;

struct FocusFilter;
struct SceneConfig;

struct SceneConfigResource {
    bool fly;
    bool rotate;
    bool depth_fog;
    bool low_pass;
    bool high_pass;
    FixedArray<unsigned int, 2> bloom_iterations;
    FixedArray<float, 3> bloom_thresholds;
    FixedArray<float, 2> bloom_std;
    FixedArray<float, 3> bloom_intensities;
    BloomMode bloom_mode;
    bool with_skybox;
    bool with_flying_logic;
    FixedArray<float, 3> background_color;
    ClearMode clear_mode;
};

class RenderableScene: public RenderLogic, public IRenderableScene {
public:
    RenderableScene(
        std::string name,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        LockableKeyConfigurations& key_configurations,
        UiFocus& ui_focus,
        const FocusFilter& focus_filter,
        const RemoteObserver& remote_observer,
        const SceneConfigResource& config);
    ~RenderableScene();
    RenderableScene(const RenderableScene&) = delete;
    RenderableScene& operator = (const RenderableScene&) = delete;

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void wait_until_done() const;
    void stop_and_join();
    void shutdown();

    void instantiate_audio_listener(
        std::chrono::steady_clock::duration delay,
        std::chrono::steady_clock::duration velocity_dt);

    DestructionFunctionsRemovalTokens on_stop_and_join_physics_;
    DestructionFunctionsRemovalTokens on_destroy_physics_;
    ObjectPool object_pool_;
    std::optional<CounterUser> counter_user_;

    std::string name_;
    DanglingBaseClassPtr<PhysicsScene> physics_scene_;
    const SceneConfig& scene_config_;
    SelectedCameras selected_cameras_;
    FlyingCameraUserClass user_object_;
    UiFocus& ui_focus_;
    FocusFilter focus_filter_;
    RemoteObserver remote_observer_;

    RenderLogics render_logics_;
    RenderLogics scene_render_logics_;
    StandardCameraLogic standard_camera_logic_;
    SkyboxLogic skybox_logic_;
    std::unique_ptr<StandardRenderLogic> standard_render_logic_;
    std::unique_ptr<AggregateRenderLogic> aggregate_render_logic_;
    std::unique_ptr<FlyingCameraLogic> flying_camera_logic_;
    std::unique_ptr<KeyBindings> key_bindings_;
    ReadPixelsLogic read_pixels_logic_;
    std::unique_ptr<DirtmapLogic> dirtmap_logic_;
    std::unique_ptr<MotionInterpolationLogic> motion_interp_logic_;
    std::unique_ptr<PostProcessingLogic> post_processing_logic_;
    std::unique_ptr<FxaaLogic> fxaa_logic_;
    std::unique_ptr<BloomLogic> bloom_logic_;
    std::unique_ptr<SkyBloomLogic> sky_bloom_logic_;
    std::unique_ptr<BloomSelectorLogic> bloom_selector_logic_;
    std::unique_ptr<RenderLogics> imposter_render_logics_;

    std::unique_ptr<AudioListenerUpdater> audio_listener_updater_;

    bool imposters_instantiated_;
    bool background_color_applied_;
};

}
