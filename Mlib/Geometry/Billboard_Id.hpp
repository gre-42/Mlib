#pragma once
#include <cstdint>
#include <limits>

namespace Mlib {

using BillboardId = uint16_t;
static const BillboardId BILLBOARD_ID_NONE = std::numeric_limits<BillboardId>::max();

}
