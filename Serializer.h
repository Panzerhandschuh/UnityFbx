#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <fstream>
#include "Mesh.h"

class Serializer
{
public:
	static void SerializeMagicNumber(ofstream &file, const char *text);
	static void Serialize(ofstream &file, const char *text);
	static void Serialize(ofstream &file, int num);
	static void Serialize(ofstream &file, float num);
	static void Serialize(ofstream &file, Vector2 vec);
	static void Serialize(ofstream &file, Vector3 vec);
};

#endif