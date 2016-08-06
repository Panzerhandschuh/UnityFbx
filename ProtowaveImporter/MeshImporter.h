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
	MeshImporter(const boost::filesystem::path &inputFile);
	~MeshImporter();
	bool Import(Mesh &mesh, bool isCollider = false);

private:
	FbxScene *scene;
	FbxManager *manager;
	bool isCollider;

	//void ProcessNode(FbxNode *node, vector<Mesh> &meshes, int parentIndex);
	Mesh ProcessMesh(FbxMesh *fbxMesh, FbxArray<FbxNode*> fbxMeshes);
	void GetAllMeshes(FbxNode *node, FbxArray<FbxNode*> &fbxMeshes);
	std::vector<Material> GetMaterials(FbxNode *node);
	boost::filesystem::path GetTexturePath(FbxSurfaceMaterial *material, const char* propertyName);
};

struct VertexInfo
{
	FbxVector4 normal;
	FbxVector2 uv;
	std::vector<int> triangles;
};

FbxVector4 operator*(const FbxVector4 &vector, const FbxAMatrix &matrix);
FbxVector4& operator*=(FbxVector4 &vector, const FbxAMatrix &matrix);

#endif