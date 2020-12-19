#pragma once
#include <Mlib/Scene_Graph/BaseLog.hpp>
#include <list>

namespace Mlib {

class FifoLoggable: public BaseLog {
public:
    explicit FifoLoggable(size_t max_log_size);
    virtual void log(const std::string& message) override;
    virtual void get_messages(std::ostream& ostr, size_t nentries) const override;
private:
    std::list<std::string> entries_;
    size_t max_log_size_;
};

}
