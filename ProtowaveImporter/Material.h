#ifndef MATERIAL_H
#define MATERIAL_H

#include <fbxsdk.h>
#include <vector>
#include <boost\filesystem\path.hpp>

using namespace std;
using namespace boost::filesystem;

struct Material
{
	FbxDouble3 color;
	bool hasOpacity;
	FbxDouble opacity;
	path diffuseMap;
	path normalMap;
	path specularMap;
	FbxDouble2 uvScaling;
	FbxDouble2 uvTranslation;

	static void LoadDefault(Material &mat)
	{
		FbxDouble colorValue = 150.0 / 255.0;
		mat.color = FbxDouble3(colorValue, colorValue, colorValue);
		mat.hasOpacity = false;
		mat.uvScaling = FbxDouble2(1, 1);
		mat.uvTranslation = FbxDouble2(0, 0);
	}

	bool operator==(const Material &mat)
	{
		return (color == mat.color && hasOpacity == mat.hasOpacity && 
			opacity == mat.opacity && diffuseMap == mat.diffuseMap &&
			normalMap == mat.normalMap && specularMap == mat.specularMap && 
			uvScaling == mat.uvScaling && uvTranslation == mat.uvTranslation);
	}
};

#endif