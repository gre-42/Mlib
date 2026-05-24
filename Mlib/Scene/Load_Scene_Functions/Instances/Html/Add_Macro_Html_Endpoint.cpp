#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Remote/Config_Server/Index_Http_Response_Generator.hpp>
#include <list>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(path);
DECLARE_ARGUMENT(content);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_macro_html_endpoint",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                args.index_html.add_function(
                    args.arguments.at<std::string>(KnownArgs::path),
                    [mle=args.macro_line_executor,
                     content=args.arguments.at(KnownArgs::content)](){
                        mle(content, nullptr);
                        return nlohmann::json("ok");
                    });
            });
    }
} obj;

}
