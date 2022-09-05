#pragma once
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <cstdint>
#include <set>

namespace Mlib {

class Player;

class Team: public Object, public ITeam {
public:
    Team();
    ~Team();

    // ITeam
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual void notify_bullet_destroyed(Bullet* bullet) override;

    void add_player(const std::string& name);
    const std::set<std::string>& players() const;

    uint32_t nwins() const;
    uint32_t nlosses() const;
    uint32_t nkills() const;
    void increase_nwins();
    void increase_nlosses();
    void increase_nkills();

    DestructionObservers destruction_observers;
private:
    std::set<std::string> players_;
    uint32_t nwins_;
    uint32_t nlosses_;
    uint32_t nkills_;
};

}
