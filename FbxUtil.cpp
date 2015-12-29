#include "stdafx.h"
#include "FbxUtil.h"

using namespace fbxsdk_2015_1;

FbxAMatrix FbxUtil::GetGeometry(FbxNode* pNode)
{
	const FbxVector4 translate = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 rotate = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 scale = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(translate, rotate, scale);
}