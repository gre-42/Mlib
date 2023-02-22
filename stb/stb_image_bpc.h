#pragma once

#include "stb_image.h"

#ifdef __cplusplus
extern "C" {
#endif

STBIDEF void *stbi_load_bpc(char const *filename, int *x, int *y, int *comp, int req_comp, int *bits_per_channel);
STBIDEF void *stbi_load_from_file_bpc(FILE *f, int *x, int *y, int *comp, int req_comp, int *bits_per_channel);
STBIDEF void *stbi_load_from_memory_bpc(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp, int *bits_per_channel);

#ifdef __cplusplus
}
#endif
