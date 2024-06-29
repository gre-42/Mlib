#include "Load_Dff.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <optional>

using namespace Mlib::Dff;

using Mlib::integral_cast;
using Mlib::linfo;
using Mlib::lwarn;
using Mlib::verbose_abort;

static bool VERBOSE = true;

static void print_char(char c) {
    char v = (c >= ' ') && (c <= '~') ? c : '.';
    linfo() << "Read: " << std::hex << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)(uint8_t)c << " - " << v;
}

static void print_chars(std::span<char> span) {
    for (char c : span) {
        print_char(c);
    }
}

template <class T>
T read_binary(std::istream& istr, const char* msg) {
    T result;
    istr.read(reinterpret_cast<char*>(&result), sizeof(result));
    if (istr.fail()) {
        THROW_OR_ABORT("Could not read " + std::string(msg) + " from stream");
    }
    if (VERBOSE) {
        char* begin = reinterpret_cast<char*>(&result);
        char* end = begin + sizeof(result);
        print_chars({ begin, end });
    }
    return result;
}

template <class TVec>
static void read_vector(std::istream& istr, TVec& vec, const char* msg) {
    istr.read(reinterpret_cast<char*>(vec.data()), integral_cast<std::streamsize>(sizeof(typename TVec::value_type) * vec.size()));
    if (istr.fail()) {
        THROW_OR_ABORT("Could not read vector from stream: " + std::string(msg));
    }
    if (VERBOSE) {
        char* begin = reinterpret_cast<char*>(vec.data());
        char* end = begin + sizeof(vec[0]) * vec.size();
        print_chars({ begin, end });
    }
}

static std::string read_string(std::istream& istr, size_t max_length, const char* msg) {
    if (max_length > 1'000) {
        THROW_OR_ABORT("String too large");
    }
    std::string s(max_length, '?');
    read_vector(istr, s, msg);
    return std::string(s.c_str());
}

static void seek_relative_positive(std::istream& str, size_t amount) {
    if (VERBOSE) {
        for (size_t i = 0; i < amount; ++i) {
            auto c = str.get();
            if (c == EOF) {
                THROW_OR_ABORT("Could not read char");
            }
            print_char((char)c);
        }
    } else {
        str.seekg(amount, std::ios::cur);
    }
}

static int32_t library_id_unpack_version(uint32_t libid)
{
    if (libid & 0xFFFF0000) {
        return
            ((libid >> 14 & 0x3FF00) + 0x30000) |
            (libid >> 16 & 0x3F);
    } else {
        return libid << 8;
    }
}

static int library_id_unpack_build(uint32_t libid)
{
    if (libid & 0xFFFF0000) {
        return libid & 0xFFFF;
    } else {
        return 0;
    }
}

struct ChunkHeaderBuf {
    int32_t type;
    int32_t size;
    uint32_t id;
};
static_assert(sizeof(ChunkHeaderBuf) == 12);

static ChunkHeaderInfo read_chunk_header_info(std::istream& str) {
    auto buf = read_binary<ChunkHeaderBuf>(str, "chunk header");
    ChunkHeaderInfo header;
    header.type = buf.type;
    header.length = buf.size;
    header.version = library_id_unpack_version(buf.id);
    header.build = library_id_unpack_build(buf.id);
    return header;
}

#define MAKEPLUGINID(v, id) (((v & 0xFFFFFF) << 8) | (id & 0xFF))
#define MAKEPIPEID(v, id) (((v & 0xFFFF) << 16) | (id & 0xFFFF))

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

static bool find_chunk(std::istream& str, uint32_t type, uint32_t *length, uint32_t *version) {
    while (str.peek() != EOF) {
        ChunkHeaderInfo header = read_chunk_header_info(str);
        if (header.type == ID_NAOBJECT) {
            return false;
        }
        if (header.type == type) {
            if (length != nullptr) {
                *length = header.length;
            }
            if (version != nullptr) {
                *version = header.version;
            }
            return true;
        }
    }
    return false;
}

struct MorphTarget
{
    Geometry *parent;
    std::vector<UFixedArray<float, 3>> vertices;
    std::vector<UFixedArray<float, 3>> normals;
    BoundingSphere<float, 3> bounding_sphere = uninitialized;
};

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

struct TexDictionary
{
    enum { ID = 6 };
    std::list<Texture*> textures;
};

struct CameraChunkData
{
    FixedArray<float, 2> viewWindow = uninitialized;
    FixedArray<float, 2> viewOffset = uninitialized;
    float nearPlane, farPlane;
    float fogPlane;
    int32_t projection;
};
static_assert(sizeof(CameraChunkData) == 32);

enum class ExtensionNotFoundBehavior {
    IGNORE,
    WARN,
    RAISE
};

enum class PluginNotFoundBehavior {
    IGNORE,
    WARN,
    RAISE
};

template <class TObject>
static void read_extension(
    std::istream& istr,
    TObject& object,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    ExtensionNotFoundBehavior extension_not_found_behavior,
    PluginNotFoundBehavior plugin_not_found_behavior,
    const char* type)
{
    uint32_t length;
    if (!find_chunk(istr, ID_EXTENSION, &length, nullptr)) {
        switch (extension_not_found_behavior) {
        case ExtensionNotFoundBehavior::IGNORE:
            return;
        case ExtensionNotFoundBehavior::WARN:
            lwarn() << "Could not find extension chunk for type " << type;
            return;
        case ExtensionNotFoundBehavior::RAISE:
            THROW_OR_ABORT("Could not find extension chunk for type " + std::string(type));
        }
        verbose_abort("Unknown extension_not_found_behavior");
    }
    while (length != 0) {
        ChunkHeaderInfo header = read_chunk_header_info(istr);
        if (length < sizeof(ChunkHeaderBuf)) {
            THROW_OR_ABORT("Unexpected length");
        }
        length -= sizeof(ChunkHeaderBuf);
        for (const auto& plugin : plugins) {
            if (plugin->id == header.type) {
                if (plugin->read(istr, header, object)) {
                    goto cont;
                }
            }
        }
        [&]() {
            switch (plugin_not_found_behavior) {
            case PluginNotFoundBehavior::IGNORE:
                return;
            case PluginNotFoundBehavior::WARN:
                lwarn() << "Could not find plugin with ID 0x" << std::hex << header.type << " for type " << type;
                return;
            case PluginNotFoundBehavior::RAISE:
                THROW_OR_ABORT((std::stringstream() << "Could not find plugin with ID 0x" << std::hex << header.type << " for type " << type).str());
            }
            verbose_abort("Unknown plugin_not_found_behavior");
            }();
        seek_relative_positive(istr, header.length);
    cont:
        if (length < header.length) {
            THROW_OR_ABORT("Unexpected header length");
        }
        length -= header.length;
    }

    // now the always callbacks
    for (const auto& plugin : plugins) {
        plugin->always_callback(object);
    }
}

static std::vector<Frame> read_frame_list(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto num_frames = read_binary<uint32_t>(istr, "frame list");
    if (num_frames > 100) {
        THROW_OR_ABORT("Unexpected number of frames");
    }
    std::vector<Frame> frames(num_frames);
    for (auto& frame : frames) {
        auto buf = read_binary<FrameStreamData>(istr, "frame stream data");
        frame.matrix = {
            FixedArray<float, 3, 3>::init(
                buf.right(0), buf.up(0), buf.at(0),
                buf.right(1), buf.up(1), buf.at(1),
                buf.right(2), buf.up(2), buf.at(2)),
            buf.pos };
        if (buf.parent >= 0) {
            if (buf.parent >= frames.size()) {
                THROW_OR_ABORT("Parent frame ID too large");
            }
        } else if (buf.parent != -1) {
            THROW_OR_ABORT("Unexpected parent frame ID");
        }
        frame.parent = buf.parent;
    }
    for (auto& frame : frames) {
        read_extension(
            istr,
            frame,
            plugins,
            ExtensionNotFoundBehavior::WARN,
            PluginNotFoundBehavior::WARN,
            "frame");
    }
    return frames;
}

struct GeoStreamData
{
    uint32_t flags;
    int32_t numTriangles;
    int32_t numVertices;
    int32_t numMorphTargets;
};
static_assert(sizeof(GeoStreamData) == 16);

struct MatStreamData
{
    int32_t flags;    // unused according to RW
    RGBA  color;
    int32_t unused;
    int32_t textured;
};
static_assert(sizeof(MatStreamData) == 16);

struct DffConfig {
    bool mipmapping = false;
    bool auto_mipmapping = false;
};

// static void stream_skip(std::istream& istr)
// {
//     uint32_t length;
//     if(!find_chunk(istr, ID_EXTENSION, &length, nullptr))
//         return;
//     while (length > 0) {
//         ChunkHeaderInfo header = read_chunk_header_info(istr);
//         seek_relative_positive(istr, header.length);
//         if (length < sizeof(ChunkHeaderBuf) + header.length) {
//             THROW_OR_ABORT("Unexpected chunk header info");
//         }
//         length -= sizeof(ChunkHeaderBuf) + header.length;
//     }
// }

static Texture read_texture(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
        THROW_OR_ABORT("Could not find struct");
    }

    Texture texture;

    texture.filterAddressing = read_binary<uint32_t>(istr, "filter addressing");
    // if V addressing is 0, copy U
    if((texture.filterAddressing & 0xF000) == 0)
        texture.filterAddressing |= (texture.filterAddressing & 0xF00) << 4;

    // if using mipmap filter mode, set automipmapping,
    // if 0x10000 is set, set mipmapping
    uint32_t length;
    if (!find_chunk(istr, ID_STRING, &length, nullptr)) {
        THROW_OR_ABORT("Could not find string");
    }
    texture.name = read_string(istr, length, "name");

    if (!find_chunk(istr, ID_STRING, &length, nullptr)){
        THROW_OR_ABORT("Could not find string");
    }
    texture.mask = read_string(istr, length, "mask");

    // uint32_t mipState = cfg.mipmapping;
    // uint32_t autoMipState = cfg.auto_mipmapping;
    // int32_t filter = filterAddressing & 0xFF;
    // if(filter == Texture::MIPNEAREST || filter == Texture::MIPLINEAR ||
    //     filter == Texture::LINEARMIPNEAREST || filter == Texture::LINEARMIPLINEAR){
    //     cfg.mipmapping = true;
    //     cfg.auto_mipmapping = ((filterAddressing&0x10000) == 0);
    // }else{
    //     cfg.mipmapping = false;
    //     cfg.auto_mipmapping = false;
    // }
    // 
    // cfg.mipmapping = mipState;
    // cfg.auto_mipmapping = autoMipState;
    // 
    // if(tex->refCount == 1)
    //     tex->filterAddressing = filterAddressing&0xFFFF;

    read_extension(
        istr,
        texture,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "texture");
    return texture;
}

static Material read_material(
    std::istream& istr,
    const std::optional<SurfaceProperties>& default_surface_properties,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    uint32_t version;
    if (!find_chunk(istr, ID_STRUCT, nullptr, &version)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto buf = read_binary<MatStreamData>(istr, "material stream data");
    RGBA col = buf.color;
    buf.color = col;
    Material material;
    material.color = buf.color;
    if (version < 0x30400) {
        if (!default_surface_properties.has_value()) {
            THROW_OR_ABORT("Default surface properties not specified");
        }
        material.surfaceProps = default_surface_properties.value();
    } else {
        material.surfaceProps = read_binary<SurfaceProperties>(istr, "surface properties");
    }
    if (buf.textured) {
        uint32_t length;
        if (!find_chunk(istr, ID_TEXTURE, &length, nullptr)){
            THROW_OR_ABORT("Could not find texture");
        }
        material.texture = read_texture(istr, plugins);
    }

    read_extension(
        istr,
        material,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "material");
    return material;
}

static MaterialList read_material_list(
    std::istream& istr,
    const std::optional<SurfaceProperties>& default_surface_properties,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    MaterialList matlist;
    if( !find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
        THROW_OR_ABORT("Could not find struct");
    }
    int32_t num_mat = read_binary<int32_t>(istr, "num mat");
    if(num_mat == 0)
        return matlist;
    if (num_mat > 100) {
        THROW_OR_ABORT("Number of materials too large");
    }
    matlist.materials.resize(num_mat);
    matlist.space = num_mat;

    std::vector<int32_t> indices(num_mat);
    read_vector(istr, indices, "indices");

    for (int32_t i = 0; i < num_mat; i++){
        if (indices[i] >= 0) {
            if (indices[i] >= i) {
                THROW_OR_ABORT("Detected material forward reference");
            }
            matlist.materials[i] = matlist.materials[indices[i]];
        } else {
            if (!find_chunk(istr, ID_MATERIAL, nullptr, nullptr)) {
                THROW_OR_ABORT("Could not find material");
            }
            matlist.materials[i] = read_material(istr, default_surface_properties, plugins);
        }
    }
    return matlist;
}

Geometry::Geometry(
    int32_t numVerts,
    int32_t numTris,
    uint32_t flags)
{
    this->flags = flags & 0xFF00FFFF;
    this->numTexCoordSets = (flags & 0xFF0000) >> 16;
    if (this->numTexCoordSets == 0)
        this->numTexCoordSets = (this->flags & TEXTURED) ? 1 :
        (this->flags & TEXTURED2) ? 2 : 0;
    this->numTriangles = numTris;
    this->numVertices = numVerts;

    if (this->numTriangles > 10'000) {
        THROW_OR_ABORT("Number of triangles too large");
    }
    if (this->numVertices > 10'000) {
        THROW_OR_ABORT("Number of vertices too large");
    }

    if (!(this->flags & NATIVE)) {
        this->triangles.resize(this->numTriangles);
        if (this->flags & PRELIT && this->numVertices) {
            this->colors.resize(numVertices);
        }
        for (int32_t i = 0; i < this->numTexCoordSets; i++) {
            this->texCoords[i].resize(this->numVertices);
        }

        // init triangles
        for (int32_t i = 0; i < this->numTriangles; i++) {
            this->triangles[i].matId = 0xFFFF;
        }
    }
}

static std::shared_ptr<Geometry> read_geometry(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    uint32_t version;
    if(!find_chunk(istr, ID_STRUCT, nullptr, &version)){
        THROW_OR_ABORT("Could nto find struct");
    }
    auto buf = read_binary<GeoStreamData>(istr, "geometry stream data");
    auto geo = std::make_shared<Geometry>(buf.numVertices, buf.numTriangles, buf.flags);
    if (buf.numMorphTargets > 100) {
        THROW_OR_ABORT("Number of morph targets too large");
    }
    geo->morphTargets.resize(buf.numMorphTargets);
    SurfaceProperties surfProps;
    if (version < 0x34000) {
        surfProps = read_binary<SurfaceProperties>(istr, "surface properties");
    }

    if (!(geo->flags & Geometry::NATIVE)){
        if (geo->flags & Geometry::PRELIT) {
            read_vector(istr, geo->colors, "vertices");
        }
        for (int32_t i = 0; i < geo->numTexCoordSets; i++) {
            read_vector(istr, geo->texCoords[i], "texture coordinates");
        }
        for(int32_t i = 0; i < geo->numTriangles; i++){
            auto tribuf = read_binary<UFixedArray<uint32_t, 2>>(istr, "triangle buffer");
            geo->triangles[i].v[0]  = tribuf(0) >> 16;
            geo->triangles[i].v[1]  = tribuf(0);
            geo->triangles[i].v[2]  = tribuf(1) >> 16;
            geo->triangles[i].matId = tribuf(1);
        }
    }

    for (auto& m : geo->morphTargets) {
        m.bounding_sphere = read_binary<UBoundingSphere<float, 3>>(istr, "bounding sphere");
        int32_t has_vertices = read_binary<int32_t>(istr, "has vertices");
        int32_t has_normals = read_binary<int32_t>(istr, "has normals");
        if (has_vertices) {
            m.vertices.resize(geo->numVertices);
            read_vector(istr, m.vertices, "vertices");
        }
        if (has_normals) {
            m.normals.resize(geo->numVertices);
            read_vector(istr, m.normals, "normals");
        }
    }

    if (!find_chunk(istr, ID_MATLIST, nullptr, nullptr)){
        THROW_OR_ABORT("Could not find matlist");
    }
    std::optional<SurfaceProperties> defaultSurfaceProps;
    if (version < 0x34000) {
        defaultSurfaceProps = surfProps;
    }
    geo->matList = read_material_list(istr, defaultSurfaceProps, plugins);
    read_extension(
        istr,
        *geo,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "geometry");
    return geo;
}

static Atomic read_atomic(
    std::istream& istr,
    const std::vector<Frame>& frames,
    const std::vector<std::shared_ptr<Geometry>>& geometries,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    uint32_t version;
    if (!find_chunk(istr, ID_STRUCT, nullptr, &version)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto frame_id = read_binary<uint32_t>(istr, "frame ID");
    auto geometry_id = read_binary<uint32_t>(istr, "geometry ID");
    auto object_flags = read_binary<int32_t>(istr, "object flags");
    if (version >= 0x30400) {
        read_binary<int32_t>(istr, "unknown value in DFF");
    }
    if (frame_id >= frames.size()) {
        THROW_OR_ABORT("Frame ID too large");
    }
    std::shared_ptr<Geometry> g;
    if (version < 0x30400){
        if (!find_chunk(istr, ID_GEOMETRY, nullptr, nullptr)){
            THROW_OR_ABORT("Could not find geometry");
        }
        g = read_geometry(istr, plugins);
    } else {
        g = geometries[geometry_id];
    }

    Atomic atomic{ &frames[frame_id], g, Object{ .flags = object_flags } };
    read_extension(
        istr,
        atomic,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "atomic");
    return atomic;
}

struct LightChunkData
{
    float radius;
    float red, green, blue;
    float minusCosAngle;
    uint32_t type_flags;
};
static_assert(sizeof(LightChunkData) == 24);

static Light read_light(
    std::istream& istr,
    const Frame& frame,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    uint32_t version;
    if(!find_chunk(istr, ID_STRUCT, nullptr, &version)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto buf = read_binary<LightChunkData>(istr, "light chunk data");
    Light light;

    light.object.object.type = buf.type_flags >> 16;
    light.radius = buf.radius;
    light.color = { buf.red, buf.green, buf.blue, 1.f };
    float a = buf.minusCosAngle;
    if(version >= 0x30300)
        light.minusCosAngle = a;
    else
        // tan -> -cos
        light.minusCosAngle = -1.0f / std::sqrtf(a * a + 1.0f);
    light.object.object.flags = (uint8_t)buf.type_flags;
    read_extension(
        istr,
        light,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "light");
    return light;
}

static Camera read_camera(
    std::istream& istr,
    Frame& frame,
    const std::list<std::unique_ptr<IPlugin>>& plugins)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto buf = read_binary<CameraChunkData>(istr, "camera chunk data");
    Camera camera;
    camera.object.inFrame = &frame;
    camera.viewWindow = buf.viewWindow;
    camera.viewOffset = buf.viewOffset;
    camera.nearPlane = buf.nearPlane;
    camera.farPlane = buf.farPlane;
    camera.fogPlane = buf.fogPlane;
    camera.projection = buf.projection;
    read_extension(
        istr,
        camera,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "camera");
    return camera;
}

class NamePlugin: public IPlugin {
public:
    NamePlugin() {
        id = 0x253f2fe;
    }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Frame& frame) override {
        frame.name = read_string(istr, header.length, "frame name");
        return true;
    }
};

static Clump read_clump(std::istream& istr)
{
    uint32_t length;
    uint32_t version;

    if (!find_chunk(istr, ID_STRUCT, &length, &version)){
        THROW_OR_ABORT("Could not find struct");
    }
    if (length < 4) {
        THROW_OR_ABORT("Struct size too small");
    }
    int32_t numAtomics = read_binary<int32_t>(istr, "number of atomics");
    int32_t numLights = 0;
    int32_t numCameras = 0;
    if (version > 0x33000) {
        if (length != 12) {
            THROW_OR_ABORT("Struct size is not 12");
        }
        numLights = read_binary<int32_t>(istr, "number of lights");
        numCameras = read_binary<int32_t>(istr, "number of cameras");
    }

    // Frame list
    if (!find_chunk(istr, ID_FRAMELIST, nullptr, nullptr)) {
        THROW_OR_ABORT("Could not find frame list");
    }
    std::list<std::unique_ptr<IPlugin>> plugins;
    plugins.push_back(std::make_unique<NamePlugin>());
    std::vector<Frame> frames = read_frame_list(istr, plugins);

    // Geometry list
    std::vector<std::shared_ptr<Geometry>> geometries;
    if (version >= 0x30400) {
        if (!find_chunk(istr, ID_GEOMETRYLIST, nullptr, nullptr)) {
            THROW_OR_ABORT("Could not find geometry list");
        }
        if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
            THROW_OR_ABORT("Could not find struct");
        }
        uint32_t num_geometries = read_binary<uint32_t>(istr, "number of geometries");
        if (num_geometries > 1000) {
            THROW_OR_ABORT("Large number of geometries");
        }
        geometries.resize(num_geometries);
        for (auto& geometry : geometries) {
            if (!find_chunk(istr, ID_GEOMETRY, nullptr, nullptr)){
                THROW_OR_ABORT("Could not find geometry");
            }
            geometry = read_geometry(istr, plugins);
        }
    }

    // Atomics
    std::vector<Atomic> atomics(numAtomics);
    for (auto& atomic : atomics) {
        if (!find_chunk(istr, ID_ATOMIC, nullptr, nullptr)){
            THROW_OR_ABORT("Could not find atomic");
        }
        atomic = read_atomic(istr, frames, geometries, plugins);
    }

    // Lights
    if (numLights > 1'000) {
        THROW_OR_ABORT("Number of lights too large");
    }
    std::vector<Light> lights(numLights);
    for (auto& light : lights) {
        if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
            THROW_OR_ABORT("Could not find struct");
        }
        auto frame_id = read_binary<uint32_t>(istr, "frame ID");
        if(!find_chunk(istr, ID_LIGHT, nullptr, nullptr)){
            THROW_OR_ABORT("Could not find light");
        }
        if (frame_id >= frames.size()) {
            THROW_OR_ABORT("Frame ID too large");
        }
        light = read_light(istr, frames[frame_id], plugins);
    }

    // Cameras
    if (numCameras > 1'000) {
        THROW_OR_ABORT("Number of cameras too large");
    }
    std::vector<Camera> cameras(numCameras);
    for (auto& camera : cameras) {
        if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
            THROW_OR_ABORT("Could not read struct");
        }
        auto frame_id = read_binary<uint32_t>(istr, "frame ID");
        if (!find_chunk(istr, ID_CAMERA, nullptr, nullptr)){
            THROW_OR_ABORT("Could not find camera");
        }
        if (frame_id >= frames.size()) {
            THROW_OR_ABORT("Frame ID too large");
        }
        camera = read_camera(istr, frames[frame_id], plugins);
    }

    if (frames.empty()) {
        THROW_OR_ABORT("Frame list is empty");
    }
    Clump clump{ &frames.front(), std::move(frames), std::move(atomics), std::move(lights) };

    read_extension(
        istr,
        clump,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "clump");
    return clump;
}

Clump Mlib::Dff::read_dff(std::istream& istr)
{
    if (!find_chunk(istr, ID_CLUMP, nullptr, nullptr)) {
        THROW_OR_ABORT("Could not find clump");
    }
    return read_clump(istr);
}
