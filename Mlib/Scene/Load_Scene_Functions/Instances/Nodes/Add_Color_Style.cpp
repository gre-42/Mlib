#include "Add_Color_Style.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(selector);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(diffuse);
DECLARE_ARGUMENT(specular);
DECLARE_ARGUMENT(reflection_strength);
DECLARE_ARGUMENT(reflection_maps);
}

AddColorStyle::AddColorStyle(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void AddColorStyle::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    using ReflectionMaps = junordered_map<VariableAndHash<std::string>, VariableAndHash<std::string>>;
    ReflectionMaps parsed_reflection_maps;
    if (auto rm = args.arguments.try_at<ReflectionMaps>(KnownArgs::reflection_maps)) {
        parsed_reflection_maps = *rm;
    }
    auto style = std::unique_ptr<ColorStyle>(new ColorStyle{
        .selector = args.arguments.contains(KnownArgs::selector)
            ? std::optional{ Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::selector)) }
            : std::nullopt,
        .ambient = args.arguments.contains(KnownArgs::ambient)
            ? args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::ambient)
            : OrderableFixedArray(fixed_full<float, 3>(-1)),
        .diffuse = args.arguments.contains(KnownArgs::diffuse)
            ? args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::diffuse)
            : OrderableFixedArray(fixed_full<float, 3>(-1)),
        .specular = args.arguments.contains(KnownArgs::specular)
            ? args.arguments.at<UOrderableFixedArray<float, 3>>(KnownArgs::specular)
            : OrderableFixedArray(fixed_full<float, 3>(-1)),
        .reflection_maps = std::move(parsed_reflection_maps),
        .reflection_strength = args.arguments.at<float>(KnownArgs::reflection_strength, -1.f)});
    if (auto node = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::node); node.has_value()) {
        DanglingRef<SceneNode> n = scene.get_node(*node, DP_LOC);
        n->add_color_style(std::move(style));
    } else {
        scene.add_color_style(std::move(style));
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_color_style",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                AddColorStyle(args.physics_scene()).execute(args);            
            });
    }
} obj;

}
