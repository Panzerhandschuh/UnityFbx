#ifndef COMMON_H
#define COMMON_H

#include <fbxsdk.h>

using namespace fbxsdk_2015_1;

bool AlmostEqual(const FbxDouble& d1, const FbxDouble& d2);
bool AlmostEqual(const FbxDouble2& d1, const FbxDouble2& d2);
bool AlmostEqual(const FbxDouble3& d1, const FbxDouble3& d2);

#endif