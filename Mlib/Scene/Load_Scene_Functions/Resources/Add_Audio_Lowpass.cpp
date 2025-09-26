#include <Mlib/Argument_List.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Geometry/Intersection/Interval_Json.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(gain);
DECLARE_ARGUMENT(high_gain);
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_audio_lowpass",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                AudioResourceContextStack::primary_audio_resources()->add_lowpass(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    AudioLowpassInformation{
                        args.arguments.at<float>(KnownArgs::gain),
                        args.arguments.at<float>(KnownArgs::high_gain)
                    });
            });
    }
} obj;

}
