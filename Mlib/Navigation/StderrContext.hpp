#pragma once
#include <Mlib/Misc/Pragma_Clang.hpp>
PRAGMA_CLANG_DIAGNOSTIC_PUSH
PRAGMA_CLANG_DIAGNOSTIC_IGNORED(-Wsign-conversion)
#include <Recast.h>
PRAGMA_CLANG_DIAGNOSTIC_POP
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
