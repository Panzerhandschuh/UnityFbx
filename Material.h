#ifndef MATERIAL_H
#define MATERIAL_H

#include <fbxsdk.h>
#include <vector>
#include <boost\filesystem\path.hpp>

using namespace std;
using namespace boost::filesystem;

struct Material
{
	path diffuseMap;
	FbxDouble3 diffuseColor;
	bool hasOpacity;
	FbxDouble opacity;
	path normalMap;
	path specularMap;
	FbxDouble2 uvScaling;
	FbxDouble2 uvTranslation;

	static void LoadDefault(Material &mat)
	{
		FbxDouble diffuseValue = 150.0 / 255.0;
		FbxDouble3 defaultDiffuse(diffuseValue, diffuseValue, diffuseValue);
		mat.diffuseColor = defaultDiffuse;
		mat.hasOpacity = false;
		mat.uvScaling = FbxDouble2(1, 1);
		mat.uvTranslation = FbxDouble2(0, 0);
	}

	bool operator==(const Material &mat)
	{
		return (diffuseMap == mat.diffuseMap && diffuseColor == mat.diffuseColor &&
			hasOpacity == mat.hasOpacity && opacity == mat.opacity &&
			normalMap == mat.normalMap && specularMap == mat.specularMap && 
			uvScaling == mat.uvScaling && uvTranslation == mat.uvTranslation);
	}
};

#endif