#pragma once
#include <cstdint>

namespace Mlib {
namespace Dff {

struct MipLevelMetadata {
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	inline uint32_t size() const {
		return stride * height;
	}
};

}
}
