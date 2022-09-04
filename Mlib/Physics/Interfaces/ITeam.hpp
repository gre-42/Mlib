#pragma once

namespace Mlib {

class Bullet;

class ITeam {
public:
    virtual void notify_kill() = 0;
    virtual void notify_bullet_destroyed(Bullet* bullet) = 0;
};

}
