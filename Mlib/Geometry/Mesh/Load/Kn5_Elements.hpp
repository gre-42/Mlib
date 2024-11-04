#pragma once
#include <Mlib/Default_Optional.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstddef>
#include <cstdint>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
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
    DefaultOptional<float> ksEmissive          = DefaultOptional<float>::from_default(0.f);
    DefaultOptional<float> ksAmbient           = DefaultOptional<float>::from_default(0.6f);
    DefaultOptional<float> ksDiffuse           = DefaultOptional<float>::from_default(0.6f);
    DefaultOptional<float> ksSpecular          = DefaultOptional<float>::from_default(0.5f);
    DefaultOptional<float> ksSpecularEXP       = DefaultOptional<float>::from_default(50.0f);
    DefaultOptional<float> ksAlphaRef          = DefaultOptional<float>::from_default(0.f);
    DefaultOptional<float> gain                = DefaultOptional<float>::from_default(1.0f);
    DefaultOptional<float> diffuseMult         = DefaultOptional<float>::from_default(1.0f);
    DefaultOptional<float> normalMult          = DefaultOptional<float>::from_default(1.0f);
    DefaultOptional<float> useDetail           = DefaultOptional<float>::from_default(0.0f);
    DefaultOptional<float> detailUVMultiplier  = DefaultOptional<float>::from_default(1.f);
    FixedArray<DefaultOptional<float>, 4> mult = {
        DefaultOptional<float>::from_default(1.f),
        DefaultOptional<float>::from_default(1.f),
        DefaultOptional<float>::from_default(1.f),
        DefaultOptional<float>::from_default(1.f) };
    DefaultOptional<float> detailNMMult        = DefaultOptional<float>::from_default(1.f);
    DefaultOptional<float> magicMult           = DefaultOptional<float>::from_default(1.f);
    DefaultOptional<float> fresnelC            = DefaultOptional<float>::from_default(0.f);
    DefaultOptional<float> fresnelEXP          = DefaultOptional<float>::from_default(0.f);
    DefaultOptional<float> fresnelMaxLevel     = DefaultOptional<float>::from_default(0.f);

    VariableAndHash<std::string> txDiffuse;
    VariableAndHash<std::string> txNormal;
    VariableAndHash<std::string> txMask;
    FixedArray<VariableAndHash<std::string>, 4> txDetail4 = uninitialized;
    VariableAndHash<std::string> txDetail1;
    VariableAndHash<std::string> txDetailNM;
    VariableAndHash<std::string> txVariation;

    std::string shaderProps = "";
};

struct kn5Vertex {
    FixedArray<float, 3> position = uninitialized;
    FixedArray<float, 3> normal = uninitialized;
    FixedArray<float, 2> uv = uninitialized;
    FixedArray<float, 3> tangent = uninitialized;
};
static_assert(sizeof(kn5Vertex) == 4 * (3 + 3 + 2 + 3));

struct kn5AnimatedVertex {
    FixedArray<float, 3> position = uninitialized;
    FixedArray<float, 3> normal = uninitialized;
    FixedArray<float, 2> uv = uninitialized;
    FixedArray<float, 3> tangent = uninitialized;
    char weights[32];
};
static_assert(sizeof(kn5AnimatedVertex) == 4 * (3 + 3 + 2 + 3) + 32);

struct kn5Node
{
    int32_t type = 1;
    std::string name = "Default";

    TransformationMatrix<float, float, 3> tmatrix;
    TransformationMatrix<float, float, 3> hmatrix;

    bool isActive = true;
    bool isRenderable = true;
    bool isTransparent = false;

    std::vector<kn5Vertex> vertices;
    std::vector<kn5AnimatedVertex> animatedVertices;

    UUVector<FixedArray<uint16_t, 3>> triangles;

    std::optional<size_t> materialID;

    //std::list<kn5Node> children; //do I really wanna do this? no
    std::optional<size_t> parentID;

    const FixedArray<float, 3>& position(size_t i) const {
        return vertices.empty() ? animatedVertices.at(i).position : vertices.at(i).position;
    }
    const FixedArray<float, 3>& normal(size_t i) const {
        return vertices.empty() ? animatedVertices.at(i).normal : vertices.at(i).normal;
    }
    const FixedArray<float, 2>& uv(size_t i) const {
        return vertices.empty() ? animatedVertices.at(i).uv : vertices.at(i).uv;
    }
    const FixedArray<float, 3>& tangent(size_t i) const {
        return vertices.empty() ? animatedVertices.at(i).tangent : vertices.at(i).tangent;
    }
};

enum class kn5LoadOptions {
    VERSION,
    TEXTURES,
    MATERIALS,
    NODES,
    ALL = NODES
};

struct kn5Texture {
    int32_t type;
    std::vector<std::byte> data;
};

struct kn5Model
{
    int32_t version;
    std::optional<int> unknownNo;
    std::unordered_map<VariableAndHash<std::string>, kn5Texture> textures;
    std::unordered_map<size_t, kn5Material> materials;
    std::unordered_map<size_t, kn5Node> nodes;
};

}
