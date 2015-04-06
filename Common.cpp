#include "stdafx.h"
#include "Common.h"

const FbxDouble EPSILON = 0.0001;

bool AlmostEqual(const FbxDouble& d1, const FbxDouble& d2)
{
	return fabs(d1 - d2) < EPSILON;
}

bool AlmostEqual(const FbxDouble2& d1, const FbxDouble2& d2)
{
	return AlmostEqual(d1[0], d2[0]) && AlmostEqual(d1[1], d2[1]);
}

bool AlmostEqual(const FbxDouble3& d1, const FbxDouble3& d2)
{
	return AlmostEqual(d1[0], d2[0]) && AlmostEqual(d1[1], d2[1]) && AlmostEqual(d1[2], d2[2]);
}