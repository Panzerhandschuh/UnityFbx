#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "Material.h"
#include "ImageInfo.h"
#include "MaterialUtil.h"

class TextureUtil
{
public:
	static void CreateDDSFiles(const boost::filesystem::path &materialsDir, const std::vector<Material> &materials, std::vector<std::string> &createdFiles);
	static void CreateDDSFile(const boost::filesystem::path &materialsDir, const boost::filesystem::path &texturePath, std::vector<std::string> &createdFiles);
};