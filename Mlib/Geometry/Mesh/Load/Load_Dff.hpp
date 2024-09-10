#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>
#include <Mlib/Geometry/Mesh/Load/Palette.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <filesystem>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

// This file is based on the "librw" project (https://github.com/aap/librw)

namespace Mlib {

namespace Dff {

#ifdef BIGENDIAN
#define ASSERTLITTLE THROW_OR_ABORT("Unsafe code on big-endian")
#else
#define ASSERTLITTLE
#endif

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
class IRasterFactory;
struct RasterConfig;

enum Platform
{
    PLATFORM_NULL = 0,
    // D3D7
    PLATFORM_GL   = 2,
    // MAC
    PLATFORM_PS2  = 4,
    PLATFORM_XBOX = 5,
    // GAMECUBE
    // SOFTRAS
    PLATFORM_D3D8 = 8,
    PLATFORM_D3D9 = 9,
    // PSP

    // non-stock-RW platforms

    PLATFORM_WDGL = 11,    // WarDrum OpenGL
    PLATFORM_GL3  = 12,    // my GL3 implementation

    NUM_PLATFORMS,

    FOURCC_PS2 = 0x00325350        // 'PS2\0'
};

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

struct Image
{
    uint32_t flags;
    uint32_t width, height;
    uint32_t depth;
    uint32_t bpp;    // bytes per pixel
    uint32_t stride;
    std::vector<uint8_t> pixels;
    Palette palette = uninitialized;
    void allocate();
    bool has_alpha() const;
    void compress_palette();
    void unpalletize(bool force_alpha = false);
    void set_pixels_dxt(uint32_t type, uint8_t *pixels);
    void remove_mask();
};

namespace Raster
{
    enum { FLIPWAITVSYNCH = 1 };

    enum Format: uint32_t {
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
    enum Type: uint32_t {
        NORMAL        = 0x00,
        ZBUFFER       = 0x01,
        CAMERA        = 0x02,
        TEXTURE       = 0x04,
        CAMERATEXTURE = 0x05,
        DONTALLOCATE  = 0x80
    };
    enum LockMode: uint32_t {
        LOCKWRITE       = 1,
        LOCKREAD        = 2,
        LOCKNOFETCH     = 4,    // don't fetch pixel data
        LOCKRAW         = 8,
    };

    enum: uint32_t
    {
        // from RW
        PRIVATELOCK_READ        = 0x02,
        PRIVATELOCK_WRITE        = 0x04,
        PRIVATELOCK_READ_PALETTE    = 0x08,
        PRIVATELOCK_WRITE_PALETTE    = 0x10,
    };
};

std::unique_ptr<IRaster> read_as_image(
    std::istream& istr,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t format,
    uint32_t num_levels,
    const IRasterFactory& raster_factory,
    const RasterConfig& raster_config,
    IoVerbosity verbosity);

uint32_t palette_size(uint32_t format);
void read_palette(Palette& palette, std::istream& istr, uint32_t format, IoVerbosity verbosity);

struct Texture;

struct TexDictionary
{
    enum { ID = 6 };
    std::vector<std::shared_ptr<Texture>> textures;
};

struct Texture
{
    enum FilterMode {
        NEAREST = 1,
        LINEAR,
        MIPNEAREST,         // one mipmap
        MIPLINEAR,
        LINEARMIPNEAREST,   // mipmap interpolated
        LINEARMIPLINEAR
    };
    enum Addressing {
        WRAP = 1,
        MIRROR,
        CLAMP,
        BORDER
    };

    std::unique_ptr<IRaster> raster;
    std::vector<uint8_t> data;
    // TexDictionary dict;
    VariableAndHash<std::string> name;
    VariableAndHash<std::string> mask;
    uint32_t filter_addressing; // VVVVUUUU FFFFFFFF
};

struct Material {
    std::shared_ptr<Texture> texture;
    RGBA color = uninitialized;
    SurfaceProperties surface_properties;
};

struct MaterialList
{
    std::vector<Material> materials;
    uint32_t space;
};

class Geometry {
public:
    Geometry(
        uint32_t numVerts,
        uint32_t numTris,
        uint32_t flags);

    enum { ID = 8 };
    uint32_t flags;
    uint32_t numTriangles;
    uint32_t numVertices;
    uint32_t numMorphTargets;
    uint32_t numTexCoordSets;

    UUVector<Triangle> triangles;
    UUVector<RGBA> colors;
    std::vector<UUVector<TexCoords>> tex_coords;

    std::vector<MorphTarget> morph_targets;
    MaterialList mat_list;

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
    uint32_t parent;
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
        POINT = 0x80,    // positioned
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
    uint32_t parent_index;
    uint32_t matrix_flags;
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

#define MAKEPLUGINID(v, id) (((v & 0xFFFFFF) << 8) | (id & 0xFF))

enum VendorID
{
    VEND_CORE           = 0,
    VEND_CRITERIONTK    = 1,
    VEND_CRITERIONINT   = 4,
    VEND_CRITERIONWORLD = 5,
    // Used for rasters (platform-specific)
    VEND_RASTER         = 10,
    // Used for driver/device allocation tags
    VEND_DRIVER         = 11
};

enum PluginID
{
    // Core
    ID_NAOBJECT      = MAKEPLUGINID(VEND_CORE, 0x00),
    ID_STRUCT        = MAKEPLUGINID(VEND_CORE, 0x01),
    ID_STRING        = MAKEPLUGINID(VEND_CORE, 0x02),
    ID_EXTENSION     = MAKEPLUGINID(VEND_CORE, 0x03),
    ID_CAMERA        = MAKEPLUGINID(VEND_CORE, 0x05),
    ID_TEXTURE       = MAKEPLUGINID(VEND_CORE, 0x06),
    ID_MATERIAL      = MAKEPLUGINID(VEND_CORE, 0x07),
    ID_MATLIST       = MAKEPLUGINID(VEND_CORE, 0x08),
    ID_WORLD         = MAKEPLUGINID(VEND_CORE, 0x0B),
    ID_MATRIX        = MAKEPLUGINID(VEND_CORE, 0x0D),
    ID_FRAMELIST     = MAKEPLUGINID(VEND_CORE, 0x0E),
    ID_GEOMETRY      = MAKEPLUGINID(VEND_CORE, 0x0F),
    ID_CLUMP         = MAKEPLUGINID(VEND_CORE, 0x10),
    ID_LIGHT         = MAKEPLUGINID(VEND_CORE, 0x12),
    ID_ATOMIC        = MAKEPLUGINID(VEND_CORE, 0x14),
    ID_TEXTURENATIVE = MAKEPLUGINID(VEND_CORE, 0x15),
    ID_TEXDICTIONARY = MAKEPLUGINID(VEND_CORE, 0x16),
    ID_IMAGE         = MAKEPLUGINID(VEND_CORE, 0x18),
    ID_GEOMETRYLIST  = MAKEPLUGINID(VEND_CORE, 0x1A),
    ID_ANIMANIMATION = MAKEPLUGINID(VEND_CORE, 0x1B),
    ID_RIGHTTORENDER = MAKEPLUGINID(VEND_CORE, 0x1F),
    ID_UVANIMDICT    = MAKEPLUGINID(VEND_CORE, 0x2B),

    // Toolkit
    ID_SKYMIPMAP     = MAKEPLUGINID(VEND_CRITERIONTK, 0x10),
    ID_SKIN          = MAKEPLUGINID(VEND_CRITERIONTK, 0x16),
    ID_HANIM         = MAKEPLUGINID(VEND_CRITERIONTK, 0x1E),
    ID_USERDATA      = MAKEPLUGINID(VEND_CRITERIONTK, 0x1F),
    ID_MATFX         = MAKEPLUGINID(VEND_CRITERIONTK, 0x20),
    ID_ANISOT        = MAKEPLUGINID(VEND_CRITERIONTK, 0x27),
    ID_PDS           = MAKEPLUGINID(VEND_CRITERIONTK, 0x31),
    ID_ADC           = MAKEPLUGINID(VEND_CRITERIONTK, 0x34),
    ID_UVANIMATION   = MAKEPLUGINID(VEND_CRITERIONTK, 0x35),

    // World
    ID_MESH          = MAKEPLUGINID(VEND_CRITERIONWORLD, 0x0E),
    ID_NATIVEDATA    = MAKEPLUGINID(VEND_CRITERIONWORLD, 0x10),
    ID_VERTEXFMT     = MAKEPLUGINID(VEND_CRITERIONWORLD, 0x11),

    // custom native raster
    ID_RASTERGL      = MAKEPLUGINID(VEND_RASTER, PLATFORM_GL),
    ID_RASTERPS2     = MAKEPLUGINID(VEND_RASTER, PLATFORM_PS2),
    ID_RASTERXBOX    = MAKEPLUGINID(VEND_RASTER, PLATFORM_XBOX),
    ID_RASTERD3D8    = MAKEPLUGINID(VEND_RASTER, PLATFORM_D3D8),
    ID_RASTERD3D9    = MAKEPLUGINID(VEND_RASTER, PLATFORM_D3D9),
    ID_RASTERWDGL    = MAKEPLUGINID(VEND_RASTER, PLATFORM_WDGL),
    ID_RASTERGL3     = MAKEPLUGINID(VEND_RASTER, PLATFORM_GL3),

    // anything driver/device related (only as allocation tag)
    ID_DRIVER        = MAKEPLUGINID(VEND_DRIVER, 0)
};
#undef MAKEPLUGINID

bool find_chunk(
    std::istream& str,
    uint32_t type,
    uint32_t *length,
    uint32_t *version,
    IoVerbosity verbosity);

class IPlugin {
public:
    uint32_t id;
    virtual ~IPlugin() = default;
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Material& material, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, const std::shared_ptr<Texture>& texture, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Frame& frame, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Atomic& atomic, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Clump& clump, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Light& light, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Camera& camera, IoVerbosity verbosity) { return false; }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Geometry& geometry, IoVerbosity verbosity) { return false; }
    virtual void always_callback(Material& material) {};
    virtual void always_callback(const std::shared_ptr<Texture>& texture) {};
    virtual void always_callback(Frame& frame) {};
    virtual void always_callback(Atomic& atomic) {};
    virtual void always_callback(Clump& clump) {};
    virtual void always_callback(Light& light) {};
    virtual void always_callback(Camera& camera) {};
    virtual void always_callback(Geometry& geometry) {};
};

Clump read_dff(std::istream& istr, IoVerbosity verbosity);

TexDictionary read_txd(
    const std::filesystem::path& path,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity);

TexDictionary read_txd(
    std::istream& istr,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity);

}

}
