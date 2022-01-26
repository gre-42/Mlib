#include "Create_Tank_Controller.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Tank_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneInstanceFunction::UserFunction CreateTankController::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_tank_controller"
        "\\s+node=([\\w+-.]+)"
        "\\s+left_tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+right_tire_ids=((?:\\d+)?(?:\\s+\\d+)*)"
        "\\s+steering_multiplier=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateTankController(renderable_scene()).execute(
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

CreateTankController::CreateTankController(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateTankController::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    auto node = scene.get_node(match[1].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Tank movable is not a rigid body");
    }
    if (rb->vehicle_controller_ != nullptr) {
        throw std::runtime_error("Tank controller already set");
    }
    std::vector<size_t> left_tire_ids = string_to_vector(match[2].str(), safe_stoz);
    std::vector<size_t> right_tire_ids = string_to_vector(match[3].str(), safe_stoz);
    rb->vehicle_controller_ = std::make_unique<TankController>(
        rb,
        left_tire_ids,
        right_tire_ids,
        safe_stof(match[4].str()));
}
