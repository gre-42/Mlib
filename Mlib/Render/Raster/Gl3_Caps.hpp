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

namespace Dff {

struct Gl3Caps
{
    int gles;
    int glversion;
    bool dxtSupported;
    bool astcSupported;    // not used yet
    float maxAnisotropy;
};
MLIB_RENDER_API extern Gl3Caps gl3Caps;

}

}
