#ifndef FBXUTIL_H
#define FBXUTIL_H

#include <fbxsdk.h>

class FbxUtil
{
public:
	static FbxAMatrix GetGeometry(FbxNode* node);
};

#endif