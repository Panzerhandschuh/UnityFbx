#ifndef FBXUTIL_H
#define FBXUTIL_H

#include <fbxsdk.h>
#include <iostream>
#include <fstream>

class FbxUtil
{
public:
	static FbxAMatrix GetGlobalPosition(FbxNode* node, FbxPose* pose = NULL, FbxAMatrix* parentGlobalPosition = NULL);
	static FbxAMatrix GetPoseMatrix(FbxPose* pose, int nodeIndex);
	static FbxAMatrix GetGeometry(FbxNode* node);
};

FbxVector4 operator*(const FbxVector4 &vector, const FbxAMatrix &matrix);
FbxVector4& operator*=(FbxVector4 &vector, const FbxAMatrix &matrix);
std::ostream& operator<<(std::ostream& os, const FbxDouble2 &d2);
std::ostream& operator<<(std::ostream& os, const FbxDouble3 &d3);
std::ofstream& operator<<(std::ofstream& os, const FbxDouble2 &d2);
std::ofstream& operator<<(std::ofstream& os, const FbxDouble3 &d3);

#endif