#pragma once
#include <Mlib/Scene_Config/Remote_Event_History_Duration.hpp>
#include <map>

namespace Mlib {

template <class TEvent, class TTime>
class EventsAndTimes {
public:
    template <class K, class... Args>
    decltype(auto) try_emplace(K&& k, Args&&... args) {
        return events_.try_emplace(std::forward<K>(k), std::forward<Args...>(args...));
    }
    template <class TDuration>
    void forget_old_entries(TTime local_time, TDuration history_duration) {
        std::erase_if(events_, [&](const auto& item){
            return item.second + history_duration < local_time;
        });
    }
    size_t size() const {
        return events_.size();
    }
    decltype(auto) begin() const {
        return events_.begin();
    }
    decltype(auto) end() const {
        return events_.end();
    }
    bool contains_key(const TEvent& event) const {
        return events_.find(event) != events_.end();
    }
private:
    std::map<TEvent, TTime> events_;
};

}
