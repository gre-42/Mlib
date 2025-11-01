#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <cstdint>
#include <set>
#include <string>

namespace Mlib {

class Player;

class Team final: public ITeam, public virtual DanglingBaseClass, public virtual DestructionNotifier {
public:
    Team(std::string name);
    ~Team();

    const std::string& name() const;

    // ITeam
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual DestructionFunctions& on_destroy_team() override;

    void add_player(const std::string& name);
    const std::set<std::string>& players() const;

    uint32_t nwins() const;
    uint32_t nlosses() const;
    uint32_t nkills() const;
    void increase_nwins();
    void increase_nlosses();
    void increase_nkills();

private:
    std::string name_;
    std::set<std::string> players_;
    uint32_t nwins_;
    uint32_t nlosses_;
    uint32_t nkills_;
    DestructionObservers<const ITeam&> destruction_observers_;
};

}
