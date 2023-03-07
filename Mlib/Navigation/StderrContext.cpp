#include "StderrContext.hpp"
#include <iostream>

using namespace Mlib;

void StderrContext::doLog(const rcLogCategory category, const char* msg, const int len) {
    std::cerr << std::string(msg, (size_t)len) << std::endl;
}

void StderrContext::doResetTimers() {
    for (int i = 0; i < RC_MAX_TIMERS; ++i) {
        m_accTime[i] = std::chrono::steady_clock::duration(0);
    }
}

void StderrContext::doStartTimer(const rcTimerLabel label) {
    m_startTime[label] = std::chrono::steady_clock::now();
}

void StderrContext::doStopTimer(const rcTimerLabel label) {
    auto endTime = std::chrono::steady_clock::now();
    auto deltaTime = endTime - m_startTime[label];
    m_accTime[label] += deltaTime;
}

int StderrContext::doGetAccumulatedTime(const rcTimerLabel label) const
{
    return (int)std::chrono::duration_cast<std::chrono::microseconds>(m_accTime[label]).count();
}
