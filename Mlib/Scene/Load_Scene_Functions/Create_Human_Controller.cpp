#include "Create_Human_Controller.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Misc/Human_Controller.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneInstanceFunction::UserFunction CreateHumanController::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_human_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+angular_velocity=([\\w+-.]+)"
        "\\s+steering_multiplier=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateHumanController(renderable_scene()).execute(
            match,
            fpath,
            macro_line_executor,
            local_substitutions,
            rsc);
        return true;
    } else {
        return false;
    }
};

CreateHumanController::CreateHumanController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHumanController::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    auto node = scene.get_node(match[1].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    if (rb->controller_ != nullptr) {
        throw std::runtime_error("Human controller already set");
    }
    rb->controller_ = std::make_unique<HumanController>(
        rb,
        float(M_PI) / 180.f * safe_stof(match[2].str()),
        safe_stof(match[3].str()));
}
