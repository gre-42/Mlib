#pragma once
#include <Mlib/Source_Location.hpp>
#include <list>
#include <map>
#include <memory>
#include <mutex>

namespace Mlib {

class IAdvanceTime;
template <class T>
class DestructionFunctionsTokensObject;
template <class T>
class DanglingBaseClassRef;
struct StaticWorld;

class AdvanceTimes {
public:
    AdvanceTimes();
    ~AdvanceTimes();
    void add_advance_time(const DanglingBaseClassRef<IAdvanceTime>& advance_time, SourceLocation loc);
    void delete_advance_time(const IAdvanceTime& advance_time, SourceLocation loc);
    void advance_time(float dt, const StaticWorld& world);
    bool empty() const;
private:
    bool advancing_time_;
    std::list<std::pair<std::unique_ptr<DestructionFunctionsTokensObject<IAdvanceTime>>, SourceLocation>> advance_times_;
};

}
