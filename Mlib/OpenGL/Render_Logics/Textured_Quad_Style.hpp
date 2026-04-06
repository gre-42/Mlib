#pragma once

#ifdef _MSC_VER
#ifdef MlibRender_EXPORTS
#define MLIB_RENDER_API __declspec(dllexport)
#else
#define MLIB_RENDER_API __declspec(dllimport)
#endif
#else
#define MLIB_RENDER_API
#endif

namespace Mlib {

MLIB_RENDER_API extern const float* standard_quad_vertices;
MLIB_RENDER_API extern const float* horizontally_flipped_quad_vertices;
MLIB_RENDER_API extern const float* vertically_flipped_quad_vertices;

}
