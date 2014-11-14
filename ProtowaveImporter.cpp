#include "stdafx.h"
#include <fbxsdk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>
#include "Mesh.h"
#include "MeshImporter.h"
#include "Serializer.h"
#include "ImageInfo.h"

using namespace std;
using namespace boost::filesystem;
using namespace fbxsdk_2015_1;

const char *PWMDL_NAME = "PWMDL";
const char *PWCOL_NAME = "PWCOL";
const int PWMDL_VERSION = 1;
const int PWCOL_VERSION = 1;

void PrintCreatedFiles(const path &outputFile, const path &texturesDir, vector<Mesh> &meshes, bool importMaterials = true);
void WriteMeshesToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals = true);
void WriteCollidersToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals = true);
void WriteMaterialsToFile(const path &outputFile, const path &materialsDir, const vector<Mesh> &meshes);
void CreateTextureFiles(const path &materialsDir, vector<Mesh> &meshes);
int RoundToNearestPow2(int num);
void EndApp(int success);

int _tmain(int argc, _TCHAR* argv[])
{
	//argc = 5;
	//argv[1] = L"C:\\Users\\Tyler\\Desktop\\Mat3.fbx";
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
			"\t-outdir <directory>\tChange directory for converted file\n" <<
			"\t-texdir <directory>\tChange directory for output textures\n" <<
			"\t-collider\t\tExport as a collision model\n" <<
			"\t-nonormals\t\tDo not import normals\n" <<
			"\tEx: pwimport \"C:\\FbxFiles\\MyModel.fbx\" -outdir \"C:\\Protowave\\Models\"" <<
			"\n\t\t-texdir \"C:\\Protowave\\Materials\\MyModel\"" << endl;
		EndApp(EXIT_SUCCESS);
	}

	// Input file
	path inputFile(argv[1]);

	// Default values
	bool importNormals = true;
	bool isCollider = false;
	//bool importMaterials = true;
	path outputDir = inputFile.parent_path();
	path texturesDir = outputDir.string() + "\\" + inputFile.stem().string();

	// Check optional arguments
	for (int i = 2; i < argc; i++)
	{
		if (_tcscmp(argv[i], L"-outdir") == 0)
		{
			i++;
			if (i < argc)
				outputDir = argv[i];
			else
			{
				cout << "Error: -outdir missing argument" << endl;
				EndApp(EXIT_FAILURE);
			}
		}
		else if (_tcscmp(argv[i], L"-texdir") == 0)
		{
			i++;
			if (i < argc)
				texturesDir = argv[i];
			else
			{
				cout << "Error: -texdir missing argument" << endl;
				EndApp(EXIT_FAILURE);
			}
		}
		else if (_tcscmp(argv[i], L"-collider") == 0)
		{
			isCollider = true;
		}
		else if (_tcscmp(argv[i], L"-nonormals") == 0)
		{
			importNormals = false;
		}
		else
		{
			cout << "Error: Invalid argument (" << argv[i] << ")" << endl;
			EndApp(EXIT_FAILURE);
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
		EndApp(EXIT_FAILURE);
	}

	if (!exists(inputFile))
		cout << "Input file does not exist" << endl;
	else if (!boost::iequals(inputFile.extension().string(), ".fbx"))
		cout << "Error: Input file must be a .fbx file" << endl;
	else
	{
		vector<Mesh> meshes;
		Mesh mesh;
		meshes.push_back(mesh);
		MeshImporter importer(inputFile);

		// Import fbx
		if (!importer.Import(meshes[0], !isCollider))
			EndApp(EXIT_FAILURE);

		if (!isCollider) // Standard model
		{
			// Create files
			path outputFile(outputDir.string() + "\\" + inputFile.stem().string() + ".pwmdl");
			CreateTextureFiles(texturesDir, meshes);
			WriteMaterialsToFile(outputFile, texturesDir, meshes);
			WriteMeshesToFile(outputFile, meshes, importNormals);

			// Print created file info to console
			PrintCreatedFiles(outputFile, texturesDir, meshes, true);
		}
		else // Collider mesh
		{
			// Create files
			path outputFile(outputDir.string() + "\\" + inputFile.stem().string() + ".pwcol");
			WriteCollidersToFile(outputFile, meshes, importNormals);

			// Print created file info to console
			PrintCreatedFiles(outputFile, texturesDir, meshes, false);
		}
	}

	EndApp(EXIT_SUCCESS);
}


void PrintCreatedFiles(const path &outputFile, const path &texturesDir, vector<Mesh> &meshes, bool importMaterials)
{
	cout << "Created Files:" << endl;
	cout << outputFile.string() << endl;

	if (!importMaterials)
		return;

	string pwmatFile = outputFile.parent_path().string() + "\\" + outputFile.stem().string() + ".pwmat";
	cout << pwmatFile << endl;
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		for (unsigned int j = 0; j < meshes[i].materials.size(); j++)
		{
			vector<Material> materials = meshes[i].materials;
			if (ImageInfo::IsValidImage(materials[j].baseTexture))
				cout << texturesDir.string() << "\\" << materials[j].baseTexture.stem().string() << ".dds" << endl;
			if (ImageInfo::IsValidImage(materials[j].bumpMap))
				cout << texturesDir.string() << "\\" << materials[j].bumpMap.stem().string() << ".dds" << endl;
		}
	}
}

void WriteMeshesToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals)
{
	ofstream file;
	file.open(outputFile.string(), ios::out | ios::binary | ios::trunc);
	if (!file)
	{
		cout << "Error: Could not open output file. Another process may be using it." << endl;
		return;
	}

	// Header
	Serializer::SerializeMagicNumber(file, PWMDL_NAME);
	Serializer::Serialize(file, PWMDL_VERSION);

	// Meshes
	int numMeshes = meshes.size();
	Serializer::Serialize(file, numMeshes);

	for (int i = 0; i < numMeshes; i++)
	{
		Mesh mesh = meshes[i];

		// Vertices
		int numVertices = mesh.vertices.size();
		Serializer::Serialize(file, numVertices);
		for (int j = 0; j < numVertices; j++)
			Serializer::Serialize(file, mesh.vertices[j]);

		// Triangles
		int numTriangles = mesh.triangles.size();
		Serializer::Serialize(file, numTriangles);
		for (int j = 0; j < numTriangles; j++)
			Serializer::Serialize(file, mesh.triangles[j]);

		// Normals
		int numNormals = 0;
		if (importNormals)
			numNormals = mesh.normals.size();
		Serializer::Serialize(file, numNormals);
		for (int j = 0; j < numNormals; j++)
			Serializer::Serialize(file, mesh.normals[j]);

		// Uvs
		int numUvs = mesh.uvs.size();
		Serializer::Serialize(file, numUvs);
		for (int j = 0; j < numUvs; j++)
			Serializer::Serialize(file, mesh.uvs[j]);

		// Material ids
		int numMaterialIds = mesh.materialIds.size();
		Serializer::Serialize(file, numMaterialIds);
		for (int j = 0; j < numMaterialIds; j++)
			Serializer::Serialize(file, mesh.materialIds[j]);

		// Print mesh info
		cout << "Mesh " << i + 1 << ":" << endl;
		cout << "Vertex Count: " << numVertices << endl;
		cout << "Triangle Count: " << numTriangles << endl;
		cout << "Normal Count: " << numNormals << endl;
		cout << "Uv Count: " << numUvs << endl;
		cout << "Material Id Count: " << numMaterialIds << endl;
		cout << "Material Count: " << mesh.materials.size() << endl;
		cout << endl;
	}
}

void WriteCollidersToFile(const path &outputFile, vector<Mesh> &meshes, bool importNormals)
{
	ofstream file;
	file.open(outputFile.string(), ios::out | ios::binary | ios::trunc);
	if (!file)
	{
		cout << "Error: Could not open output file. Another process may be using it." << endl;
		return;
	}

	// Header
	Serializer::SerializeMagicNumber(file, PWCOL_NAME);
	Serializer::Serialize(file, PWCOL_VERSION);

	// Meshes
	int numMeshes = meshes.size();
	Serializer::Serialize(file, numMeshes);

	for (int i = 0; i < numMeshes; i++)
	{
		Mesh mesh = meshes[i];

		// Vertices
		int numVertices = mesh.vertices.size();
		Serializer::Serialize(file, numVertices);
		for (int j = 0; j < numVertices; j++)
			Serializer::Serialize(file, mesh.vertices[j]);

		// Triangles
		int numTriangles = mesh.triangles.size();
		Serializer::Serialize(file, numTriangles);
		for (int j = 0; j < numTriangles; j++)
			Serializer::Serialize(file, mesh.triangles[j]);

		// Normals
		int numNormals = 0;
		if (importNormals)
			numNormals = mesh.normals.size();
		Serializer::Serialize(file, numNormals);
		for (int j = 0; j < numNormals; j++)
			Serializer::Serialize(file, mesh.normals[j]);

		// Print mesh info
		cout << "Mesh " << i + 1 << ":" << endl;
		cout << "Vertex Count: " << numVertices << endl;
		cout << "Triangle Count: " << numTriangles << endl;
		cout << "Normal Count: " << numNormals << endl;
		cout << endl;
	}
}

void WriteMaterialsToFile(const path &outputFile, const path &materialsDir, const vector<Mesh> &meshes)
{
	string pwmatFile = outputFile.parent_path().string() + "\\" + outputFile.stem().string() + ".pwmat";
	string pwmdlFileName = outputFile.stem().string();
	ofstream file;
	file.open(pwmatFile);

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		for (unsigned int j = 0; j < meshes[i].materials.size(); j++)
		{
			vector<Material> materials = meshes[i].materials;
			file << "material" << j << endl;
			file << "{" << endl;
			if (!materials[j].baseTexture.empty())
			{
				string textureName = materials[j].baseTexture.stem().string();
				file << "\t" << "basetexture " << pwmdlFileName << "\\" << textureName << endl;
			}
			if (!materials[j].bumpMap.empty())
			{
				string textureName = materials[j].bumpMap.stem().string();
				file << "\t" << "bumpmap " << pwmdlFileName << "\\" << textureName << endl;
			}
			file << "\t" << "diffuse " << (int)(materials[j].diffuse[0] * 255) << " " <<
				(int)(materials[j].diffuse[1] * 255) << " " << (int)(materials[j].diffuse[2] * 255) << endl;
			if (materials[j].hasSpecular)
				file << "\t" << "specular " << (int)(materials[j].specular[0] * 255) << " " <<
				(int)(materials[j].specular[1] * 255) << " " << (int)(materials[j].specular[2] * 255) << " " <<
				materials[j].shininess << endl;
			if (materials[j].uvScaling != FbxDouble3(1, 1, 1))
				file << "\t" << "tiling " << materials[j].uvScaling[0] << " " << materials[j].uvScaling[1] << endl;
			if (materials[j].uvTranslation != FbxDouble3(0, 0, 0))
				file << "\t" << "offset " << materials[j].uvTranslation[0] << " " << materials[j].uvTranslation[1] << endl;
			file << "}";
			if (j != materials.size() - 1)
				file << endl << endl;
		}
	}
	file.close();
}

void CreateTextureFiles(const path &materialsDir, vector<Mesh> &meshes)
{
	string texconvPath = initial_path().string() + "\\texconv.exe";
	if (!exists(texconvPath))
	{
		cout << "Missing texconv.exe, textures will not be saved." << endl;
		return;
	}

	// Create DDS files
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		for (unsigned int j = 0; j < meshes[i].materials.size(); j++)
		{
			vector<Material> materials = meshes[i].materials;
			if (!ImageInfo::IsValidImage(materials[j].baseTexture))
			{
				materials[j].baseTexture = "";
				continue;
			}

			string newTexturePath = materialsDir.string() + "\\" + materials[j].baseTexture.filename().string();
			if (boost::iequals(materials[j].baseTexture.extension().string(), ".dds")) // Copy the source file
			{
				try
				{
					copy_file(materials[j].baseTexture, newTexturePath, copy_option::overwrite_if_exists);
				}
				catch (exception &e)
				{
					cout << e.what() << endl;
					materials[j].baseTexture = "";
					continue;
				}
				continue;
			}

			ImageInfo::ImageData img;
			if (!ImageInfo::GetImageInfo(materials[j].baseTexture, img))
			{
				cout << "DDS Error: Could not read source image info, DDS file will not be generated" << endl;
				materials[j].baseTexture = "";
				continue;
			}
			img.width = RoundToNearestPow2(img.width);
			img.height = RoundToNearestPow2(img.height);
			//int smaller = min(img.width, img.height);
			//int numMipMaps = (log(smaller) / log(2)) + 1;

			string format;
			if (img.hasAlpha)
				format = "DXT5";
			else
				format = "DXT1";
			//cout << format << " " << img.width << " " << img.height << endl;
			string imagePath = materials[j].baseTexture.string();
			replace(imagePath.begin(), imagePath.end(), '/', '\\');
			int size = snprintf(NULL, 0, "texconv -nologo -w %d -h %d -f %s -o \"%s\" \"%s\"", img.width, img.height, format.c_str(), materialsDir.string().c_str(), imagePath.c_str());
			char *command = new char[size + 1];
			sprintf(command, "texconv -nologo -w %d -h %d -f %s -o \"%s\" \"%s\"", img.width, img.height, format.c_str(), materialsDir.string().c_str(), imagePath.c_str());
			//cout << command << endl;
			system(command);
			delete[] command;
			cout << endl;
		}
	}

	// Print material info
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		for (unsigned int j = 0; j < meshes[i].materials.size(); j++)
		{
			vector<Material> materials = meshes[i].materials;
			cout << "Material " << j << ":" << endl;
			cout << materials[j];
			cout << endl;
		}
	}
}

int RoundToNearestPow2(int num)
{
	return (int)pow(2, ceil(log(num) / log(2)));
}

void EndApp(int success)
{
	cout << "Press enter to continue...";
	cin.ignore();
	exit(success);
}