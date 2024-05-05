#pragma once
#include <compare>

namespace Mlib {

enum class ActorType;
enum class VehicleDomain;
struct SkillScenario {
	ActorType actor_type;
	VehicleDomain vehicle_domain;
	std::strong_ordering operator <=> (const SkillScenario&) const = default;
};

std::string skill_scenario_to_string(const SkillScenario& scenario);

struct SkillFactor {
	SkillScenario scenario;
	float factor;
};

}
