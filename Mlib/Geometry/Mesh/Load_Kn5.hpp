#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <cstdint>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

// From: https://github.com/gro-ove/actools/tree/master

enum class kn5BlendMode: uint8_t {
    OPAQUE = 0,
    ALPHA_BLEND = 1,
    ALPHA_TO_COVERAGE = 2
};

enum class kn5MaterialDepthMode: uint8_t {
    DEPTH_NORMAL = 0,
    DEPTH_NO_WRITE = 1,
    DEPTH_OFF = 2
};

// From: https://github.com/RaduMC/kn5-converter/blob/master/kn5%20converter/Program.cs

struct kn5Material
{
    std::string name = "Default";
    std::string shader = "";
    kn5BlendMode blendMode;
    bool alphaTested;
    kn5MaterialDepthMode depthMode;
    float ksEmissive = 0.f;
    float ksAmbient = 0.6f;
    float ksDiffuse = 0.6f;
    float ksSpecular = 0.9f;
    float ksSpecularEXP = 1.0f;
    float ksAlphaRef = 0.f;
    float diffuseMult = 1.0f;
    float normalMult = 1.0f;
    float useDetail = 0.0f;
    float detailUVMultiplier = 4.f;
    FixedArray<float, 4> mult = {1.f, 1.f, 1.f, 1.f};
    float detailNMMult = 1.f;
    float magicMult = 2.f;

    std::string txDiffuse;
    std::string txNormal;
    std::string txMask;
    FixedArray<std::string, 4> txDetail;
    std::string txDetailNM;

    std::string shaderProps = "";
};

struct kn5Node
{
    int type = 1;
    std::string name = "Default";

    TransformationMatrix<float, float, 3> tmatrix;
    TransformationMatrix<float, float, 3> hmatrix;

    bool isActive = true;
    bool isRenderable = true;
    bool isTransparent = false;

    size_t vertexCount = 0;
    std::vector<FixedArray<float, 3>> position;
    std::vector<FixedArray<float, 3>> normal;
    std::vector<FixedArray<float, 2>> uv;

    std::vector<FixedArray<uint16_t, 3>> triangles;

    std::optional<size_t> materialID;

    //std::list<kn5Node> children; //do I really wanna do this? no
    std::optional<size_t> parentID;
};

struct kn5Model
{
    int version;
    std::map<std::string, std::vector<uint8_t>> textures;
    std::map<size_t, kn5Material> materials;
    std::map<size_t, kn5Node> nodes;
};

kn5Model load_kn5(const std::string& filename, bool verbose = false);

}
