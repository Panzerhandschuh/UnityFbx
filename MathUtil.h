#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <fbxsdk.h>

class MathUtil
{
public:
	static bool AlmostEqual(const FbxDouble& d1, const FbxDouble& d2);
	static bool AlmostEqual(const FbxDouble2& d1, const FbxDouble2& d2);
	static bool AlmostEqual(const FbxDouble3& d1, const FbxDouble3& d2);
};

#endif