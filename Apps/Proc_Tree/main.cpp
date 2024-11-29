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

#include "proctree.h"
#include <Mlib/Env.hpp>
#include <Mlib/Os/Os.hpp>
#include <chrono>
#include <print>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Mlib;
using namespace Proctree;

void benchmark()
{
    Proctree::Tree tree;
    printf("Start..\n");
    int n = 100000;
    int j;
    for (j = 0; j < 10; j++)
    {
        std::chrono::steady_clock::time_point start, end;
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

void draw_arrays(std::ostream& file, int vertCount, fvec3 * vert, fvec3 * normal, fvec2 * uv, int faceCount, ivec3 * face, int faceOffset, const fvec3& position, bool rotate90) {
    for (int i = 0; i < vertCount; i++) {
        std::print(file, "v %{:3f} %{:3f} %{:3f}\n",
            (rotate90 ? -vert[i].z : vert[i].x) + position.x,
            vert[i].y + position.y,
            (rotate90 ? vert[i].x : vert[i].z) + position.z);
    }
    for (int i = 0; i < vertCount; ++i) {
        std::print(file, "vn %{:3f} %{:3f} %{:3f}\n", normal[i].x, normal[i].y, normal[i].z);
    }
    for (int i = 0; i < vertCount; ++i) {
        std::print(file, "vt %{:3f} %{:3f}\n", uv[i].u, uv[i].v);
    }
    for (int i = 0; i < faceCount; ++i) {
        std::print(
            file,
            "f {:d}/{:d}/{:d} {:d}/{:d}/{:d} {:d}/{:d}/{:d}\n",
            face[i].x + 1 + faceOffset, face[i].x + 1 + faceOffset, face[i].x + 1 + faceOffset,
            face[i].y + 1 + faceOffset, face[i].y + 1 + faceOffset, face[i].y + 1 + faceOffset,
            face[i].z + 1 + faceOffset, face[i].z + 1 + faceOffset, face[i].z + 1 + faceOffset);
    }
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

    tree.mProperties.mSegments = getenv_default_int("mSegments", 6);
    tree.mProperties.mLevels = getenv_default_int("mLevels", 5);
    tree.mProperties.mVMultiplier = getenv_default_float("mVMultiplier", 0.36f);
    tree.mProperties.mTwigScale = getenv_default_float("mTwigScale", 0.39f);
    tree.mProperties.mInitialBranchLength = getenv_default_float("mInitialBranchLength", 0.49f);
    tree.mProperties.mLengthFalloffFactor = getenv_default_float("mLengthFalloffFactor", 0.85f);
    tree.mProperties.mLengthFalloffPower = getenv_default_float("mLengthFalloffPower", 0.99f);
    tree.mProperties.mClumpMax = getenv_default_float("mClumpMax", 0.454f);
    tree.mProperties.mClumpMin = getenv_default_float("mClumpMin", 0.404f);
    tree.mProperties.mBranchFactor = getenv_default_float("mBranchFactor", 2.45f);
    tree.mProperties.mDropAmount = getenv_default_float("mDropAmount", -0.1f);
    tree.mProperties.mGrowAmount = getenv_default_float("mGrowAmount", 0.235f);
    tree.mProperties.mSweepAmount = getenv_default_float("mSweepAmount", 0.01f);
    tree.mProperties.mMaxRadius = getenv_default_float("mMaxRadius", 0.139f);
    tree.mProperties.mClimbRate = getenv_default_float("mClimbRate", 0.371f);
    tree.mProperties.mTrunkKink = getenv_default_float("mTrunkKink", 0.093f);
    tree.mProperties.mTreeSteps = getenv_default_int("mTreeSteps", 5);
    tree.mProperties.mTaperRate = getenv_default_float("mTaperRate", 0.947f);
    tree.mProperties.mRadiusFalloffRate = getenv_default_float("mRadiusFalloffRate", 0.73f);
    tree.mProperties.mTwistRate = getenv_default_float("mTwistRate", 3.02f);
    tree.mProperties.mTrunkLength = getenv_default_float("mTrunkLength", 2.4f);
    int ntrees = getenv_default_int("ntrees", 1);
    float tree_distance = getenv_default_float("tree_distance", 0.2f);
    std::string trunk_diffuse = getenv_default("trunk_diffuse", "bark.jpg");
    std::string twig_diffuse = getenv_default("twig_diffuse", "twig.jpg");
    std::string trunk_normal = getenv_default("trunk_normal", "");
    std::string twig_normal = getenv_default("twig_normal", "");

    // 3) Call generate
    // tree.generate();

    // 4) Use the data

    {
        auto file = create_ofstream("tree.obj");
        if (file->fail()) {
            verbose_abort("Could not open tree.obj for write");
        }
        *file << "mtllib tree.mtl\n";
        int faceOffset = 0;
        fvec3 position{float(-ntrees) * tree_distance / 2.f, 0.f, 0.f};
        int seed0 = getenv_default_int("mSeed", 1);
        if (seed0 == 0) {
            verbose_abort("mSeed=0 not allowed");
        }
        for (int i = 0; i < ntrees; ++i) {
            srand((unsigned int)(seed0 + i));
            tree.generate();
            std::print(*file, "g Tree{:d}\n", i);
            std::print(*file, "o Tree{:d}\n", i);
            std::print(*file, "usemtl tree\n");
            draw_arrays(*file, tree.mVertCount, tree.mVert, tree.mNormal, tree.mUV, tree.mFaceCount, tree.mFace, faceOffset, position, i % 2);
            faceOffset += tree.mVertCount;
            std::print(*file, "\n");

            std::print(*file, "g Twig{:d}\n", i);
            std::print(*file, "o Twig{:d}\n", i);
            std::print(*file, "usemtl twig\n");
            draw_arrays(*file, tree.mTwigVertCount, tree.mTwigVert, tree.mTwigNormal, tree.mTwigUV, tree.mTwigFaceCount, tree.mTwigFace, faceOffset, position, i % 2);
            faceOffset += tree.mTwigVertCount;

            position.x += tree_distance;
        }

        // 5) Profit.

        // Note: You can change the properties and call generate to change the data,
        // no need to delete the tree object in between.

        file->flush();
        if (file->fail()) {
            verbose_abort("Could not write to file tree.obj");
        }
    }
    {
        auto file = create_ofstream("tree.mtl");
        if (file->fail()) {
            verbose_abort("Could not open tree.mtl for write");
        }
        std::print(*file, "newmtl tree\n");
        std::print(*file, "map_Kd {:s}\n", trunk_diffuse);
        if (!trunk_normal.empty()) std::print(*file, "map_Bump {:s}\n", trunk_normal);
        std::print(*file, "\n");
        std::print(*file, "newmtl twig\n");
        std::print(*file, "map_Kd {:s}\n", twig_diffuse);
        std::print(*file, "map_d {:s}\n", twig_diffuse);
        if (!twig_normal.empty()) std::print(*file, "map_Bump {:s}\n", twig_normal);

        file->flush();
        if (file->fail()) {
            verbose_abort("Could not write to file tree.mtl");
        }
    }
}


int main(int argc, char **argv)
{
    //benchmark();
    basic_use();

}