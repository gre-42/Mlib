#include "Skill_Factor.hpp"
#include <Mlib/Physics/Rigid_Body/Actor_Task.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>

using namespace Mlib;

std::string Mlib::skill_scenario_to_string(const SkillScenario& scenario) {
    return '(' + actor_type_to_string(scenario.actor_type) + ", " +
        actor_task_to_string(scenario.actor_task) + ')';
}
