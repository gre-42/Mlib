#include "Skill_Map.hpp"
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <stdexcept>

using namespace Mlib;

Skills& SkillMap::skills(ControlSource source) {
    switch (source) {
    case ControlSource::AI:
        return ai_skills_;
    case ControlSource::USER:
        return user_skills_;
    }
    throw std::runtime_error("Unknown control source");
}

const Skills& SkillMap::skills(ControlSource source) const {
    return const_cast<SkillMap*>(this)->skills(source);
}
