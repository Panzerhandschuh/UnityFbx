#include "stdafx.h"
#include "MaterialUtil.h"

using namespace std;
using namespace boost::filesystem;

void MaterialUtil::WriteMaterialsToFile(const path &outputFile, const path &materialsDir, const vector<Material> &materials)
{
	path pwmatFile(outputFile.stem().string() + ".pwmat");
	path outputPath = outputFile.parent_path() / pwmatFile;
	string pwmdlFileName = outputFile.stem().string();
	std::ofstream file;
	file.open(outputPath.string());

	for (unsigned int i = 0; i < materials.size(); i++)
	{
		file << "Material Standard" << endl;
		file << "{" << endl;
		WriteMaterial(file, materials[i], pwmdlFileName);
		file << "}";
		if (i != materials.size() - 1) // Create line spacing if this is not the last material
			file << endl << endl;
	}
	file.close();
}

void MaterialUtil::WriteMaterial(ofstream& os, const Material &mat, const string &pwmdlFileName)
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
	if (!MathUtil::AlmostEqual(mat.uvScaling, FbxDouble2(1, 1)))
		os << "\t" << "Tiling " << mat.uvScaling << endl;
	if (!MathUtil::AlmostEqual(mat.uvTranslation, FbxDouble2(0, 0)))
		os << "\t" << "Offset " << mat.uvTranslation << endl;
}

void MaterialUtil::PrintMaterial(const Material &mat)
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
	if (!MathUtil::AlmostEqual(mat.uvScaling, FbxDouble2(1, 1)))
		cout << "Tiling: " << mat.uvScaling << endl;
	if (!MathUtil::AlmostEqual(mat.uvTranslation, FbxDouble2(0, 0)))
		cout << "Offset: " << mat.uvTranslation << endl;
}

void MaterialUtil::WriteColor(std::ofstream& os, const FbxDouble3 &d3)
{
	os << (int)(d3[0] * 255) << " " << (int)(d3[1] * 255) << " " << (int)(d3[2] * 255);
}

void MaterialUtil::PrintColor(const FbxDouble3 &d3)
{
	cout << "R:" << (int)(d3[0] * 255) << " G:" << (int)(d3[1] * 255) << " B:" << (int)(d3[2] * 255);
}