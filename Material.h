#ifndef MATERIAL_H
#define MATERIAL_H

#include <fbxsdk.h>
#include <vector>
#include <boost\filesystem\path.hpp>

using namespace std;
using namespace boost::filesystem;

struct Material
{
	path baseTexture;
	path bumpMap;
	FbxDouble3 diffuse;
	bool hasSpecular;
	FbxDouble3 specular;
	FbxDouble shininess;
	FbxDouble3 uvScaling;
	FbxDouble3 uvTranslation;

	bool operator==(const Material &mat)
	{
		return (baseTexture == mat.baseTexture && bumpMap == mat.bumpMap && 
			diffuse == mat.diffuse && hasSpecular == mat.hasSpecular && 
			specular == mat.specular && shininess == mat.shininess &&
			uvScaling == mat.uvScaling && uvTranslation == mat.uvTranslation);
	}
};

#endif