#pragma once
#include <Mlib/Physics/Ai/Skills.hpp>

namespace Mlib {

enum class ControlSource;

class SkillMap {
public:
    Skills& skills(ControlSource source);
    const Skills& skills(ControlSource source) const;
private:
    Skills ai_skills_;
    Skills user_skills_;
};

}
