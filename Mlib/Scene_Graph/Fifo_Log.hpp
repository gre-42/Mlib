#pragma once
#include <Mlib/Os/Threads/Fast_Mutex.hpp>
#include <Mlib/Scene_Graph/Base_Log.hpp>
#include <list>

namespace Mlib {

class FifoLog: public BaseLog {
public:
    explicit FifoLog(size_t max_log_size);
    virtual void log(const std::string& message, LogEntrySeverity severity) override;
    virtual void get_messages(std::ostream& ostr, size_t nentries, LogEntrySeverity severity) const override;
private:
    std::list<std::pair<LogEntrySeverity, std::string>> entries_;
    size_t max_log_size_;
    mutable FastMutex mutex_;
};

}
