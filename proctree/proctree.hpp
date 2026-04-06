// proctree.js Copyright (c) 2012, Paul Brunt
// c++ port Copyright (c) 2015, Jari Komppa
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of proctree.js nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL PAUL BRUNT BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#pragma once
#include <cstdint>

namespace Proctree
{
    typedef struct
    {
        float x, y, z;
    } fvec3;


    typedef struct
    {
        float u, v;
    } fvec2;


    typedef struct
    {
        int x, y, z;
    } ivec3;

    struct Properties
    {
        float mClumpMax = 0.454f;
        float mClumpMin = 0.404f;
        float mLengthFalloffFactor = 0.85f;
        float mLengthFalloffPower = 0.99f;
        float mBranchFactor = 2.45f;
        float mRadiusFalloffRate = 0.73f;
        float mClimbRate = 0.371f;
        float mTrunkKink = 0.093f;
        float mMaxRadius = 0.139f;
        int mTreeSteps = 5;
        float mTaperRate = 0.947f;
        float mTwistRate = 3.02f;
        int mSegments = 6;
        int mLevels = 5;
        float mSweepAmount = 0.01f;
        float mInitialBranchLength = 0.49f;
        float mTrunkLength = 2.4f;
        float mDropAmount = -0.1f;
        float mGrowAmount = 0.235f;
        float mVMultiplier = 0.36f;
        float mTwigScale = 0.39f;
        float random();
    };


    class Branch
    {
    public:
        Branch *mChild0;
        Branch *mChild1;
        Branch *mParent;
        fvec3 mHead;
        fvec3 mTangent;
        float mLength;
        int mTrunktype;
        int *mRing0, *mRing1, *mRing2;
        int *mRootRing;
        float mRadius;
        int mEnd;

        ~Branch();
        Branch();
        Branch(fvec3 aHead, Branch *aParent);
        void split(int aLevel, int aSteps, Properties &aProperties, int aL1 = 1, int aL2 = 1);
    };


    class Tree
    {
        Branch *mRoot;
        void init();
        void allocVertBuffers();
        void allocFaceBuffers();
        void calcVertSizes(Branch *aBranch);
        void calcFaceSizes(Branch *aBranch);
        void calcNormals();
        void doFaces(Branch *aBranch);
        void createTwigs(Branch *aBranch);
        void createForks(Branch *aBranch, float aRadius);
        void fixUVs();
    public:
        Properties mProperties;
        int mVertCount;
        int mTwigVertCount;
        int mFaceCount;
        int mTwigFaceCount;

        fvec3 *mVert;
        fvec3 *mNormal;
        fvec2 *mUV;
        fvec3 *mTwigVert;
        fvec3 *mTwigNormal;
        fvec2 *mTwigUV;
        ivec3 *mFace;
        ivec3 *mTwigFace;

        Tree();
        ~Tree();
        void generate();
    };


    fvec3 mirrorBranch(fvec3 aVec, fvec3 aNorm, Properties &aProperties);
    fvec3 axisAngle(fvec3 aVec, fvec3 aAxis, float aAngle);
    fvec3 scaleInDirection(fvec3 aVector, fvec3 aDirection, float aScale);
    fvec3 scaleVec(fvec3 a, float b);
    fvec3 add(fvec3 a, fvec3 b);
    fvec3 sub(fvec3 a, fvec3 b);
    float dot(fvec3 a, fvec3 b);
    fvec3 cross(fvec3 a, fvec3 b);
    fvec3 normalize(fvec3 a);
    float length(fvec3 a);
}
