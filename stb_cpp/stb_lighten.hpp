#pragma once

void stb_lighten(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    short color[3]);

void stb_lighten_horizontal_gradient(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    short color_left[3],
    short color_right[3]);


void stb_lighten_vertical_gradient(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    short color_top[3],
    short color_bottom[3]);
