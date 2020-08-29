/*
    proctree c++ port test / example
    Copyright (c) 2015 Jari Komppa

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgement in the product documentation would be
    appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

    NOTE: this license covers this example only, proctree.cpp has different license
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <stdexcept>
#include <string.h>
#include "proctree.h"
#include <Mlib/String.hpp>

using namespace Proctree;

void benchmark()
{
    Proctree::Tree tree;
    printf("Start..\n");
    int n = 100000;
    int j;
    for (j = 0; j < 10; j++)
    {
        std::chrono::time_point<std::chrono::steady_clock> start, end;
        start = std::chrono::steady_clock::now();
        int i;
        for (i = 0; i < n; i++)
        {
            tree.generate();
        }
        end = std::chrono::steady_clock::now();
        std::chrono::duration<double> sec = end - start;
        printf("%3.3fs (%3.3f trees per second)\n", sec.count(), n / sec.count());
    }
}

void draw_arrays(FILE * pFile, int vertCount, fvec3 * vert, fvec3 * normal, fvec2 * uv, int faceCount, ivec3 * face, int faceOffset) {
    for (int i = 0; i < vertCount; i++) {
        int res = fprintf(pFile, "v %+3.3f %+3.3f %+3.3f\n", vert[i].x, vert[i].y, vert[i].z);
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
    for(int i = 0; i < vertCount; ++i) {
        int res = fprintf(pFile, "vn %+3.3f %+3.3f %+3.3f\n", normal[i].x, normal[i].y, normal[i].z);
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
    for(int i = 0; i < vertCount; ++i) {
        int res = fprintf(pFile, "vt %+3.3f %+3.3f\n", uv[i].u, uv[i].v);
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
    for(int i = 0; i < faceCount; ++i) {
        int res = fprintf(
			pFile,
			"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			face[i].x + 1 + faceOffset, face[i].x + 1 + faceOffset, face[i].x + 1 + faceOffset,
			face[i].y + 1 + faceOffset, face[i].y + 1 + faceOffset, face[i].y + 1 + faceOffset,
			face[i].z + 1 + faceOffset, face[i].z + 1 + faceOffset, face[i].z + 1 + faceOffset);
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
}

float getenvf(const char* n, float deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stof(v);
}

int getenvi(const char* n, int deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stoi(v);
}

void basic_use()
{
    // 1) Create the tree object

    Proctree::Tree tree;

    // 2) Change properties here for different kinds of trees
    /*
    tree.mProperties.mSeed = 7;
    tree.mProperties.mTreeSteps = 7;
    tree.mProperties.mLevels = 3;
    // etc.
    */

    tree.mProperties.mSeed = getenvi("mSeed", 262);
    tree.mProperties.mSegments = getenvi("mSegments", 6);
    tree.mProperties.mLevels = getenvi("mLevels", 5);
    tree.mProperties.mVMultiplier = getenvf("mVMultiplier", 0.36f);
    tree.mProperties.mTwigScale = getenvf("mTwigScale", 0.39f);
    tree.mProperties.mInitialBranchLength = getenvf("mInitialBranchLength", 0.49f);
    tree.mProperties.mLengthFalloffFactor = getenvf("mLengthFalloffFactor", 0.85f);
    tree.mProperties.mLengthFalloffPower = getenvf("mLengthFalloffPower", 0.99f);
    tree.mProperties.mClumpMax = getenvf("mClumpMax", 0.454f);
    tree.mProperties.mClumpMin = getenvf("mClumpMin", 0.404f);
    tree.mProperties.mBranchFactor = getenvf("mBranchFactor", 2.45f);
    tree.mProperties.mDropAmount = getenvf("mDropAmount", -0.1f);
    tree.mProperties.mGrowAmount = getenvf("mGrowAmount", 0.235f);
    tree.mProperties.mSweepAmount = getenvf("mSweepAmount", 0.01f);
    tree.mProperties.mMaxRadius = getenvf("mMaxRadius", 0.139f);
    tree.mProperties.mClimbRate = getenvf("mClimbRate", 0.371f);
    tree.mProperties.mTrunkKink = getenvf("mTrunkKink", 0.093f);
    tree.mProperties.mTreeSteps = getenvi("mTreeSteps", 5);
    tree.mProperties.mTaperRate = getenvf("mTaperRate", 0.947f);
    tree.mProperties.mRadiusFalloffRate = getenvf("mRadiusFalloffRate", 0.73f);
    tree.mProperties.mTwistRate = getenvf("mTwistRate", 3.02f);
    tree.mProperties.mTrunkLength = getenvf("mTrunkLength", 2.4f);

    // 3) Call generate
    tree.generate();

    // 4) Use the data

    {
        FILE * pFile = fopen("tree.obj", "w");
        if (pFile == nullptr) {
            throw std::runtime_error("Could not open tree.obj for write");
        }
        fprintf(pFile, "mtllib tree.mtl\n");
        fprintf(pFile, "g Tree\n");
        fprintf(pFile, "o Tree\n");
        fprintf(pFile, "usemtl tree\n");
        draw_arrays(pFile, tree.mVertCount, tree.mVert, tree.mNormal, tree.mUV, tree.mFaceCount, tree.mFace, 0);
        fprintf(pFile, "\n");

        fprintf(pFile, "g Twig\n");
        fprintf(pFile, "o Twig\n");
        fprintf(pFile, "usemtl twig\n");
        draw_arrays(pFile, tree.mTwigVertCount, tree.mTwigVert, tree.mTwigNormal, tree.mTwigUV, tree.mTwigFaceCount, tree.mTwigFace, tree.mVertCount);

        // 5) Profit.

        // Note: You can change the properties and call generate to change the data,
        // no need to delete the tree object in between.

        if (fclose(pFile) < 0) {
            throw std::runtime_error("Could not close file");
        }
    }
    {
        FILE * pFile = fopen("tree.mtl", "w");
        if (pFile == nullptr) {
            throw std::runtime_error("Could not open tree.mtl for write");
        }
        fprintf(pFile, "newmtl tree\n");
        fprintf(pFile, "map_Kd bark.jpg\n");
        fprintf(pFile, "\n");
        fprintf(pFile, "newmtl twig\n");
        fprintf(pFile, "map_Kd twig.png\n");
        fprintf(pFile, "map_d twig.png\n");

        if (fclose(pFile) < 0) {
            throw std::runtime_error("Could not close file");
        }
    }
}


int main(int argc, char **argv)
{
    //benchmark();
    basic_use();

}