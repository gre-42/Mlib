#pragma once
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Render/Any_Gl.hpp>

template <class TData>
struct StbInfo;

namespace Mlib {

StbInfo<uint8_t> download_as_stb_image(
	GLuint frame_buffer,
	GLsizei width,
	GLsizei height,
	int nchannels);

}
