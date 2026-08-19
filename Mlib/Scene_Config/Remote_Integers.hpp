#pragma once
#include <cstdint>

namespace Mlib {

using RemoteSiteId = uint8_t;
using LocalObjectId = uint16_t;
using RemoteSceneObjectUnderlyingType = uint8_t;
using GenericSessionIdType = uint32_t;
using ReloadCountType = GenericSessionIdType;
using SessionIdType = GenericSessionIdType;
using SkillsType = uint8_t;
using NUserCountType = uint8_t;
enum class NTeamCountType : uint8_t {};
using UserStatusType = uint8_t;

}
