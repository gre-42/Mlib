#pragma once
#include <cstdint>
#include <vector>

std::vector<uint8_t> stb_encode_png(
	const uint8_t* data,
	int width,
	int height,
	int nchannels);
