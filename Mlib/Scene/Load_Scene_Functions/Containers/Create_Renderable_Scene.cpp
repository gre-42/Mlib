#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Render_Logics/Bloom/Bloom_Mode.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Physics_Scenes.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/Renderable_Scenes.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(fly);
DECLARE_ARGUMENT(rotate);
DECLARE_ARGUMENT(depth_fog);
DECLARE_ARGUMENT(low_pass);
DECLARE_ARGUMENT(high_pass);
DECLARE_ARGUMENT(bloom_iterations);
DECLARE_ARGUMENT(bloom_thresholds);
DECLARE_ARGUMENT(bloom_std);
DECLARE_ARGUMENT(bloom_intensities);
DECLARE_ARGUMENT(bloom_mode);
DECLARE_ARGUMENT(with_skybox);
DECLARE_ARGUMENT(with_flying_logic);
DECLARE_ARGUMENT(clear_mode);
DECLARE_ARGUMENT(physics);
DECLARE_ARGUMENT(layout);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
DECLARE_ARGUMENT(full_user_name);
DECLARE_ARGUMENT(local_user_id);
}

using namespace Mlib;

enum class SceneLayout {
    CHILD,
    FULL_SCREEN,
    TILED
};

SceneLayout scene_layout_from_string(const std::string& s) {
    static const std::map<std::string, SceneLayout> m{
        {"child", SceneLayout::CHILD},
        {"full_screen", SceneLayout::FULL_SCREEN},
        {"tiled", SceneLayout::TILED}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown scene layout: \"" + s + '"');
    }
    return it->second;
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_renderable_scene",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                auto layout = scene_layout_from_string(args.arguments.at<std::string>(KnownArgs::layout));
                auto name = args.arguments.at<std::string>(KnownArgs::name);
                auto& physics_scene = args.physics_scenes[args.arguments.at<std::string>(KnownArgs::physics)];
                auto local_user_id = args.arguments.at<uint32_t>(KnownArgs::local_user_id);
                auto remote_observer = RemoteObserver::all();
                if (auto full_user_name = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::full_user_name);
                    full_user_name.has_value())
                {
                    auto user = args.remote_sites.get_user(*full_user_name);
                    remote_observer = { user->site_id, user->user_id };
                }
                auto [_, state] = args.renderable_scenes.try_emplace(
                    name,
                    name,
                    DanglingBaseClassRef<PhysicsScene>{ physics_scene, CURRENT_SOURCE_LOCATION },
                    args.scene_config,
                    args.button_states,
                    args.cursor_states,
                    args.scroll_wheel_states,
                    args.key_configurations,
                    layout == SceneLayout::FULL_SCREEN
                        ? physics_scene.ui_focus_
                        : args.ui_focuses[local_user_id],
                    FocusFilter{
                        .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
                        .submenu_ids = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::submenus, {})},
                    remote_observer,
                    SceneConfigResource{
                        .fly = args.arguments.at<bool>(KnownArgs::fly),
                        .rotate = args.arguments.at<bool>(KnownArgs::rotate),
                        .depth_fog = args.arguments.at<bool>(KnownArgs::depth_fog),
                        .low_pass = args.arguments.at<bool>(KnownArgs::low_pass),
                        .high_pass = args.arguments.at<bool>(KnownArgs::high_pass),
                        .bloom_iterations = args.arguments.at<EFixedArray<unsigned int, 2>>(KnownArgs::bloom_iterations),
                        .bloom_thresholds = args.arguments.at<EFixedArray<float, 3>>(KnownArgs::bloom_thresholds),
                        .bloom_std = args.arguments.at<EFixedArray<float, 2>>(KnownArgs::bloom_std, 3.f),
                        .bloom_intensities = args.arguments.at<EFixedArray<float, 3>>(KnownArgs::bloom_intensities, 0.f),
                        .bloom_mode = bloom_mode_from_string(args.arguments.at<std::string>(KnownArgs::bloom_mode, "sky")),
                        .with_skybox = args.arguments.at<bool>(KnownArgs::with_skybox),
                        .with_flying_logic = args.arguments.at<bool>(KnownArgs::with_flying_logic),
                        .background_color = {1.f, 0.f, 1.f},
                        .clear_mode = clear_mode_from_string(args.arguments.at<std::string>(KnownArgs::clear_mode))});
                if (state == InsertionStatus::FAILURE_NAME_COLLISION) {
                    THROW_OR_ABORT("Scene with name \"" + name + "\" already exists");
                }
                if (state == InsertionStatus::FAILURE_SHUTDOWN) {
                    THROW_OR_ABORT("Attempt to create scene with name \"" + name + "\" during shutdown");
                }
                if (state != InsertionStatus::SUCCESS) {
                    THROW_OR_ABORT("Unknown state after creating scene with name \"" + name + '"');
                }
                switch (layout) {
                    case SceneLayout::CHILD:
                        // Do nothing
                        break;
                    case SceneLayout::FULL_SCREEN:
                        args.renderable_scenes.add_fullscreen_scene(name);
                        break;
                    case SceneLayout::TILED:
                        args.renderable_scenes.add_tiled_scene(
                            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
                            name);
                        break;
                    default:
                        THROW_OR_ABORT("Unknown scene layout");
                }
            });
    }
} obj;

}
