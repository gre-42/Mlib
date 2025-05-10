#pragma once

template <class TData>
class StbInfo;

StbInfo<unsigned char> stb_multiply_with_alpha(
    unsigned char* data,
    int width,
    int height,
    int nrChannels);
