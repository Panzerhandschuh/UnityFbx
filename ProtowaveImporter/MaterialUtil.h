#pragma once

#include <iostream>
#include <boost/filesystem/path.hpp>
#include "BinaryWriter.h"
#include "MathUtil.h"
#include "FbxUtil.h"

class MaterialUtil
{
public:
	static void WriteMaterialsToFile(const boost::filesystem::path &outputFile, const boost::filesystem::path &materialsDir, const std::vector<Material> &materials);
	static void WriteMaterial(std::ofstream& os, const Material &mat, const std::string &pwmdlFileName);
	static void PrintMaterial(const Material &mat);

private:
	static void WriteColor(std::ofstream& os, const FbxDouble3 &d3);
	static void PrintColor(const FbxDouble3 &d3);
};