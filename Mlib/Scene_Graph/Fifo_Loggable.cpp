#include "Fifo_Loggable.hpp"
#include <ostream>

using namespace Mlib;

FifoLoggable::FifoLoggable(size_t max_log_size)
: max_log_size_{max_log_size}
{}

void FifoLoggable::log(const std::string& message) {
    if (entries_.size() > max_log_size_) {
        throw std::runtime_error("Log race condition");
    }
    if (entries_.size() == max_log_size_) {
        entries_.pop_front();
    }
    entries_.push_back(message);
}

void FifoLoggable::get_messages(std::ostream& ostr, size_t nentries) const
{
    size_t i = 0;
    for(const std::string& s : entries_) {
        if (i == nentries) {
            break;
        }
        ostr << s << std::endl;
        ++i;
    }
}
