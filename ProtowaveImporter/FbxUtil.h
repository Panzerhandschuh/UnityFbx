#ifndef FBXUTIL_H
#define FBXUTIL_H

#include <fbxsdk.h>

class FbxUtil
{
public:
	static FbxAMatrix GetGlobalPosition(FbxNode* node, FbxPose* pose = NULL, FbxAMatrix* parentGlobalPosition = NULL);
	static FbxAMatrix GetPoseMatrix(FbxPose* pose, int nodeIndex);
	static FbxAMatrix GetGeometry(FbxNode* node);
};

#endif