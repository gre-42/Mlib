#include "Fifo_Log.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <ostream>

using namespace Mlib;

FifoLog::FifoLog(size_t max_log_size)
: max_log_size_{max_log_size}
{}

void FifoLog::log(const std::string& message, LogEntrySeverity severity) {
    if (max_log_size_ == 0) {
        return;
    }
    std::scoped_lock lock{ mutex_ };
    if (entries_.size() > max_log_size_) {
        THROW_OR_ABORT("Log race condition");
    }
    if (entries_.size() == max_log_size_) {
        entries_.pop_front();
    }
    entries_.push_back({severity, message});
}

void FifoLog::get_messages(std::ostream& ostr, size_t nentries, LogEntrySeverity severity) const
{
    std::scoped_lock lock{ mutex_ };
    auto it = entries_.end();
    for (size_t i = 0; (i < entries_.size()) && (nentries > 0); ++i) {
        if (it->first >= severity) {
            --nentries;
        }
        --it;
    }
    for (; it != entries_.end(); ++it) {
        if (it->first >= severity) {
            ostr << it->second << std::endl;
        }
    }
}
