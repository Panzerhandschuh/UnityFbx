#ifndef MESH_H
#define MESH_H

#include <vector>
#include "Material.h"

struct Vector2
{
	float x, y;

	Vector2() { }

	Vector2(const FbxVector2 &vec)
	{
		x = (float)vec[0];
		y = (float)vec[1];
	}

	Vector2& operator=(const FbxVector2 &vec)
	{
		x = (float)vec[0];
		y = (float)vec[1];
		return *this;
	}
};

struct Vector3
{
	float x, y, z;

	Vector3() { }

	Vector3(const FbxVector4 &vec)
	{
		x = (float)vec[0];
		y = (float)vec[1];
		z = (float)vec[2];
	}

	Vector3& operator=(const FbxVector4 &vec)
	{
		x = (float)vec[0];
		y = (float)vec[1];
		z = (float)vec[2];
		return *this;
	}
};

struct Mesh
{
	//string name;
	//int parentIndex;
	//Vector3 translation;
	//Vector3 rotation;
	//Vector3 scale;
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;
	std::vector<int> triangles;
	std::vector<std::vector<int>> subMeshTriangles;
	std::vector<Material> materials;
};

#endif