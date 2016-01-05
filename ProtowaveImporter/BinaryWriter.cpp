#include "stdafx.h"
#include "BinaryWriter.h"

using namespace std;

BinaryWriter::BinaryWriter(const std::string &fname)
{
	file.open(fname, ios::out | ios::binary | ios::trunc);

	if (!file.is_open())
		throw runtime_error("Failed to open file (" + fname + ")");
}

BinaryWriter::~BinaryWriter()
{
	file.close();
}

void BinaryWriter::Write(const char *text)
{
	file.write(text, strlen(text));
}

// Write a length-prefixed string
void BinaryWriter::Write(const std::string &text)
{
	int length = text.length();
	file.write((char*)&length, 1);
	file.write(text.c_str(), length);
}

void BinaryWriter::Write(int num)
{
	file.write((char*)&num, sizeof(num));
}

void BinaryWriter::Write(float num)
{
	file.write((char*)&num, sizeof(num));
}

void BinaryWriter::Write(Vector2 vec)
{
	file.write((char*)&vec, sizeof(vec));
}

void BinaryWriter::Write(Vector3 vec)
{
	file.write((char*)&vec, sizeof(vec));
}