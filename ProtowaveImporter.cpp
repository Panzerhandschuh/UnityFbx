#include "stdafx.h"
#include <fbxsdk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <exception>
#include <boost/filesystem/path.hpp>
#include "MathUtil.h"
#include "Mesh.h"
#include "MeshImporter.h"
#include "BinaryWriter.h"
#include "ImageInfo.h"

using namespace std;
using namespace boost::filesystem;
using namespace fbxsdk_2015_1;

const char *PWMDL_NAME = "PWMDL";
const char *PWCOL_NAME = "PWCOL";
const int PWMDL_VERSION = 1;
const int PWCOL_VERSION = 1;

void PrintCreatedFiles(const path &outputFile, const vector<string> &createdFiles, bool importMaterials = true);
void WriteMeshesToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals = true);
void WriteCollidersToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals = true);
void WriteMaterialsToFile(const path &outputFile, const path &materialsDir, const vector<Mesh> &meshes);
void CreateDDSFiles(const path &materialsDir, vector<Mesh> &meshes, vector<string> &createdFiles);
void CreateDDSFile(const path &materialsDir, path &texturePath, vector<string> &createdFiles);
ostream& operator<<(ostream& os, const FbxDouble2 &d2);
ostream& operator<<(ostream& os, const FbxDouble3 &d3);
ofstream& operator<<(ofstream& os, const FbxDouble2 &d2);
ofstream& operator<<(ofstream& os, const FbxDouble3 &d3);
void PrintColor(const FbxDouble3 &d3);
void WriteColor(ofstream& os, const FbxDouble3 &d3);
void PrintMaterial(const Material &mat);
void WriteMaterial(ofstream& os, const Material &mat, const string &pwmdlFileName);

int _tmain(int argc, _TCHAR* argv[])
{
	//argc = 2;
	//argv[1] = L"D:\\Users\\Tyler\\Documents\\Protowave\\Importer\\Pipe.FBX";

	//argv[2] = L"C:\\Users\\Tyler\\Documents\\Protowave\\Models\\Model.pwm";
	//argv[3] = L"-materials";
	//argv[4] = L"C:\\Users\\Tyler\\Documents\\Protowave\\Materials\\Model";

	if (argc < 2)
	{
		//cout << "pwmimport <fbx file> <pwm file> [materials dir] [/C] [/N] [/M]\n" << 
		//	"\tfbx file\tFbx file to import\n" <<
		//	"\tpwm file\tPwm file to create\n" <<
		//	"\tmaterials dir\tDirectory to output materials\n" <<
		//	"\t/C\t\tCombine meshes (animations will not work)\n" <<
		//	"\t/N\t\tDo not import normals\n" <<
		//	"\t/M\t\tDo not import materials (materials dir will be ignored)\n" <<
		//	"\tEx: pwmimport \"C:\\FbxFiles\\Cube.fbx\" \"C:\\Protowave\\Models\\MyModel.pwm\"" << 
		//	"\n\t\t\"C:\\Protowave\\Materials\\MyModel\"" << endl;
		cout << "pwimport <fbx file> [options]\n" <<
			"\t-o <directory>\tChange directory for converted mesh, collision, and material files\n" <<
			"\t-t <directory>\tChange directory for converted textures\n" <<
			"\t-c\t\tExport as a collision model\n" <<
			"\t-n\t\tDo not import normals\n" <<
			"\tEx: pwimport \"C:\\FbxFiles\\MyModel.fbx\" -o \"C:\\Protowave\\Models\"" <<
			"\n\t\t-t \"C:\\Protowave\\Materials\"" << endl;
		exit(EXIT_SUCCESS);
	}

	// Input file
	path inputFile = (argv[1]);
	if (!exists(inputFile))
	{
		cout << "Input file does not exist" << endl;
		exit(EXIT_FAILURE);
	}
	else if (!boost::iequals(inputFile.extension().string(), ".fbx"))
	{
		cout << "Error: Input file must be a .fbx file" << endl;
		exit(EXIT_FAILURE);
	}

	// Default values
	bool importNormals = true;
	bool isCollider = false;
	//bool importMaterials = true;
	path outputDir = inputFile.parent_path();
	if (outputDir.empty())
		outputDir = ".";
	path texturesDir = outputDir / inputFile.stem();

	// Check optional arguments
	for (int i = 2; i < argc; i++)
	{
		if (_tcscmp(argv[i], L"-o") == 0)
		{
			i++;
			if (i < argc)
				outputDir = argv[i];
			else
			{
				cout << "Error: -o missing argument" << endl;
				exit(EXIT_FAILURE);
			}
		}
		else if (_tcscmp(argv[i], L"-t") == 0)
		{
			i++;
			if (i < argc)
				texturesDir = argv[i];
			else
			{
				cout << "Error: -t missing argument" << endl;
				exit(EXIT_FAILURE);
			}
		}
		else if (_tcscmp(argv[i], L"-c") == 0)
			isCollider = true;
		else if (_tcscmp(argv[i], L"-n") == 0)
			importNormals = false;
		else
		{
			cout << "Error: Invalid argument (" << argv[i] << ")" << endl;
			exit(EXIT_FAILURE);
		}
	}

	// Check if directories exist
	try
	{
		if (!is_directory(outputDir))
			create_directories(outputDir);

		// Remove trailing slash from material dir
		string materialsString = texturesDir.string();
		if (materialsString[materialsString.length() - 1] == '\"')
			texturesDir = materialsString.substr(0, materialsString.length() - 1);

		if (!is_directory(texturesDir))
			create_directories(texturesDir);
	}
	catch (exception &e)
	{
		cout << e.what() << endl;
		exit(EXIT_FAILURE);
	}

	// Initialize importer
	MeshImporter importer(inputFile);

	// Import fbx file
	vector<Mesh> meshes;
	Mesh mesh;
	meshes.push_back(mesh);
	if (!importer.Import(meshes[0], !isCollider))
		exit(EXIT_FAILURE);

	// Write the imported mesh info to files
	if (!isCollider) // Standard model
	{
		// Create files
		vector<string> createdFiles;
		CreateDDSFiles(texturesDir, meshes, createdFiles);
		path pwmdlFile(inputFile.stem().string() + ".pwmdl");
		path outputPath(outputDir / pwmdlFile);
		WriteMaterialsToFile(outputPath, texturesDir, meshes);
		WriteMeshesToFile(outputPath, meshes, importNormals);

		// Print created file info to console
		PrintCreatedFiles(outputPath, createdFiles, true);
	}
	else // Collider mesh
	{
		// Create files
		path pwcolFile(inputFile.stem().string() + ".pwcol");
		path outputPath(outputDir / pwcolFile);
		WriteCollidersToFile(outputPath, meshes, importNormals);

		// Print created file info to console
		PrintCreatedFiles(outputPath, vector<string>(0), false);
	}

	// Remove import textures
	path fbmDir(inputFile.stem().string() + ".fbm");
	path fbmPath(outputDir / fbmDir);
	if (exists(fbmPath))
		remove_all(fbmPath);

	exit(EXIT_SUCCESS);
}


void PrintCreatedFiles(const path &outputFile, const vector<string> &createdFiles, bool importMaterials)
{
	cout << "Created Files:" << endl;
	cout << outputFile.string() << endl;

	if (importMaterials)
	{
		path pwmatFile(outputFile.stem().string() + ".pwmat");
		path outputPath = outputFile.parent_path() / pwmatFile;
		cout << outputPath.string() << endl;
		for (unsigned int i = 0; i < createdFiles.size(); i++)
			cout << createdFiles[i] << endl;
	}
}

void WriteMeshesToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals)
{
	try
	{
		BinaryWriter writer(outputFile.string());

		// Header
		writer.Write(PWMDL_NAME);
		writer.Write(PWMDL_VERSION);

		// Meshes
		int numMeshes = meshes.size();
		writer.Write(numMeshes);

		for (int i = 0; i < numMeshes; i++)
		{
			Mesh mesh = meshes[i];

			// Vertices
			int numVertices = mesh.vertices.size();
			writer.Write(numVertices);
			for (int j = 0; j < numVertices; j++)
				writer.Write(mesh.vertices[j]);

			// Triangles
			int numTriangles = mesh.triangles.size();
			writer.Write(numTriangles);
			for (int j = 0; j < numTriangles; j++)
				writer.Write(mesh.triangles[j]);

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

			// Material ids
			int numMaterialIds = mesh.materialIds.size();
			writer.Write(numMaterialIds);
			for (int j = 0; j < numMaterialIds; j++)
				writer.Write(mesh.materialIds[j]);

			// Print mesh info
			cout << "Mesh " << i << ":" << endl;
			cout << "Vertex Count: " << numVertices << endl;
			cout << "Triangle Index Count: " << numTriangles << endl;
			cout << "Normal Count: " << numNormals << endl;
			cout << "Uv Count: " << numUvs << endl;
			cout << "Material Id Count: " << numMaterialIds << endl;
			cout << "Material Count: " << mesh.materials.size() << endl;
			cout << endl;
		}
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}
}

void WriteCollidersToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals)
{
	try
	{
		BinaryWriter writer(outputFile.string());

		// Header
		writer.Write(PWCOL_NAME);
		writer.Write(PWCOL_VERSION);

		// Meshes
		int numMeshes = meshes.size();
		writer.Write(numMeshes);

		for (int i = 0; i < numMeshes; i++)
		{
			Mesh mesh = meshes[i];

			// Vertices
			int numVertices = mesh.vertices.size();
			writer.Write(numVertices);
			for (int j = 0; j < numVertices; j++)
				writer.Write(mesh.vertices[j]);

			// Triangles
			int numTriangles = mesh.triangles.size();
			writer.Write(numTriangles);
			for (int j = 0; j < numTriangles; j++)
				writer.Write(mesh.triangles[j]);

			// Normals
			int numNormals = 0;
			if (importNormals)
				numNormals = mesh.normals.size();
			writer.Write(numNormals);
			for (int j = 0; j < numNormals; j++)
				writer.Write(mesh.normals[j]);

			// Print mesh info
			cout << "Mesh " << i + 1 << ":" << endl;
			cout << "Vertex Count: " << numVertices << endl;
			cout << "Triangle Index Count: " << numTriangles << endl;
			cout << "Normal Count: " << numNormals << endl;
			cout << endl;
		}
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}
}

void WriteMaterialsToFile(const path &outputFile, const path &materialsDir, const vector<Mesh> &meshes)
{
	path pwmatFile(outputFile.stem().string() + ".pwmat");
	path outputPath = outputFile.parent_path() / pwmatFile;
	string pwmdlFileName = outputFile.stem().string();
	ofstream file;
	file.open(outputPath.string());

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		for (unsigned int j = 0; j < meshes[i].materials.size(); j++)
		{
			vector<Material> materials = meshes[i].materials;
			file << "Material Standard" << endl;
			file << "{" << endl;
			WriteMaterial(file, materials[j], pwmdlFileName);
			file << "}";
			if (j != materials.size() - 1) // Create line spacing if this is not the last material
				file << endl << endl;
		}
	}
	file.close();
}

void CreateDDSFiles(const path &materialsDir, vector<Mesh> &meshes, vector<string> &createdFiles)
{
	path texconvPath(initial_path() / "texconv.exe");
	if (exists(texconvPath))
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			vector<Material> materials = meshes[i].materials;
			for (unsigned int j = 0; j < materials.size(); j++)
			{
				CreateDDSFile(materialsDir, materials[j].diffuseMap, createdFiles);
				CreateDDSFile(materialsDir, materials[j].normalMap, createdFiles);
				CreateDDSFile(materialsDir, materials[j].specularMap, createdFiles);

				// Print material info
				cout << "Material " << j << ":" << endl;
				PrintMaterial(materials[j]);
				cout << endl;
			}
		}
	}
	else
		cout << "Warning: Missing texconv.exe, textures will not be saved." << endl;
}

void CreateDDSFile(const path &materialsDir, path &texturePath, vector<string> &createdFiles)
{
	if (texturePath.empty()) // Skip empty textures
	{
		texturePath = "";
		return;
	}

	try
	{
		ImageInfo imgInfo(texturePath);

		path ddsFile(texturePath.stem().string() + ".dds");
		path outputPath(materialsDir / ddsFile);

		string format;
		if (imgInfo.hasAlpha)
			format = "BC3_UNORM";
		else
			format = "BC1_UNORM";
		//cout << format << " " << img.width << " " << img.height << endl;

		// Execute texture conversion
		string imagePath = texturePath.string();
		replace(imagePath.begin(), imagePath.end(), '/', '\\');
		char command[512];
		sprintf(command, "texconv -nologo -hflip -vflip -pow2 -f %s -o \"%s\" \"%s\"", format.c_str(), materialsDir.string().c_str(), imagePath.c_str());
		//cout << command << endl;
		system(command);
		cout << endl;

		// Add newly created texture to created files
		createdFiles.push_back(outputPath.string());
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
		cerr << "DDS Error: Could not read source image info, DDS file will not be generated" << endl;
		texturePath = "";
	}
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

void PrintColor(const FbxDouble3 &d3)
{
	cout << "R:" << (int)(d3[0] * 255) << " G:" << (int)(d3[1] * 255) << " B:" << (int)(d3[2] * 255);
}

void WriteColor(ofstream& os, const FbxDouble3 &d3)
{
	os << (int)(d3[0] * 255) << " " << (int)(d3[1] * 255) << " " << (int)(d3[2] * 255);
}

void PrintMaterial(const Material &mat)
{
	// Color/opacity info
	cout << "Color: ";
	PrintColor(mat.color);
	cout << endl;
	if (mat.hasOpacity)
		cout << "Opacity: " << mat.opacity << endl;

	// Diffuse map info
	if (!mat.diffuseMap.empty())
		cout << "Diffuse Map: " << mat.diffuseMap.stem().string() << endl;

	// Normal map info
	if (!mat.normalMap.empty())
		cout << "Normal Map: " << mat.normalMap.stem().string() << endl;

	// Specular map info
	if (!mat.specularMap.empty())
		cout << "Specular Map: " << mat.specularMap.stem().string() << endl;

	// UV info
	if (!AlmostEqual(mat.uvScaling, FbxDouble2(1, 1)))
		cout << "Tiling: " << mat.uvScaling << endl;
	if (!AlmostEqual(mat.uvTranslation, FbxDouble2(0, 0)))
		cout << "Offset: " << mat.uvTranslation << endl;
}

void WriteMaterial(ofstream& os, const Material &mat, const string &pwmdlFileName)
{
	// Color/opacity info
	os << "\t" << "Color ";
	WriteColor(os, mat.color);
	os << endl;
	if (mat.hasOpacity)
		os << "\t" << "Opacity " << mat.opacity << endl;

	// Diffuse map info
	if (!mat.diffuseMap.empty())
	{
		string textureName = mat.diffuseMap.stem().string();
		os << "\t" << "DiffuseMap " << pwmdlFileName << "\\" << textureName << endl;
	}

	// Normal map info
	if (!mat.normalMap.empty())
	{
		string textureName = mat.normalMap.stem().string();
		os << "\t" << "NormalMap " << pwmdlFileName << "\\" << textureName << endl;
	}

	// Specular map info
	if (!mat.specularMap.empty())
	{
		string textureName = mat.specularMap.stem().string();
		os << "\t" << "SpecularMap " << pwmdlFileName << "\\" << textureName << endl;
	}

	// UV info
	if (!AlmostEqual(mat.uvScaling, FbxDouble2(1, 1)))
		os << "\t" << "Tiling " << mat.uvScaling << endl;
	if (!AlmostEqual(mat.uvTranslation, FbxDouble2(0, 0)))
		os << "\t" << "Offset " << mat.uvTranslation << endl;
}