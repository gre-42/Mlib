#pragma once

void stb_replace_color(
    unsigned char* data,
    int width,
    int height,
    int nrChannels,
    unsigned char color_to_replace[3],
    unsigned char replacement_color[3],
    unsigned char tolerance);
