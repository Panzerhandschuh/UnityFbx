#ifndef MESHIMPORTER_H
#define MESHIMPORTER_H

#include <fbxsdk.h>
#include <iostream>
#include <vector>
#include <exception>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "Mesh.h"
#include "MathUtil.h"
#include "FbxUtil.h"

class MeshImporter
{
public:
	MeshImporter(const path &inputFile);
	~MeshImporter();
	bool Import(Mesh &mesh, bool importMaterials = true);

private:
	path fbxFile;
	FbxScene *scene;
	FbxManager *manager;

	void GetAllMeshes(FbxNode *node, FbxArray<FbxNode*> &fbxMeshes);
	void GetMaterials(FbxNode *node, vector<Material> &materials);
	void GetMaterialIndices(FbxMesh *mesh, vector<int> &materialIndices);
	path GetTexturePath(FbxSurfaceMaterial *material, const char* propertyName);
};

struct VertexInfo
{
	FbxVector4 normal;
	FbxVector2 uv;
	vector<int> triangles;
};

FbxVector4 operator*(const FbxVector4 &vector, const FbxAMatrix &matrix);
FbxVector4& operator*=(FbxVector4 &vector, const FbxAMatrix &matrix);

#endif