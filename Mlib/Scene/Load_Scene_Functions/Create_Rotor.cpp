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
        "\\s+vehicle=([\\w+-.]+)"
        "(?:\\s+blades=([\\w+-.]+)"
        "\\s+vehicle_mount_0=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+vehicle_mount_1=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blades_mount_0=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+blades_mount_1=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+rotation=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+engine=(\\w+)"
        "\\s+power2lift=([\\w+-.]+)"
        "(?:\\s+max_align_to_gravity=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_p=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_i=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_d=([\\w+-.]+))?"
        "(?:\\s+align_to_gravity_pid_a=([\\w+-.]+))?"
        "(?:\\s+drift_reduction_factor=([\\w+-.]+))?"
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
    auto vehicle_node = scene.get_node(match[1].str());
    auto vehicle_rb = dynamic_cast<RigidBodyVehicle*>(vehicle_node->get_absolute_movable());
    if (vehicle_rb == nullptr) {
        throw std::runtime_error("Car movable is not a rigid body");
    }
    FixedArray<float, 3> vehicle_mount_0(NAN);
    FixedArray<float, 3> vehicle_mount_1(NAN);
    FixedArray<float, 3> blades_mount_0(NAN);
    FixedArray<float, 3> blades_mount_1(NAN);
    RigidBodyVehicle* blades_rb = nullptr;
    std::string blades_node_name;
    if (match[2].matched) {
        blades_node_name = match[2].str();
        auto blades_node = scene.get_node(blades_node_name);
        blades_rb = dynamic_cast<RigidBodyVehicle*>(blades_node->get_absolute_movable());
        if (blades_rb == nullptr) {
            throw std::runtime_error("Blades movable is not a rigid body");
        }
        vehicle_mount_0 = FixedArray<float, 3>{
            safe_stof(match[3].str()),
            safe_stof(match[4].str()),
            safe_stof(match[5].str())};
        vehicle_mount_1 = FixedArray<float, 3>{
            safe_stof(match[6].str()),
            safe_stof(match[7].str()),
            safe_stof(match[8].str())};
        blades_mount_0 = FixedArray<float, 3>{
            safe_stof(match[9].str()),
            safe_stof(match[10].str()),
            safe_stof(match[11].str())};
        blades_mount_1 = FixedArray<float, 3>{
            safe_stof(match[12].str()),
            safe_stof(match[13].str()),
            safe_stof(match[14].str())};
    }
    FixedArray<float, 3> position{
        safe_stof(match[15].str()),
        safe_stof(match[16].str()),
        safe_stof(match[17].str())};
    FixedArray<float, 3> rotation{
        safe_stof(match[18].str()) * float(M_PI / 180.f),
        safe_stof(match[19].str()) * float(M_PI / 180.f),
        safe_stof(match[20].str()) * float(M_PI / 180.f)};
    std::string engine = match[21].str();
    float power2lift = safe_stof(match[22].str());
    float max_align_to_gravity = match[23].matched
        ? safe_stof(match[23].str()) * float(M_PI / 180.f)
        : NAN;
    size_t tire_id = safe_stoz(match[29].str());
    auto r = tait_bryan_angles_2_matrix<float>(rotation);
    auto tp = vehicle_rb->rotors_.insert({
        tire_id,
        std::make_unique<Rotor>(
            engine,
            TransformationMatrix<float, 3>{ r, position },
            power2lift,
            max_align_to_gravity,
            PidController<float, float>{
                match[24].matched ? safe_stof(match[24].str()) : NAN,
                match[25].matched ? safe_stof(match[25].str()) : NAN,
                match[26].matched ? safe_stof(match[26].str()) : NAN,
                match[27].matched ? safe_stof(match[27].str()) : NAN},
            match[28].matched ? safe_stof(match[28].str()) : NAN,
            vehicle_mount_0,
            vehicle_mount_1,
            blades_mount_0,
            blades_mount_1,
            blades_rb,
            blades_node_name,
            scene)});
    if (!tp.second) {
        throw std::runtime_error("Rotor with ID \"" + std::to_string(tire_id) + "\" already exists");
    }
}
