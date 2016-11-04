#include "stdafx.h"
#include "MeshUtil.h"

using namespace std;
using namespace boost::filesystem;

const char *PWMDL_NAME = "PWMDL";
const char *PWCOL_NAME = "PWCOL";
const int PWMDL_VERSION = 1;
const int PWCOL_VERSION = 1;

void MeshUtil::WriteMeshToFile(const path &outputFile, const Mesh &mesh, bool importNormals)
{
	try
	{
		BinaryWriter writer(outputFile.string());

		// Header
		writer.Write(PWMDL_NAME);
		writer.Write(PWMDL_VERSION);

		//writer.Write(mesh.name);

		// Transform
		//writer.Write(mesh.parentIndex);
		//writer.Write(mesh.translation);
		//writer.Write(mesh.rotation);
		//writer.Write(mesh.scale);

		// Vertices
		int numVertices = mesh.vertices.size();
		writer.Write(numVertices);
		for (int j = 0; j < numVertices; j++)
			writer.Write(mesh.vertices[j]);

		// Normals
		int numNormals = 0;
		if (importNormals)
			numNormals = mesh.normals.size();
		writer.Write(numNormals);
		for (int j = 0; j < numNormals; j++)
			writer.Write(mesh.normals[j]);

		// Uvs
		int numUvs = mesh.uvs.size();
		writer.Write(numUvs);
		for (int j = 0; j < numUvs; j++)
			writer.Write(mesh.uvs[j]);

		// Sub mesh triangles
		int subMeshCount = mesh.subMeshTriangles.size();
		writer.Write(subMeshCount);
		for (int j = 0; j < subMeshCount; j++)
		{
			int numTriangles = mesh.subMeshTriangles[j].size();
			writer.Write(numTriangles);
			for (int k = 0; k < numTriangles; k++)
				writer.Write(mesh.subMeshTriangles[j][k]);
		}

		// Print mesh info
		cout << "Mesh Info:" << endl;
		cout << "Vertex Count: " << numVertices << endl;
		cout << "Normal Count: " << numNormals << endl;
		cout << "UV Count: " << numUvs << endl;
		cout << "Triangle Count: " << mesh.triangles.size() << endl;
		cout << "Material Count: " << mesh.materials.size() << endl;
		cout << endl;
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}
}

void MeshUtil::WriteCollisionsToFile(const path &outputFile, const Mesh &mesh, bool importNormals)
{
	try
	{
		BinaryWriter writer(outputFile.string());

		// Header
		writer.Write(PWCOL_NAME);
		writer.Write(PWCOL_VERSION);

		// Vertices
		int numVertices = mesh.vertices.size();
		writer.Write(numVertices);
		for (int j = 0; j < numVertices; j++)
			writer.Write(mesh.vertices[j]);

		// Normals
		int numNormals = 0;
		if (importNormals)
			numNormals = mesh.normals.size();
		writer.Write(numNormals);
		for (int j = 0; j < numNormals; j++)
			writer.Write(mesh.normals[j]);

		// Triangles
		int numTriangles = mesh.triangles.size();
		writer.Write(numTriangles);
		for (int j = 0; j < numTriangles; j++)
			writer.Write(mesh.triangles[j]);

		// Print mesh info
		cout << "Mesh Info:" << endl;
		cout << "Vertex Count: " << numVertices << endl;
		cout << "Normal Count: " << numNormals << endl;
		cout << "Triangle Count: " << numTriangles << endl;
		cout << endl;
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}
}
