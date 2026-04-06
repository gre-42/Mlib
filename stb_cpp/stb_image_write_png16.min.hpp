#pragma once
// This file avoids macro collisions with the original "stb_image_png.h" file

namespace png16 {

int stbi_write_png16(char const* filename, int w, int h, int comp, const void* data, int stride_in_bytes);

}
