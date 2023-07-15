#include "Load_Kn5.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>
#include <nv_dds/nv_dds.h>
#include <sstream>

using namespace Mlib;

// From: https://github.com/RaduMC/kn5-converter/blob/master/kn5%20converter/Program.cs

template <class T>
T ReadBinary(std::istream& str) {
    T result;
    str.read((char*)&result, sizeof(result));
    return result;
}

static std::string ReadStr(std::istream& str, uint32_t len) {
    //int len = str.ReadInt32();
    std::vector<uint8_t> stringData(len);
    str.read((char*)stringData.data(), len);
    if (str.fail()) {
        THROW_OR_ABORT("Could not read string from stream");
    }
    return std::string(stringData.begin(), stringData.end());
}

static int32_t ReadInt32(std::istream& str) {
    auto result = ReadBinary<int32_t>(str);;
    if (str.fail()) {
        THROW_OR_ABORT("Could not read int32 from stream");
    }
    return result;
}

static int16_t ReadInt16(std::istream& str) {
    auto result = ReadBinary<int16_t>(str);;
    if (str.fail()) {
        THROW_OR_ABORT("Could not read int16 from stream");
    }
    return result;
}

static float ReadSingle(std::istream& str) {
    auto result = ReadBinary<float>(str);;
    if (str.fail()) {
        THROW_OR_ABORT("Could not read float from stream");
    }
    return result;
}

static uint8_t ReadByte(std::istream& str) {
    auto result = ReadBinary<uint8_t>(str);;
    if (str.fail()) {
        THROW_OR_ABORT("Could not read byte from stream");
    }
    return result;
}

static uint32_t ReadUInt32(std::istream& str) {
    auto result = ReadBinary<uint32_t>(str);
    if (str.fail()) {
        THROW_OR_ABORT("Could not read uint32 from stream");
    }
    return result;
}

static uint16_t ReadUInt16(std::istream& str) {
    auto result = ReadBinary<uint16_t>(str);
    if (str.fail()) {
        THROW_OR_ABORT("Could not read uint16 from stream");
    }
    return result;
}

static void readNodes(
    std::istream& modelStream,
    std::map<size_t, kn5Node>& nodeList,
    std::optional<size_t> parentID,
    const kn5Model& model)
{
    kn5Node newNode;
    newNode.parentID = parentID;

    newNode.type = ReadInt32(modelStream);
    newNode.name = ReadStr(modelStream, ReadUInt32(modelStream));
    size_t childrenCount = ReadUInt32(modelStream);
    ReadByte(modelStream); // abyte

    switch (newNode.type)
    {
        // dummy node
        case 1: //dummy
            {
                FixedArray<float, 4, 4> tmatrix;
                tmatrix(0u, 0u) = ReadSingle(modelStream);
                tmatrix(0u, 1u) = ReadSingle(modelStream);
                tmatrix(0u, 2u) = ReadSingle(modelStream);
                tmatrix(0u, 3u) = ReadSingle(modelStream);
                tmatrix(1u, 0u) = ReadSingle(modelStream);
                tmatrix(1u, 1u) = ReadSingle(modelStream);
                tmatrix(1u, 2u) = ReadSingle(modelStream);
                tmatrix(1u, 3u) = ReadSingle(modelStream);
                tmatrix(2u, 0u) = ReadSingle(modelStream);
                tmatrix(2u, 1u) = ReadSingle(modelStream);
                tmatrix(2u, 2u) = ReadSingle(modelStream);
                tmatrix(2u, 3u) = ReadSingle(modelStream);
                tmatrix(3u, 0u) = ReadSingle(modelStream);
                tmatrix(3u, 1u) = ReadSingle(modelStream);
                tmatrix(3u, 2u) = ReadSingle(modelStream);
                tmatrix(3u, 3u) = ReadSingle(modelStream);

                newNode.tmatrix = TransformationMatrix<float, float, 3>{tmatrix};
            
                break;
            }
        // mesh node
        case 2: //mesh
            {
                newNode.isActive = (bool)ReadByte(modelStream);
                newNode.isRenderable = (bool)ReadByte(modelStream);
                newNode.isTransparent = (bool)ReadByte(modelStream);

                newNode.vertexCount = ReadUInt32(modelStream);
                newNode.position.resize(newNode.vertexCount);
                newNode.normal.resize(newNode.vertexCount);
                newNode.uv.resize(newNode.vertexCount);

                for (size_t v = 0; v < newNode.vertexCount; v++)
                {
                    newNode.position[v] = {
                        ReadSingle(modelStream),
                        ReadSingle(modelStream),
                        ReadSingle(modelStream)};

                    newNode.normal[v] = {
                        ReadSingle(modelStream),
                        ReadSingle(modelStream),
                        ReadSingle(modelStream)};

                    newNode.uv[v] = {
                        ReadSingle(modelStream),
                        1 - ReadSingle(modelStream)};

                    modelStream.seekg(12, std::ios::cur); //tangents
                }

                size_t indexCount = ReadUInt32(modelStream);
                if (indexCount % 3 != 0) {
                    THROW_OR_ABORT("Index-count not divisible by 3");
                }
                newNode.triangles.resize(indexCount / 3);
                for (size_t i = 0; i < indexCount / 3; i++) {
                    newNode.triangles[i] = {
                        ReadUInt16(modelStream),
                        ReadUInt16(modelStream),
                        ReadUInt16(modelStream)};
                }

                newNode.materialID = ReadInt32(modelStream);
                modelStream.seekg(29, std::ios::cur);

                break;
            }
        // animated mesh
        case 3: //animated mesh
            {
                newNode.isActive = (bool)ReadByte(modelStream);
                newNode.isRenderable = (bool)ReadByte(modelStream);
                newNode.isTransparent = (bool)ReadByte(modelStream);

                int boneCount = ReadInt32(modelStream);
                for (int b = 0; b < boneCount; b++)
                {
                    std::string boneName = ReadStr(modelStream, ReadUInt32(modelStream));
                    modelStream.seekg(64, std::ios::cur); //transformation matrix
                }

                newNode.vertexCount = ReadUInt32(modelStream);
                newNode.position.resize(newNode.vertexCount);
                newNode.normal.resize(newNode.vertexCount);
                newNode.uv.resize(newNode.vertexCount);

                for (size_t v = 0; v < newNode.vertexCount; v++)
                {
                    newNode.position[v] = {
                        ReadSingle(modelStream),
                        ReadSingle(modelStream),
                        ReadSingle(modelStream)};

                    newNode.normal[v] = {
                        ReadSingle(modelStream),
                        ReadSingle(modelStream),
                        ReadSingle(modelStream)};

                    newNode.uv[v] = {
                        ReadSingle(modelStream),
                        1 - ReadSingle(modelStream)};

                    modelStream.seekg(44, std::ios::cur); //tangents & weights
                }

                size_t indexCount = ReadUInt32(modelStream);
                if (indexCount % 3 != 0) {
                    THROW_OR_ABORT("Index-count not divisible by 3");
                }
                newNode.triangles.resize(indexCount / 3);
                for (size_t i = 0; i < indexCount / 3; i++) {
                    newNode.triangles[i] = {
                        ReadUInt16(modelStream),
                        ReadUInt16(modelStream),
                        ReadUInt16(modelStream)};
                }

                newNode.materialID = ReadInt32(modelStream);
                modelStream.seekg(12,  std::ios::cur);

                break;
            }
    }

    {
        std::stringstream matInfo;
        if (newNode.materialID.has_value()) {
            const auto& material = model.materials.at(newNode.materialID.value());
            matInfo << " shader: " << material.shader << " diffuse: " << material.txDiffuse;
            if (!material.txDiffuse.empty()) {
                std::stringstream sstr;
                for (uint8_t c : model.textures.at(material.txDiffuse)) {
                    sstr << c;
                }
                nv_dds::CDDSImage image;
                image.load(sstr);
                matInfo << ' ' << image.get_width() << 'x' << image.get_height();
            }
        }
        linfo() <<
            "Node: " << newNode.name <<
            " type: " << newNode.type <<
            " transparent: " << (int)newNode.isTransparent <<
            matInfo.str();
    }
    if (!parentID.has_value()) { newNode.hmatrix = newNode.tmatrix; }
    else { newNode.hmatrix = newNode.tmatrix * nodeList.at(parentID.value()).hmatrix; }

    size_t currentID = nodeList.size();
    nodeList[currentID] = std::move(newNode);

    for (size_t c = 0; c < childrenCount; c++)
    {
        readNodes(modelStream, nodeList, currentID, model);
    }
}

kn5Model Mlib::load_kn5(const std::string& filename) {
    auto binStream = create_ifstream(filename);
    if (binStream->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    std::string magicNumber = ReadStr(*binStream, 6);
    if (magicNumber != "sc6969") {
        THROW_OR_ABORT("Magic number mismatch");
    }
    kn5Model newModel;
    newModel.version = ReadInt32(*binStream);
    if (newModel.version > 5) { ReadInt32(*binStream); /* unknownNo */ } //673425

    // extract textures
    int texCount = ReadInt32(*binStream);
    for (int t = 0; t < texCount; t++)
    {
        int texType = ReadInt32(*binStream);
        std::string texName = ReadStr(*binStream, ReadUInt32(*binStream));
        int texSize = ReadInt32(*binStream);
        auto tex = newModel.textures.try_emplace(texName, texSize);
        if (!tex.second) {
            THROW_OR_ABORT("Found multiple textures with name \"" + texName + '"');
        }

        linfo() << "Texture: " << texName << " type: " << texType;

        binStream->read((char*)tex.first->second.data(), texSize);
    }

    // read materials
    size_t matCount = ReadUInt32(*binStream);
    for (size_t m = 0; m < matCount; m++)
    {
        kn5Material newMaterial;

        newMaterial.name = ReadStr(*binStream, ReadUInt32(*binStream));
        newMaterial.shader = ReadStr(*binStream, ReadUInt32(*binStream));
        linfo() << "Material " << newMaterial.name << ", shader " << newMaterial.shader;
        ReadInt16(*binStream); // ashort
        if (newModel.version > 4) { ReadInt32(*binStream); /* azero */ }

        size_t propCount = ReadUInt32(*binStream);
        for (size_t p = 0; p < propCount; p++)
        {
            std::string propName = ReadStr(*binStream, ReadUInt32(*binStream));
            float propValue = ReadSingle(*binStream);
            newMaterial.shaderProps += propName + " = " + std::to_string(propValue) + "&cr;&lf;";

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
            } else if (propName == "diffuseMult") {
                newMaterial.diffuseMult = propValue;
            } else if (propName == "normalMult") {
                newMaterial.normalMult = propValue;
            } else if (propName == "useDetail") {
                newMaterial.useDetail = propValue;
            } else if (propName == "detailUVMultiplier") {
                newMaterial.detailUVMultiplier = propValue;
            } else {
                linfo() << "Unknown material property: " << propName << " = " << propValue;
            }

            binStream->seekg(36, std::ios::cur);
        }

        size_t textures = ReadUInt32(*binStream);
        for (size_t t = 0; t < textures; t++)
        {
            std::string sampleName = ReadStr(*binStream, ReadUInt32(*binStream));
            int sampleSlot = ReadInt32(*binStream);
            std::string texName = ReadStr(*binStream, ReadUInt32(*binStream));

            newMaterial.shaderProps += sampleName + " = " + texName + "&cr;&lf;";

            if (sampleName == "txDiffuse") {
                newMaterial.txDiffuse = texName;
            } else if (sampleName == "txNormal") {
                newMaterial.txNormal = texName;
            } else if (sampleName == "txDetail") {
                newMaterial.txDetail = texName;
            } else {
                linfo() << "Unknown sample name: " << sampleName;
            }
            linfo() << "sampleSlot: " << sampleSlot;
        }

        newModel.materials[newModel.materials.size()] = std::move(newMaterial);
    }

    readNodes(*binStream, newModel.nodes, std::nullopt, newModel); //recursive

    return newModel;
}
