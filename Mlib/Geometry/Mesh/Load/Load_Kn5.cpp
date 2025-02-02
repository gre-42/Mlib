#include "Load_Kn5.hpp"
#include <Mlib/Geometry/Mesh/Load/Kn5_Elements.hpp>
#include <Mlib/Images/Image_Info.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>
#include <sstream>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

static std::string ModifiedTextureName(const std::string& name) {
    if (name.find('.') != std::string::npos) {
        return name;
    }
    if (name.ends_with("_dds_img")) {
        return name + ".dds";
    }
    if (name.ends_with("_bmp_img")) {
        return name + ".bmp";
    }
    return name + ".dds";
}

// From: https://github.com/RaduMC/kn5-converter/blob/master/kn5%20converter/Program.cs

template <class T>
T ReadBinary(std::istream& str, const char* msg) {
    return read_binary<T>(str, msg, IoVerbosity::SILENT);
}

template <class TVec>
static void ReadVector(std::istream& str, TVec& vec, const char* msg) {
    read_vector(str, vec, msg, IoVerbosity::SILENT);
}

static std::string ReadStr(std::istream& str, uint32_t len, const char* msg) {
    return read_string(str, len, msg, IoVerbosity::SILENT);
}

static int32_t ReadInt32(std::istream& str) {
    return ReadBinary<int32_t>(str, "int32");
}

static float ReadSingle(std::istream& str) {
    return ReadBinary<float>(str, "float");
}

static uint8_t ReadByte(std::istream& str) {
    return ReadBinary<uint8_t>(str, "byte");
}

static uint32_t ReadUInt32(std::istream& str) {
    return ReadBinary<uint32_t>(str, "uint32");
}

static void readNodes(
    std::istream& modelStream,
    std::unordered_map<size_t, kn5Node>& nodeList,
    std::optional<size_t> parentID,
    const kn5Model& model,
    bool verbose)
{
    kn5Node newNode{ .tmatrix = uninitialized, .hmatrix = uninitialized };
    newNode.parentID = parentID;

    newNode.type = ReadInt32(modelStream);
    newNode.name = ReadStr(modelStream, ReadUInt32(modelStream), "node name");
    size_t childrenCount = ReadUInt32(modelStream);
    ReadByte(modelStream); // abyte

    switch (newNode.type)
    {
        // dummy node
        case 1: //dummy
            {
                FixedArray<float, 4, 4> tmatrix = uninitialized;
                tmatrix(0, 0) = ReadSingle(modelStream);
                tmatrix(1, 0) = ReadSingle(modelStream);
                tmatrix(2, 0) = ReadSingle(modelStream);
                tmatrix(3, 0) = ReadSingle(modelStream);
                tmatrix(0, 1) = ReadSingle(modelStream);
                tmatrix(1, 1) = ReadSingle(modelStream);
                tmatrix(2, 1) = ReadSingle(modelStream);
                tmatrix(3, 1) = ReadSingle(modelStream);
                tmatrix(0, 2) = ReadSingle(modelStream);
                tmatrix(1, 2) = ReadSingle(modelStream);
                tmatrix(2, 2) = ReadSingle(modelStream);
                tmatrix(3, 2) = ReadSingle(modelStream);
                tmatrix(0, 3) = ReadSingle(modelStream);
                tmatrix(1, 3) = ReadSingle(modelStream);
                tmatrix(2, 3) = ReadSingle(modelStream);
                tmatrix(3, 3) = ReadSingle(modelStream);

                newNode.tmatrix = TransformationMatrix<float, float, 3>{tmatrix};
            
                break;
            }
        // mesh node
        case 2: //mesh
            {
                newNode.tmatrix = TransformationMatrix<float, float, 3>::identity();

                newNode.isActive = (bool)ReadByte(modelStream);
                newNode.isRenderable = (bool)ReadByte(modelStream);
                newNode.isTransparent = (bool)ReadByte(modelStream);

                uint32_t vertexCount = ReadUInt32(modelStream);
                newNode.vertices.resize(vertexCount);
                ReadVector(modelStream, newNode.vertices, "vertices");

                for (auto& vertex : newNode.vertices) {
                    vertex.uv(1) = 1 - vertex.uv(1);
                }

                size_t indexCount = ReadUInt32(modelStream);
                if (indexCount % 3 != 0) {
                    THROW_OR_ABORT("Index-count not divisible by 3");
                }
                newNode.triangles.resize(indexCount / 3);
                ReadVector(modelStream, newNode.triangles, "triangles");

                newNode.materialID = ReadInt32(modelStream);
                modelStream.seekg(29, std::ios::cur);

                break;
            }
        // animated mesh
        case 3: //animated mesh
            {
                newNode.tmatrix = TransformationMatrix<float, float, 3>::identity();
                
                newNode.isActive = (bool)ReadByte(modelStream);
                newNode.isRenderable = (bool)ReadByte(modelStream);
                newNode.isTransparent = (bool)ReadByte(modelStream);

                int boneCount = ReadInt32(modelStream);
                for (int b = 0; b < boneCount; b++)
                {
                    std::string boneName = ReadStr(modelStream, ReadUInt32(modelStream), "bone name");
                    modelStream.seekg(64, std::ios::cur); //transformation matrix
                }

                uint32_t vertexCount = ReadUInt32(modelStream);
                newNode.animatedVertices.resize(vertexCount);

                ReadVector(modelStream, newNode.animatedVertices, "animated vertices");
                for (auto& vertex : newNode.animatedVertices) {
                    vertex.uv(1) = 1 - vertex.uv(1);
                }

                size_t indexCount = ReadUInt32(modelStream);
                if (indexCount % 3 != 0) {
                    THROW_OR_ABORT("Index-count not divisible by 3");
                }
                newNode.triangles.resize(indexCount / 3);
                ReadVector(modelStream, newNode.triangles, "triangles");

                newNode.materialID = ReadInt32(modelStream);
                modelStream.seekg(12,  std::ios::cur);

                break;
            }
    }

    if (!parentID.has_value()) { newNode.hmatrix = newNode.tmatrix; }
    else { newNode.hmatrix = nodeList.at(*parentID).hmatrix * newNode.tmatrix; }

    if (verbose) {
        std::stringstream matInfo;
        if (newNode.materialID.has_value()) {
            const auto& material = model.materials.at(*newNode.materialID);
            matInfo <<
                " material: " << material.name <<
                " shader: " << material.shader;
            if (!material.txDiffuse->empty()) {
                matInfo << " diffuse: " << *material.txDiffuse;
                auto info = ImageInfo::load(*material.txDiffuse, &model.textures.at(material.txDiffuse).data);
                matInfo << ' ' << info.size(0) << 'x' << info.size(1);
            }
            if (!material.txNormal->empty()) {
                matInfo << " normal: " << *material.txNormal;
            }
            if (!material.txMask->empty()) {
                matInfo << " mask: " << *material.txMask;
            }
            if (material.mult(0).has_value()) {
                matInfo << " multR: " << material.mult(0).value();
            }
            if (material.mult(1).has_value()) {
                matInfo << " multG: " << material.mult(1).value();
            }
            if (material.mult(2).has_value()) {
                matInfo << " multB: " << material.mult(2).value();
            }
            if (material.mult(3).has_value()) {
                matInfo << " multA: " << material.mult(3).value();
            }
            if (material.detailUVMultiplier.has_value()) {
                matInfo << " detailUVMultiplier: " << material.detailUVMultiplier.value();
            }
            if (material.detailNMMult.has_value()) {
                matInfo << " detailNMMult: " << material.detailNMMult.value();
            }
            if (!material.txDetail4(0)->empty()) {
                matInfo << " detailR: " << *material.txDetail4(0);
            }
            if (!material.txDetail4(1)->empty()) {
                matInfo << " detailG: " << *material.txDetail4(1);
            }
            if (!material.txDetail4(2)->empty()) {
                matInfo << " detailB: " << *material.txDetail4(2);
            }
            if (!material.txDetail4(3)->empty()) {
                matInfo << " detailA: " << *material.txDetail4(3);
            }
            if (!material.txDetail1->empty()) {
                matInfo << " detail: " << *material.txDetail1;
            }
            if (!material.txDetailNM->empty()) {
                matInfo << " detailNM: " << *material.txDetailNM;
            }
            if (material.useDetail.value_or_default() == 1.f) {
                matInfo << " useDetail";
            }
            matInfo <<
                " phong: " << material.ksEmissive <<
                ' ' << material.ksAmbient <<
                ' ' << material.ksDiffuse <<
                ' ' << material.ksSpecular;
            matInfo << '\n' << material.shaderProps;
        }
        linfo() <<
            "Node: " << newNode.name <<
            " type: " << newNode.type <<
            " position: " << newNode.hmatrix.t <<
            " active: " << (int)newNode.isActive <<
            " renderable: " << (int)newNode.isRenderable <<
            " transparent: " << (int)newNode.isTransparent <<
            matInfo.str();
    }

    size_t currentID = nodeList.size();
    if (!nodeList.try_emplace(currentID, std::move(newNode)).second) {
        verbose_abort("load kn5 internal error");
    }

    for (size_t c = 0; c < childrenCount; c++)
    {
        readNodes(modelStream, nodeList, currentID, model, verbose);
    }
}

kn5Model Mlib::load_kn5(
    const std::string& filename,
    bool verbose,
    kn5LoadOptions opts)
{
    auto binStream = create_ifstream(filename, std::ios::binary);
    if (binStream->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    return load_kn5(*binStream, verbose, opts);
}

kn5Model Mlib::load_kn5(
    std::istream& binStream,
    bool verbose,
    kn5LoadOptions opts)
{
    std::string magicNumber = ReadStr(binStream, 6, "magic number");
    if (magicNumber != "sc6969") {
        THROW_OR_ABORT("Magic number mismatch");
    }
    kn5Model newModel;
    newModel.version = ReadInt32(binStream);
    if (newModel.version > 5) {
        newModel.unknownNo = ReadInt32(binStream);  //673425
    }

    // extract textures
    if (opts < kn5LoadOptions::TEXTURES) {
        return newModel;
    }
    int texCount = ReadInt32(binStream);
    for (int t = 0; t < texCount; t++)
    {
        int texType = ReadInt32(binStream);
        std::string texName = ModifiedTextureName(ReadStr(binStream, ReadUInt32(binStream), "texture name"));
        int texSize = ReadInt32(binStream);
        auto tex = newModel.textures.try_emplace(VariableAndHash{ texName }, kn5Texture{ .type = texType });
        if (!tex.second) {
            lwarn() << "Found multiple textures with name \"" << texName << "\", keeping the first one";
            binStream.ignore(texSize);
            continue;
        }

        if (verbose) {
            linfo() << "Texture: " << texName << " type: " << texType;
        }

        tex.first->second.data.resize(integral_cast<size_t>(texSize));
        ReadVector(binStream, tex.first->second.data, "texture data");
    }

    // read materials
    if (opts < kn5LoadOptions::MATERIALS) {
        return newModel;
    }
    size_t matCount = ReadUInt32(binStream);
    for (size_t m = 0; m < matCount; m++)
    {
        kn5Material newMaterial;

        newMaterial.name = ReadStr(binStream, ReadUInt32(binStream), "material name");
        newMaterial.shader = ReadStr(binStream, ReadUInt32(binStream), "shader");
        newMaterial.blendMode = (kn5BlendMode)ReadByte(binStream);
        newMaterial.alphaTested = (bool)ReadByte(binStream);
        if (newModel.version > 4) {
            newMaterial.depthMode = (kn5MaterialDepthMode)ReadByte(binStream);
            binStream.seekg(3, std::ios::cur);
        } else {
            newMaterial.depthMode = kn5MaterialDepthMode::DEPTH_NORMAL;
        }
        if (verbose) {
            linfo() <<
                "Material " << newMaterial.name <<
                ", shader " << newMaterial.shader <<
                ", blend-mode " << (uint32_t)newMaterial.blendMode <<
                ", alpha-tested " << (uint32_t)newMaterial.alphaTested <<
                ", depth-mode " << (uint32_t)newMaterial.depthMode;
        }

        size_t propCount = ReadUInt32(binStream);
        for (size_t p = 0; p < propCount; p++)
        {
            std::string propName = ReadStr(binStream, ReadUInt32(binStream), "property name");
            float propValue = ReadSingle(binStream);
            newMaterial.shaderProps += propName + " = " + std::to_string(propValue) + '\n';

            if (propName == "ksEmissive") {
                newMaterial.ksEmissive = propValue;
            } else if (propName == "ksAmbient") {
                newMaterial.ksAmbient = propValue;
            } else if (propName == "ksDiffuse") {
                newMaterial.ksDiffuse = propValue;
            } else if (propName == "ksSpecular") {
                newMaterial.ksSpecular = propValue;
            } else if (propName == "ksSpecularEXP") {
                newMaterial.ksSpecularEXP = propValue;
            } else if (propName == "ksAlphaRef") {
                newMaterial.ksAlphaRef = propValue;
            } else if (propName == "gain") {
                newMaterial.gain = propValue;
            } else if (propName == "diffuseMult") {
                newMaterial.diffuseMult = propValue;
            } else if (propName == "normalMult") {
                newMaterial.normalMult = propValue;
            } else if (propName == "useDetail") {
                newMaterial.useDetail = propValue;
            } else if (propName == "detailUVMultiplier") {
                newMaterial.detailUVMultiplier = propValue;
            } else if (propName == "multR") {
                newMaterial.mult(0) = propValue;
            } else if (propName == "multG") {
                newMaterial.mult(1) = propValue;
            } else if (propName == "multB") {
                newMaterial.mult(2) = propValue;
            } else if (propName == "multA") {
                newMaterial.mult(3) = propValue;
            } else if (propName == "detailNMMult") {
                newMaterial.detailNMMult = propValue;
            } else if (propName == "magicMult") {
                newMaterial.magicMult = propValue;
            } else if (propName == "fresnelC") {
                newMaterial.fresnelC = propValue;
            } else if (propName == "fresnelEXP") {
                newMaterial.fresnelEXP = propValue;
            } else if (propName == "fresnelMaxLevel") {
                newMaterial.fresnelMaxLevel = propValue;
            } else {
                lwarn() << "Unknown material property: " << propName << " = " << propValue;
            }

            binStream.seekg(36, std::ios::cur);
        }

        size_t textures = ReadUInt32(binStream);
        for (size_t t = 0; t < textures; t++)
        {
            std::string samplerName = ReadStr(binStream, ReadUInt32(binStream), "sampler name");
            int samplerSlot = ReadInt32(binStream);
            std::string texName = ModifiedTextureName(ReadStr(binStream, ReadUInt32(binStream), "texture name"));

            newMaterial.shaderProps += samplerName + " = " + texName + '\n';

            if (samplerName == "txDiffuse") {
                newMaterial.txDiffuse = texName;
            } else if (samplerName == "txNormal") {
                newMaterial.txNormal = texName;
            } else if (samplerName == "txMask") {
                newMaterial.txMask = texName;
            } else if (samplerName == "txDetailR") {
                newMaterial.txDetail4(0) = texName;
            } else if (samplerName == "txDetailG") {
                newMaterial.txDetail4(1) = texName;
            } else if (samplerName == "txDetailB") {
                newMaterial.txDetail4(2) = texName;
            } else if (samplerName == "txDetailA") {
                newMaterial.txDetail4(3) = texName;
            } else if (samplerName == "txDetailNM") {
                newMaterial.txDetailNM = texName;
            } else if (samplerName == "txDetail") {
                newMaterial.txDetail1 = texName;
            } else if (samplerName == "txVariation") {
                newMaterial.txVariation = texName;
            } else {
                lwarn() << "Unknown sampler name: " << samplerName;
            }
            lwarn() << "samplerSlot: " << samplerSlot;
        }

        newModel.materials[newModel.materials.size()] = std::move(newMaterial);
    }

    if (opts < kn5LoadOptions::NODES) {
        return newModel;
    }
    readNodes(binStream, newModel.nodes, std::nullopt, newModel, verbose); //recursive

    return newModel;
}
