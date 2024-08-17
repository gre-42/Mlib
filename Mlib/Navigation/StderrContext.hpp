#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#include <Recast.h>
#pragma clang diagnostic pop
#include <chrono>

namespace Mlib {

class StderrContext: public rcContext {
    std::chrono::steady_clock::time_point m_startTime[RC_MAX_TIMERS];
    std::chrono::steady_clock::duration m_accTime[RC_MAX_TIMERS];
public:
    StderrContext();
    virtual ~StderrContext() override;
protected:
    virtual void doLog(const rcLogCategory category, const char* msg, const int len) override;
    virtual void doResetTimers() override;
    virtual void doStartTimer(const rcTimerLabel label) override;
    virtual void doStopTimer(const rcTimerLabel label) override;
    virtual int doGetAccumulatedTime(const rcTimerLabel label) const override;
};

}
