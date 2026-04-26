#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Dynamic_Lights/Animated_Dynamic_Light.hpp>
#include <Mlib/Physics/Dynamic_Lights/Constant_Dynamic_Light.hpp>
#include <Mlib/Physics/Dynamic_Lights/Dynamic_Light_Db.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

namespace KnownConfigArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(time);
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(distances);
DECLARE_ARGUMENT(intensities);
}

namespace Mlib {

void from_json(const nlohmann::json& j, AnimatedDynamicLightConfiguration& item) {
    JsonView jv{ j };
    jv.validate(KnownConfigArgs::options);

    item.time_to_color = {
        jv.at<std::vector<float>>(KnownConfigArgs::time),
        jv.at<UUVector<FixedArray<float, 3>>>(KnownConfigArgs::intensities) };
    item.squared_distance_to_intensity = {
        jv.at_vector<ScenePos>(KnownConfigArgs::distances, [](ScenePos v) { return squared(v); }),
        jv.at<std::vector<float>>(KnownConfigArgs::intensities),
        OutOfRangeBehavior::CLAMP};
}

}

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(properties);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_animated_dynamic_light_properties",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                args.dynamic_light_db.add(
                    args.arguments.at<std::string>(KnownArgs::name),
                    args.arguments.at<AnimatedDynamicLightConfiguration>(KnownArgs::properties));
            });
    }
} obj;

}
