#include "stdafx.h"
#include "MathUtil.h"

using namespace fbxsdk_2015_1;

const FbxDouble EPSILON = 0.0001;

bool MathUtil::AlmostEqual(const FbxDouble& d1, const FbxDouble& d2)
{
	return fabs(d1 - d2) < EPSILON;
}

bool MathUtil::AlmostEqual(const FbxDouble2& d1, const FbxDouble2& d2)
{
	return AlmostEqual(d1[0], d2[0]) && AlmostEqual(d1[1], d2[1]);
}

bool MathUtil::AlmostEqual(const FbxDouble3& d1, const FbxDouble3& d2)
{
	return AlmostEqual(d1[0], d2[0]) && AlmostEqual(d1[1], d2[1]) && AlmostEqual(d1[2], d2[2]);
}