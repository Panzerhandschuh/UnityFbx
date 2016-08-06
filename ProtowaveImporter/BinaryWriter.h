#ifndef BinaryWriter_H
#define BinaryWriter_H

#include <fstream>
#include <exception>
#include "Mesh.h"

class BinaryWriter
{
public:
	BinaryWriter(const std::string &fname);
	~BinaryWriter();
	void Write(const char *text);
	void Write(const std::string &text);
	void Write(int num);
	void Write(float num);
	void Write(Vector2 vec);
	void Write(Vector3 vec);

private:
	std::ofstream file;
};

#endif