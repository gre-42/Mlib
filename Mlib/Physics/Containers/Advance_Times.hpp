#pragma once
#include <list>
#include <memory>
#include <set>

namespace Mlib {

class AdvanceTime;

class AdvanceTimes {
    friend class PhysicsEngine;
public:
    void add_advance_time(const std::shared_ptr<AdvanceTime>& advance_time);
    void schedule_delete_advance_time(const AdvanceTime* advance_time);
    void delete_scheduled_advance_times();
private:
    std::list<std::shared_ptr<AdvanceTime>> advance_times_;
    std::set<const AdvanceTime*> advance_times_to_delete_;
};

}
