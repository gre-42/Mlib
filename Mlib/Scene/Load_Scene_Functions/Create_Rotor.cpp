#include "Create_Rotor.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controller.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneInstanceFunction::UserFunction CreateRotor::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*rotor"
        "\\s+rigid_body=([\\w+-.]+)"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+engine=(\\w+)"
        "\\s+power2lift=([\\w+-.]+)"
        "(?:\\s+max_align_to_gravity=([\\w+-.]+))?"
        "\\s+tire_id=(\\d+)$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateRotor(renderable_scene()).execute(
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

CreateRotor::CreateRotor(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRotor::execute(
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
        throw std::runtime_error("Car controller already set");
    }
    FixedArray<float, 3> position{
        safe_stof(match[2].str()),
        safe_stof(match[3].str()),
        safe_stof(match[4].str())};
    FixedArray<float, 3> rotation{
        safe_stof(match[5].str()) * float(M_PI / 180.f),
        safe_stof(match[6].str()) * float(M_PI / 180.f),
        safe_stof(match[7].str()) * float(M_PI / 180.f)};
    std::string engine = match[8].str();
    float power2lift = safe_stof(match[9].str());
    float max_align_to_gravity = match[10].matched
        ? safe_stof(match[10].str()) * float(M_PI / 180.f)
        : NAN;
    size_t tire_id = safe_stoz(match[11].str());
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    auto tp = rb->rotors_.insert({
        tire_id,
        Rotor{
            engine,
            TransformationMatrix<float, 3>{ r, position },
            power2lift,
            max_align_to_gravity}});
    if (!tp.second) {
        throw std::runtime_error("Rotor with ID \"" + std::to_string(tire_id) + "\" already exists");
    }
}
