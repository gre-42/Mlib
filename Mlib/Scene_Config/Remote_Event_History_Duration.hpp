#pragma once
#include <chrono>
#include <cstdint>

namespace Mlib {

using RemoteTimePeriod = std::micro;
using RemoteEventHistoryOffset = std::chrono::duration<uint32_t, RemoteTimePeriod>;
static const auto REMOTE_EVENT_HISTORY_DURATION = RemoteEventHistoryOffset{std::chrono::seconds{5}};
static const auto REMOTE_DATAGRAM_INTERVAL = REMOTE_EVENT_HISTORY_DURATION;
static const auto REMOTE_PLAYBACK_DELAY = std::chrono::milliseconds{ 200 };

}
