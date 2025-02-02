#pragma once
#include <compare>
#include <string>

namespace Mlib {

enum class ActorType;
enum class ActorTask;
struct SkillScenario {
    ActorType actor_type;
    ActorTask actor_task;
    std::strong_ordering operator <=> (const SkillScenario&) const = default;
};

std::string skill_scenario_to_string(const SkillScenario& scenario);

struct SkillFactor {
    SkillScenario scenario;
    float factor;
};

}
