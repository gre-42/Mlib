#pragma once
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <source_location>

namespace Mlib {

class AdvanceTime;

class AdvanceTimes {
    friend class PhysicsEngine;
public:
    AdvanceTimes();
    ~AdvanceTimes();
    void add_advance_time(std::unique_ptr<AdvanceTime>&& advance_time);
    void add_advance_time(AdvanceTime& advance_time);
    void schedule_delete_advance_time(const AdvanceTime& advance_time, std::source_location loc);
    void delete_advance_time(const AdvanceTime& advance_time, std::source_location loc);
    void delete_scheduled_advance_times(std::source_location loc);
private:
    std::list<std::unique_ptr<AdvanceTime>> advance_times_shared_;
    std::list<AdvanceTime*> advance_times_ptr_;
    std::map<const AdvanceTime*, std::source_location> advance_times_to_delete_;
    std::mutex scheduled_deletion_mutex_;
};

}
