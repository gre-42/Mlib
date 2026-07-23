#pragma once
#include <chrono>
#include <cstdint>

namespace Mlib {

// 1e-3 * (2**8) == 256 => 0.256s
using RemoteTimePeriod = std::milli;
using RemoteEventHistoryOffset = std::chrono::duration<uint8_t, RemoteTimePeriod>;
static constexpr const auto REMOTE_EVENT_HISTORY_DURATION = RemoteEventHistoryOffset{std::chrono::milliseconds{250}};
static constexpr const auto REMOTE_DATAGRAM_INTERVAL = REMOTE_EVENT_HISTORY_DURATION;

}
