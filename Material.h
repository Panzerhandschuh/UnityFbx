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
	path bumpMap;
	bool hasSpecular;
	path specularMap;
	FbxDouble3 specularColor;
	FbxDouble specularFactor;
	FbxDouble2 uvScaling;
	FbxDouble2 uvTranslation;
	FbxDouble3 uvRotation;

	bool operator==(const Material &mat)
	{
		return (diffuseMap == mat.diffuseMap && diffuseColor == mat.diffuseColor &&
			hasOpacity == mat.hasOpacity && opacity == mat.opacity &&
			bumpMap == mat.bumpMap && hasSpecular == mat.hasSpecular && 
			specularMap == mat.specularMap && specularColor == mat.specularColor && 
			specularFactor == mat.specularFactor && uvScaling == mat.uvScaling && 
			uvTranslation == mat.uvTranslation && uvRotation == mat.uvRotation);
	}
};

#endif