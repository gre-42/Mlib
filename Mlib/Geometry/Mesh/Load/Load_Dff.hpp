#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

// This file is based on the "librw" project (https://github.com/aap/librw)

namespace Mlib {

namespace Dff {

using Mlib::uninitialized;
using Mlib::FixedArray;
using Mlib::UFixedArray;
using Mlib::BoundingSphere;
template <class TData, size_t tndim>
using UBoundingSphere = Mlib::UBoundingSphere<TData, tndim>;
using Mlib::TransformationMatrix;
using Mlib::UUVector;

class Geometry;
struct Clump;

struct Triangle
{
    FixedArray<uint16_t, 3> v;
    uint16_t matId;
};
static_assert(sizeof(Triangle) == 8);

using TexCoords = FixedArray<float, 2>;
static_assert(sizeof(TexCoords) == 8);

using RGBA = FixedArray<uint8_t, 4>;
using RGBAf = FixedArray<float, 4>;

struct MorphTarget
{
    Geometry *parent;
    std::vector<UFixedArray<float, 3>> vertices;
    std::vector<UFixedArray<float, 3>> normals;
    BoundingSphere<float, 3> bounding_sphere = uninitialized;
};

struct SurfaceProperties
{
    float ambient;
    float specular;
    float diffuse;
};
static_assert(sizeof(SurfaceProperties) == 12);

struct Raster
{
    enum { FLIPWAITVSYNCH = 1 };

    int32_t platform;

    // TODO: use bytes
    int32_t type;
    int32_t flags;
    int32_t privateFlags;
    int32_t format;
    int32_t width, height, depth;
    int32_t stride;
    uint8_t *pixels;
    uint8_t *palette;
    // remember for locked rasters
    uint8_t *originalPixels;
    int32_t originalWidth;
    int32_t originalHeight;
    int32_t originalStride;
    // subraster
    Raster *parent;
    int32_t offsetX;
    int32_t offsetY;

    enum Format {
        DEFAULT    = 0,
        C1555      = 0x0100,
        C565       = 0x0200,
        C4444      = 0x0300,
        LUM8       = 0x0400,
        C8888      = 0x0500,
        C888       = 0x0600,
        D16        = 0x0700,
        D24        = 0x0800,
        D32        = 0x0900,
        C555       = 0x0A00,
        AUTOMIPMAP = 0x1000,
        PAL8       = 0x2000,
        PAL4       = 0x4000,
        MIPMAP     = 0x8000
    };
    enum Type {
        NORMAL        = 0x00,
        ZBUFFER       = 0x01,
        CAMERA        = 0x02,
        TEXTURE       = 0x04,
        CAMERATEXTURE = 0x05,
        DONTALLOCATE  = 0x80
    };
    enum LockMode {
        LOCKWRITE    = 1,
        LOCKREAD    = 2,
        LOCKNOFETCH    = 4,    // don't fetch pixel data
        LOCKRAW        = 8,
    };

    enum
    {
        // from RW
        PRIVATELOCK_READ        = 0x02,
        PRIVATELOCK_WRITE        = 0x04,
        PRIVATELOCK_READ_PALETTE    = 0x08,
        PRIVATELOCK_WRITE_PALETTE    = 0x10,
    };
};

struct Texture;

struct TexDictionary
{
    enum { ID = 6 };
    std::list<Texture*> textures;
};

struct Texture
{
    enum FilterMode {
        NEAREST = 1,
        LINEAR,
        MIPNEAREST,    // one mipmap
        MIPLINEAR,
        LINEARMIPNEAREST,    // mipmap interpolated
        LINEARMIPLINEAR
    };
    enum Addressing {
        WRAP = 1,
        MIRROR,
        CLAMP,
        BORDER
    };

    Raster raster;
    TexDictionary dict;
    std::string name;
    std::string mask;
    uint32_t filterAddressing; // VVVVUUUU FFFFFFFF
};

struct Material {
    std::optional<Texture> texture;
    RGBA color = uninitialized;
    SurfaceProperties surfaceProps;
};

struct MaterialList
{
    std::vector<Material> materials;
    uint32_t space;
};

class Geometry {
public:
    Geometry(
        int32_t numVerts,
        int32_t numTris,
        uint32_t flags);

    enum { ID = 8 };
    uint32_t flags;
    int32_t numTriangles;
    int32_t numVertices;
    int32_t numMorphTargets;
    int32_t numTexCoordSets;

    UUVector<Triangle> triangles;
    UUVector<RGBA> colors;
    std::vector<UUVector<TexCoords>> texCoords;

    std::vector<MorphTarget> morphTargets;
    MaterialList matList;

    enum Flags
    {
        TRISTRIP  = 0x01,
        POSITIONS = 0x02,
        TEXTURED  = 0x04,
        PRELIT    = 0x08,
        NORMALS   = 0x10,
        LIGHT     = 0x20,
        MODULATE  = 0x40,
        TEXTURED2 = 0x80,
        // When this flag is set the geometry has
        // native geometry. When streamed out this geometry
        // is written out instead of the platform independent data.
        // When streamed in with this flag, the geometry is mostly empty.
        NATIVE         = 0x01000000,
        // Just for documentation: RW sets this flag
        // to prevent rendering when executing a pipeline,
        // so only instancing will occur.
        // librw's pipelines are different so it's unused here.
        NATIVEINSTANCE = 0x02000000
    };

    enum LockFlags
    {
        LOCKPOLYGONS     = 0x0001,
        LOCKVERTICES     = 0x0002,
        LOCKNORMALS      = 0x0004,
        LOCKPRELIGHT     = 0x0008,

        LOCKTEXCOORDS    = 0x0010,
        LOCKTEXCOORDS1   = 0x0010,
        LOCKTEXCOORDS2   = 0x0020,
        LOCKTEXCOORDS3   = 0x0040,
        LOCKTEXCOORDS4   = 0x0080,
        LOCKTEXCOORDS5   = 0x0100,
        LOCKTEXCOORDS6   = 0x0200,
        LOCKTEXCOORDS7   = 0x0400,
        LOCKTEXCOORDS8   = 0x0800,
        LOCKTEXCOORDSALL = 0x0ff0,

        LOCKALL          = 0x0fff
    };
};

struct Frame {
    std::string name;
    TransformationMatrix<float, float, 3> matrix = uninitialized;
    int32_t parent;
};

struct Object {
    int32_t type;
    int32_t flags;
};

struct ObjectWithFrame {
    Object object;
    Frame* inFrame;
};

struct World {};

class Light {
public:
    ObjectWithFrame object;
    float radius;
    RGBAf color = uninitialized;
    float minusCosAngle;

    // clump extension
    Clump *clump;

    // world extension
    World *world;

    enum Type {
        DIRECTIONAL = 1,
        AMBIENT,
        POINT = 0x80,	// positioned
        SPOT,
        SOFTSPOT
    };
    enum Flags {
        LIGHTATOMICS = 1,
        LIGHTWORLD = 2
    };
};

struct Atomic {
    const Frame* frame = nullptr;
    std::shared_ptr<Geometry> geometry;
    Object object;
};

struct Clump {
    Frame* frame;
    std::vector<Frame> frames;
    std::vector<Atomic> atomics;
    std::vector<Light> lights;
};

struct FrameStreamData
{
    FixedArray<float, 3> right = uninitialized;
    FixedArray<float, 3> up = uninitialized;
    FixedArray<float, 3> at = uninitialized;
    FixedArray<float, 3> pos = uninitialized;
    int32_t parent;
    int32_t matflag;
};
static_assert(sizeof(FrameStreamData) == 56);

struct Camera {
    enum { ID = 4 };
    enum { PERSPECTIVE = 1, PARALLEL };
    enum { CLEARIMAGE = 0x1, CLEARZ = 0x2, CLEARSTENCIL = 0x4 };
    // return value of frustumTestSphere
    enum { SPHEREOUTSIDE, SPHEREBOUNDARY, SPHEREINSIDE };

    ObjectWithFrame object;
    FixedArray<float, 2> viewWindow = uninitialized;
    FixedArray<float, 2> viewOffset = uninitialized;
    float nearPlane, farPlane;
    float fogPlane;
    int32_t projection;

    FixedArray<float, 4, 4> viewMatrix = uninitialized;
    float zScale, zShift;
};

struct ChunkHeaderInfo
{
    uint32_t type;
    uint32_t length;
    uint32_t version;
    uint32_t build;
};
static_assert(sizeof(ChunkHeaderInfo) == 16);

class IPlugin {
public:
    uint32_t id;
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Material& material) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Texture& texture) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Frame& frame) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Atomic& atomic) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Clump& clump) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Light& light) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Camera& camera) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Geometry& geometry) { return false; }
    virtual void always_callback(Material& material) {};
    virtual void always_callback(Texture& texture) {};
    virtual void always_callback(Frame& frame) {};
    virtual void always_callback(Atomic& atomic) {};
    virtual void always_callback(Clump& clump) {};
    virtual void always_callback(Light& light) {};
    virtual void always_callback(Camera& camera) {};
    virtual void always_callback(Geometry& geometry) {};
};

Clump read_dff(std::istream& istr);

}

}
