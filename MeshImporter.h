#ifndef MESHIMPORTER_H
#define MESHIMPORTER_H

#include <fbxsdk.h>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include "Mesh.h"
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

	bool PrepareMeshes();
	bool GetFbxMeshes(FbxArray<FbxNode*> &fbxMeshes);
	void GetMeshNodes(FbxNode *node, FbxArray<FbxNode*> &fbxMeshes);
	void GetMaterials(const FbxArray<FbxNode*> &meshes, vector<Material> &materials, vector<int> &materialIndices);
	void GetMaterialIndices(FbxMesh *mesh, vector<int> &materialIndices);
	path GetTexturePath(FbxSurfaceMaterial *material, const char* propertyName);
};

struct VertexGroupInfo
{
	FbxVector4 normal;
	FbxVector2 uv;
	vector<int> triangleIndices;
};

FbxVector4 operator*(const FbxVector4 &vector, const FbxAMatrix &matrix);
FbxVector4& operator*=(FbxVector4 &vector, const FbxAMatrix &matrix);

#endif