#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <chrono>

namespace Mlib {

class AdvanceTimes;
class Players;
class CollisionQuery;
class DeleteNodeMutex;

class PodBots: public AdvanceTime {
public:
    PodBots(
        AdvanceTimes& advance_times,
        Players& players,
        CollisionQuery& collision_query,
        DeleteNodeMutex& delete_node_mutex);
    ~PodBots();
    virtual void advance_time(float dt) override;
private:
    AdvanceTimes& advance_times_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
