#include "Create_Human_As_Car_Controller.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Human_As_Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(ANGULAR_VELOCITY);
DECLARE_OPTION(STEERING_MULTIPLIER);

LoadSceneInstanceFunction::UserFunction CreateHumanAsCarController::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_human_as_car_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+angular_velocity=([\\w+-.]+)"
        "\\s+steering_multiplier=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateHumanAsCarController(renderable_scene()).execute(
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

CreateHumanAsCarController::CreateHumanAsCarController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateHumanAsCarController::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    auto node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Human controller already set");
    }
    rb->vehicle_controller_ = std::make_unique<HumanAsCarController>(
        rb,
        float(M_PI) / 180.f * safe_stof(match[ANGULAR_VELOCITY].str()),
        safe_stof(match[STEERING_MULTIPLIER].str()));
}
