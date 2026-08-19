#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Physics/Interfaces/ITeam.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>
#include <set>
#include <string>

namespace Mlib {

class Player;
template <class T>
class VariableAndHash;
class GameStatistics;

class Team final: public ITeam, public virtual DanglingBaseClass, public virtual DestructionNotifier {
public:
    Team(NTeamCountType id, VariableAndHash<std::string> name, GameStatistics& game_statistics_);
    ~Team();

    NTeamCountType id() const;
    const VariableAndHash<std::string>& name() const;

    // ITeam
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual DestructionFunctions& on_destroy_team() override;

    void add_player(const VariableAndHash<std::string>& name);
    const std::set<VariableAndHash<std::string>>& players() const;

    uint32_t nwins() const;
    uint32_t nlosses() const;
    uint32_t nkills() const;
    void increase_nwins();
    void increase_nlosses();

private:
    NTeamCountType id_;
    VariableAndHash<std::string> name_;
    std::set<VariableAndHash<std::string>> players_;
    GameStatistics& game_statistics_;
    uint32_t nwins_;
    uint32_t nlosses_;
    DestructionObservers<const ITeam&> destruction_observers_;
};

}
