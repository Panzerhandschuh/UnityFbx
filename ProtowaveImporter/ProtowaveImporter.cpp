#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <boost/filesystem/path.hpp>
#include "Mesh.h"
#include "MeshImporter.h"
#include "MeshUtil.h"
#include "MaterialUtil.h"
#include "TextureUtil.h"

using namespace std;
using namespace boost::filesystem;

void PrintCreatedFiles(const path &outputFile, const vector<string> &createdFiles, bool importMaterials = true);

int _tmain(int argc, _TCHAR* argv[])
{
	//argc = 2;
	//argv[1] = L"D:\\Users\\Tyler\\Documents\\Protowave\\Importer\\Pipe.FBX";

	//argv[2] = L"C:\\Users\\Tyler\\Documents\\Protowave\\Models\\Model.pwm";
	//argv[3] = L"-materials";
	//argv[4] = L"C:\\Users\\Tyler\\Documents\\Protowave\\Materials\\Model";

	if (argc < 2)
	{
		cout << "pwimport <fbx file> [options]\n" <<
			"\t-o <directory>\tChange directory for converted mesh, collision, and material files\n" <<
			"\t-t <directory>\tChange directory for converted textures\n" <<
			"\t-c\t\tExport as a collision model\n" <<
			"\t-n\t\tDo not import normals\n" <<
			"\tEx: pwimport \"C:\\FbxFiles\\MyModel.fbx\" -o \"C:\\Protowave\\Models\"" <<
			"\n\t\t-t \"C:\\Protowave\\Materials\"" << endl;
		return EXIT_SUCCESS;
	}

	// Input file
	path inputFile = (argv[1]);
	if (!exists(inputFile))
	{
		cout << "Input file does not exist" << endl;
		return EXIT_FAILURE;
	}
	else if (!boost::iequals(inputFile.extension().string(), ".fbx"))
	{
		cout << "Error: Input file must be a .fbx file" << endl;
		return EXIT_FAILURE;
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
				return EXIT_FAILURE;
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
				return EXIT_FAILURE;
			}
		}
		else if (_tcscmp(argv[i], L"-c") == 0)
			isCollider = true;
		else if (_tcscmp(argv[i], L"-n") == 0)
			importNormals = false;
		else
		{
			cout << "Error: Invalid argument (" << argv[i] << ")" << endl;
			return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}

	// Initialize importer
	MeshImporter importer(inputFile);

	// Import fbx file
	//vector<Mesh> meshes;
	Mesh mesh;
	if (!importer.Import(mesh, isCollider))
		return EXIT_FAILURE;

	// Write the imported mesh info to files
	if (!isCollider) // Standard model
	{
		// Create files
		vector<string> createdFiles;
		TextureUtil::CreateDDSFiles(texturesDir, mesh.materials, createdFiles);
		path pwmdlFile(inputFile.stem().string() + ".pwmdl");
		path outputPath(outputDir / pwmdlFile);
		MaterialUtil::WriteMaterialsToFile(outputPath, texturesDir, mesh.materials);
		MeshUtil::WriteMeshToFile(outputPath, mesh, importNormals);

		// Print created file info to console
		PrintCreatedFiles(outputPath, createdFiles, true);
	}
	else // Collider mesh
	{
		// Create files
		path pwcolFile(inputFile.stem().string() + ".pwcol");
		path outputPath(outputDir / pwcolFile);
		MeshUtil::WriteCollisionsToFile(outputPath, mesh, importNormals);

		// Print created file info to console
		PrintCreatedFiles(outputPath, vector<string>(0), false);
	}

	// Remove import textures
	path fbmDir(inputFile.stem().string() + ".fbm");
	path fbmPath(outputDir / fbmDir);
	if (exists(fbmPath))
		remove_all(fbmPath);
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