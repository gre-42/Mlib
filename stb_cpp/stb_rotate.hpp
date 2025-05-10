#pragma once

template <class TData>
class StbInfo;

StbInfo<unsigned char> stb_rotate(
    const StbInfo<unsigned char>& data,
    int degrees);
