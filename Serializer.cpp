#include "stdafx.h"
#include "Serializer.h"

void Serializer::SerializeMagicNumber(ofstream &file, const char *text)
{
	file.write(text, strlen(text));
}

void Serializer::Serialize(ofstream &file, const char *text)
{
	char textLength = (char)strlen(text);
	file.write(&textLength, 1);
	file.write(text, strlen(text));
}

void Serializer::Serialize(ofstream &file, int num)
{
	file.write((char*)&num, sizeof(num));
}

void Serializer::Serialize(ofstream &file, float num)
{
	file.write((char*)&num, sizeof(num));
}

void Serializer::Serialize(ofstream &file, Vector2 vec)
{
	file.write((char*)&vec, sizeof(vec));
}

void Serializer::Serialize(ofstream &file, Vector3 vec)
{
	file.write((char*)&vec, sizeof(vec));
}