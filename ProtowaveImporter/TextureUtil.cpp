#include "stdafx.h"
#include "TextureUtil.h"

using namespace std;
using namespace boost::filesystem;

void TextureUtil::CreateDDSFiles(const path &materialsDir, const vector<Material> &materials, vector<string> &createdFiles)
{
	path texconvPath(initial_path() / "texconv.exe");
	if (exists(texconvPath))
	{
		for (unsigned int i = 0; i < materials.size(); i++)
		{
			CreateDDSFile(materialsDir, materials[i].diffuseMap, createdFiles);
			CreateDDSFile(materialsDir, materials[i].normalMap, createdFiles);
			CreateDDSFile(materialsDir, materials[i].specularMap, createdFiles);

			// Print material info
			cout << "Material " << i << " Info:" << endl;
			MaterialUtil::PrintMaterial(materials[i]);
			cout << endl;
		}
	}
	else
		cout << "Warning: Missing texconv.exe, textures will not be saved." << endl;
}

void TextureUtil::CreateDDSFile(const path &materialsDir, const path &texturePath, vector<string> &createdFiles)
{
	if (texturePath.empty()) // Skip empty textures
		return;

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
		cerr << "Create DDS Error: Could not read source image info, DDS file will not be generated" << endl;
	}
}
