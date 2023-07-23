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

// From: https://github.com/RaduMC/kn5-converter/blob/master/kn5%20converter/Program.cs

struct kn5Material
{
    std::string name = "Default";
    std::string shader = "";
    float ksEmissive = 0.f;
    float ksAmbient = 0.6f;
    float ksDiffuse = 0.6f;
    float ksSpecular = 0.9f;
    float ksSpecularEXP = 1.0f;
    float diffuseMult = 1.0f;
    float normalMult = 1.0f;
    float useDetail = 0.0f;
    float detailUVMultiplier = 1.0f;

    std::string txDiffuse;
    std::string txNormal;
    std::string txMask;
    std::string txDetailR;
    std::string txDetailG;
    std::string txDetailB;
    std::string txDetailA;
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
