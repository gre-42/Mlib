#pragma once
#include <chrono>
#include <cstdint>

namespace Mlib {

// 1e-3 * (2**16) == 65.536 => 65s
using RemoteTimePeriod = std::milli;
using RemoteEventHistoryOffset = std::chrono::duration<uint16_t, RemoteTimePeriod>;
static const auto REMOTE_EVENT_HISTORY_DURATION = RemoteEventHistoryOffset{std::chrono::seconds{5}};
static const auto REMOTE_DATAGRAM_INTERVAL = REMOTE_EVENT_HISTORY_DURATION;

}
