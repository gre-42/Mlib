#include "Load_Scene.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Physics_Scenes.hpp>
#ifndef WITHOUT_GRAPHICS
#include <Mlib/Scene/Renderable_Scenes.hpp>
#endif

using namespace Mlib;
using namespace LoadSceneFuncs;

LoadScene::LoadScene(
    const std::vector<Utf8Path>& search_path,
    const std::string& script_filename,
    ThreadSafeString& next_scene_filename,
    LocalSceneLevel scene_level,
    NotifyingJsonMacroArguments& external_json_macro_arguments,
    bool verbose,
    SurfaceContactDb& surface_contact_db,
    BulletPropertyDb& bullet_property_db,
    DynamicLightDb& dynamic_light_db,
    SceneConfig& scene_config,
    Users& users,
    RemoteSites& remote_sites,
    AssetReferences& asset_references,
    Translators& translators,
    PhysicsScenes& physics_scenes,
    #ifndef WITHOUT_GRAPHICS
    std::atomic_size_t& num_renderings,
    RealtimeDependentFps& render_set_fps,
    ButtonStates& button_states,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    VerboseVector<ButtonPress>& confirm_button_press,
    LockableKeyConfigurations& key_configurations,
    LockableKeyDescriptions& key_descriptions,
    UiFocuses& ui_focuses,
    LayoutConstraints& layout_constraints,
    RenderLogicGallery& gallery,
    RenderableScenes& renderable_scenes,
    WindowLogic& window_logic,
    #endif
    const std::function<void()>& exit)
    : json_user_function_{ [&](
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments,
        JsonMacroArguments* local_json_macro_arguments)
    {
        auto physics_scene = [&]() -> PhysicsScene& {
            return physics_scenes[context];
        };
        #ifndef WITHOUT_GRAPHICS
        auto renderable_scene = [&]() -> RenderableScene& {
            return renderable_scenes[context];
        };
        #endif
        LoadSceneJsonUserFunctionArgs args{
            .name = name,
            .arguments = arguments,
            .physics_scene = physics_scene,
            .macro_line_executor = macro_line_executor,
            .scene_level_selector = scene_level_selector_,
            .external_json_macro_arguments = external_json_macro_arguments,
            .local_json_macro_arguments = local_json_macro_arguments,
            .surface_contact_db = surface_contact_db,
            .bullet_property_db = bullet_property_db,
            .dynamic_light_db = dynamic_light_db,
            .scene_config = scene_config,
            .users = users,
            .remote_sites = remote_sites,
            .script_filename = script_filename,
            .next_scene_filename = next_scene_filename,
            .asset_references = asset_references,
            .translators = translators,
            .physics_scenes = physics_scenes,
            #ifndef WITHOUT_GRAPHICS
            .renderable_scene = renderable_scene,
            .button_states = button_states,
            .cursor_states = cursor_states,
            .scroll_wheel_states = scroll_wheel_states,
            .confirm_button_press = confirm_button_press,
            .key_configurations = key_configurations,
            .key_descriptions = key_descriptions,
            .ui_focuses = ui_focuses,
            .layout_constraints = layout_constraints,
            .num_renderings = num_renderings,
            .render_set_fps = render_set_fps,
            .gallery = gallery,
            .renderable_scenes = renderable_scenes,
            .window_logic = window_logic,
            #endif
            .exit = exit};
        auto* func = try_get_json_user_function(args.name);
        if (func == nullptr) {
            return false;
        }
        (*func)(args);
        return true;
    }}
    , macro_line_executor_{
        macro_file_executor_,
        script_filename,
        search_path,
        json_user_function_,
        "no_scene_specified",
        nlohmann::json::object(),
        external_json_macro_arguments,
        asset_references,
        verbose}
    #ifndef WITHOUT_GRAPHICS
    , focus_finalizer_{ ui_focuses, macro_line_executor_ }
    #endif
    , scene_level_selector_{
        std::move(scene_level),
        [this](){
            macro_line_executor_({{"playback", "remote.level.load_" + scene_level_selector_.get_next_scene_name()}}, nullptr);
        }}
{}

LoadScene::~LoadScene() = default;

void LoadScene::operator () () {
    macro_file_executor_(macro_line_executor_);
}

LocalSceneLevel LoadScene::scene_level() const {
    return scene_level_selector_.get_next_scene_level();
}

void LoadScene::notify_level_loaded() {
    scene_level_selector_.notify_level_loaded();
}

bool LoadScene::level_loaded() const {
    switch (scene_level_selector_.load_status()) {
    case LocalSceneLevelLoadStatus::LOADING:
        return false;
    case LocalSceneLevelLoadStatus::RUNNING:
        return true;
    }
    throw std::runtime_error("Unknown level loaded status");
}
