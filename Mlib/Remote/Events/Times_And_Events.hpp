#pragma once
#include <Mlib/Scene_Config/Remote_Event_History_Duration.hpp>
#include <map>

namespace Mlib {

template <class TTime, class TEvent>
class TimesAndEvents {
public:
    template <class K, class... Args>
    decltype(auto) try_emplace(K&& k, Args&&... args) {
        return events_.try_emplace(std::forward<K>(k), std::forward<Args...>(args...));
    }
    void forget_old_entries(TTime local_time) {
        std::erase_if(events_, [&](const auto& item){
            return item.first + REMOTE_EVENT_HISTORY_DURATION < local_time;
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
private:
    std::map<TTime, TEvent> events_;
};

}
