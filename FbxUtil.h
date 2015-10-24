#ifndef FBXUTIL_H
#define FBXUTIL_H

#include <fbxsdk.h>

using namespace fbxsdk_2015_1;

class FbxUtil
{
public:
	static FbxAMatrix GetGeometry(FbxNode* node);
};

#endif