#pragma once
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Render/Any_Gl.hpp>

template <class TData>
struct StbInfo;

namespace Mlib {

template <class TData>
class Array;
enum class FlipMode;

StbInfo<uint8_t> download_as_stb_image(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    int nchannels,
    FlipMode flip_mode);

Array<float> color_to_array(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    int nchannels,
    FlipMode flip_mode);

Array<float> depth_to_array(
    GLuint frame_buffer,
    GLsizei width,
    GLsizei height,
    FlipMode flip_mode);

}
