#include "stdafx.h"
#include "FbxUtil.h"

using namespace std;

FbxAMatrix FbxUtil::GetGlobalPosition(FbxNode* node, FbxPose* pose, FbxAMatrix* parentGlobalPosition)
{
	FbxAMatrix globalPosition;
	bool positionFound = false;

	if (pose)
	{
		int nodeIndex = pose->Find(node);

		if (nodeIndex > -1)
		{
			// The bind pose is always a global matrix. If we have a rest pose, we need to check if it is stored in global or local space.
			if (pose->IsBindPose() || !pose->IsLocalMatrix(nodeIndex))
				globalPosition = GetPoseMatrix(pose, nodeIndex);
			else
			{
				// We have a local matrix, we need to convert it to a global space matrix
				FbxAMatrix parentMatrix;

				if (parentGlobalPosition)
					parentMatrix = *parentGlobalPosition;
				else
				{
					if (node->GetParent())
						parentMatrix = GetGlobalPosition(node->GetParent(), pose);
				}

				FbxAMatrix localPosition = GetPoseMatrix(pose, nodeIndex);
				globalPosition = parentMatrix * localPosition;
			}

			positionFound = true;
		}
	}

	if (!positionFound)
	{
		// There is no pose entry for that node, get the current global position instead
		globalPosition = node->EvaluateGlobalTransform();
	}

	return globalPosition;
}

// Get the matrix of the given pose
FbxAMatrix FbxUtil::GetPoseMatrix(FbxPose* pose, int nodeIndex)
{
	FbxAMatrix poseMatrix;
	FbxMatrix matrix = pose->GetMatrix(nodeIndex);

	memcpy((double*)poseMatrix, (double*)matrix, sizeof(matrix.mData));

	return poseMatrix;
}

FbxAMatrix FbxUtil::GetGeometry(FbxNode* node)
{
	const FbxVector4 translate = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 rotate = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 scale = node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(translate, rotate, scale);
}

FbxVector4 operator*(const FbxVector4 &vector, const FbxAMatrix &matrix)
{
	FbxVector4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i] += matrix[j][i] * vector[j];
		}
	}
	return result;
}

FbxVector4& operator*=(FbxVector4 &vector, const FbxAMatrix &matrix)
{
	FbxVector4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i] += matrix[j][i] * vector[j];
		}
	}
	vector = result;
	return vector;
}

ostream& operator<<(ostream& os, const FbxDouble2 &d2)
{
	os << d2[0] << " " << d2[1];
	return os;
}

ostream& operator<<(ostream& os, const FbxDouble3 &d3)
{
	os << d3[0] << " " << d3[1] << " " << d3[2];
	return os;
}

ofstream& operator<<(ofstream& os, const FbxDouble2 &d2)
{
	os << d2[0] << " " << d2[1];
	return os;
}

ofstream& operator<<(ofstream& os, const FbxDouble3 &d3)
{
	os << d3[0] << " " << d3[1] << " " << d3[2];
	return os;
}