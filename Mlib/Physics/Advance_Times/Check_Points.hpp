#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class AbsoluteMovable;
class AdvanceTimes;
class SceneNode;
class Players;
class Player;

class CheckPoints: public DestructionObserver, public AdvanceTime {
public:
    CheckPoints(
        const std::string& filename,
        AdvanceTimes& advance_times,
        SceneNode* moving_node,
        AbsoluteMovable* moving,
        SceneNode* beacon_node0,
        SceneNode* beacon_node1,
        Players* players,
        Player* player,
        size_t nth,
        float radius);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
private:
    AdvanceTimes& advance_times_;
    TrackReader track_reader_;
    SceneNode* moving_node_;
    AbsoluteMovable* moving_;
    SceneNode* beacon_node0_;
    SceneNode* beacon_node1_;
    Players* players_;
    Player* player_;
    FixedArray<float, 3> current_checkpoint_position_;
    float radius_;
    size_t nth_;
    size_t i01_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
};

}
