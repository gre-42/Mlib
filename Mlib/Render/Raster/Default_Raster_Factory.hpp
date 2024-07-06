#pragma once

#define RW_GL3

#ifdef RW_PS2
#error PS2 support not yet implemented
#elif defined(RW_WDGL)
#error WDGL support not yet implemented
#elif defined(RW_GL3)
#include <Mlib/Render/Raster/Gl3_Raster_Factory.hpp>
namespace Mlib {
namespace Dff {
using DefaultRasterFactory = Gl3RasterFactory;
}
}
#elif defined(RW_D3D9)
#error D3D9 support not yet implemented
#else
#error Unknown rendering backend, or backend not defined
#endif
