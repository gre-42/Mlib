#include "Fifo_Log.hpp"
#include <ostream>

using namespace Mlib;

FifoLog::FifoLog(size_t max_log_size)
: max_log_size_{max_log_size}
{}

void FifoLog::log(const std::string& message) {
    std::lock_guard lock{mutex_};
    if (entries_.size() > max_log_size_) {
        throw std::runtime_error("Log race condition");
    }
    if (entries_.size() == max_log_size_) {
        entries_.pop_front();
    }
    entries_.push_back(message);
}

void FifoLog::get_messages(std::ostream& ostr, size_t nentries) const
{
    std::lock_guard lock{mutex_};
    if (!entries_.empty()) {
        auto it = entries_.end();
        for(size_t i = 0; i < std::min(nentries, entries_.size()); ++i) {
            --it;
        }
        for(; it != entries_.end(); ++it) {
            ostr << *it << std::endl;
        }
    }
}
