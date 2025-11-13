#pragma once
#include <iosfwd>

namespace Mlib {

struct Skills {
    template <class Archive>
    void serialize(Archive& archive) {
        archive(can_drive);
        archive(can_aim);
        archive(can_shoot);
        archive(can_select_opponent);
        archive(can_select_weapon);
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
