#pragma once
#include <Mlib/Source_Location.hpp>
#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace Mlib {

class IAdvanceTime;

class AdvanceTimes {
    friend class PhysicsEngine;
public:
    AdvanceTimes();
    ~AdvanceTimes();
    void add_advance_time(std::unique_ptr<IAdvanceTime>&& advance_time);
    void add_advance_time(IAdvanceTime& advance_time);
    void schedule_delete_advance_time(const IAdvanceTime& advance_time, SourceLocation loc);
    void delete_advance_time(const IAdvanceTime& advance_time, SourceLocation loc);
    void delete_scheduled_advance_times();
private:
    std::list<std::unique_ptr<IAdvanceTime>> advance_times_shared_;
    std::list<IAdvanceTime*> advance_times_ptr_;
    std::mutex scheduled_deletion_mutex_;
};

}
