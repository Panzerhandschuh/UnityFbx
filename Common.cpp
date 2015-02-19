#include "stdafx.h"
#include "Common.h"

const FbxDouble EPSILON = 0.0001;

int RoundToNearestPow2(int num)
{
	return (int)pow(2, ceil(log2((float)num)));
}

bool AlmostEqual(const FbxDouble& d1, const FbxDouble& d2)
{
	return fabs(d1 - d2) < EPSILON;
}

bool AlmostEqual(const FbxDouble2& d1, const FbxDouble2& d2)
{
	return AlmostEqual(d1[0], d2[0]) && AlmostEqual(d1[1], d2[1]);
}