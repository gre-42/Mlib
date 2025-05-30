#pragma once
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Descriptions_Fwd.hpp>
#include <atomic>
#include <functional>

namespace Mlib {

template <class T>
class VerboseVector;
class RenderLogicGallery;
class AssetReferences;
class Translators;
class PhysicsScenes;
class RenderableScenes;
class NotifyingJsonMacroArguments;
struct SceneConfig;
struct RenderConfig;
class ButtonStates;
class CursorStates;
class ButtonPress;
class UiFocuses;
class RenderLogics;
class ThreadSafeString;
class SurfaceContactDb;
class BulletPropertyDb;
class DynamicLightDb;
class LayoutConstraints;
struct RealtimeDependentFps;
class WindowLogic;

class LoadScene {
public:
    LoadScene();
    ~LoadScene();
    void operator () (
        const std::list<std::string>* search_path,
        const std::string& script_filename,
        ThreadSafeString& next_scene_filename,
        NotifyingJsonMacroArguments& external_json_macro_arguments,
        std::atomic_size_t& num_renderings,
        RealtimeDependentFps& render_set_fps,
        bool verbose,
        SurfaceContactDb& surface_contact_db,
        BulletPropertyDb& bullet_property_db,
        DynamicLightDb& dynamic_light_db,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        VerboseVector<ButtonPress>& confirm_button_press,
        LockableKeyConfigurations& key_configurations,
        LockableKeyDescriptions& key_descriptions,
        UiFocuses& ui_focuses,
        LayoutConstraints& layout_constraints,
        RenderLogicGallery& gallery,
        AssetReferences& asset_references,
        Translators& translators,
        PhysicsScenes& physics_scenes,
        RenderableScenes& renderable_scenes,
        WindowLogic& window_logic,
        const std::function<void()>& exit);
private:
    MacroRecorder macro_file_executor_;
};

}
