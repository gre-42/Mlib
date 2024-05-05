#include "Skill_Factor.hpp"
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>

using namespace Mlib;

std::string Mlib::skill_scenario_to_string(const SkillScenario& scenario) {
	return '(' + actor_type_to_string(scenario.actor_type) + ", " +
		vehicle_domain_to_string(scenario.vehicle_domain) + ')';
}
