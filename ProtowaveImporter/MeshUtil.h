#pragma once

#include <iostream>
#include <boost/filesystem/path.hpp>
#include "Mesh.h"
#include "BinaryWriter.h"

class MeshUtil
{
public:
	static void WriteMeshToFile(const boost::filesystem::path &outputFile, const Mesh &mesh, bool importNormals = true);
	static void WriteCollisionsToFile(const boost::filesystem::path &outputFile, const Mesh &mesh, bool importNormals = true);
};