#ifndef MESH_H
#define MESH_H

#include <vector>
#include "Material.h"

struct Vector3
{
	float x, y, z;
};

struct Vector2
{
	float x, y;
};

struct Mesh
{
	std::vector<Vector3> vertices;
	std::vector<int> triangles;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;
	std::vector<int> materialIds;
	std::vector<Material> materials;
};

#endif