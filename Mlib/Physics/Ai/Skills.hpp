#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <iosfwd>

namespace Mlib {

struct Skills {
    template <class Archive>
    void serialize(Archive& archive) {
        if (Archive::is_saving::value) {
            SkillsType packed =
                ((SkillsType)can_drive << 0) |
                ((SkillsType)can_aim << 1) |
                ((SkillsType)can_shoot << 2) |
                ((SkillsType)can_select_opponent << 3) |
                ((SkillsType)can_select_weapon << 4);
            archive(packed);
        } else {
            SkillsType packed;
            archive(packed);
            can_drive = bool(packed & (1 << 0));
            can_aim = bool(packed & (1 << 1));
            can_shoot = bool(packed & (1 << 2));
            can_select_opponent = bool(packed & (1 << 3));
            can_select_weapon = bool(packed & (1 << 4));
        }
    }
    void write(std::ostream& ostr) const;
    Skills& read(std::istream& istr);
    bool can_drive = false;
    bool can_aim = false;
    bool can_shoot = false;
    bool can_select_opponent = false;
    bool can_select_weapon = false;
};

}
