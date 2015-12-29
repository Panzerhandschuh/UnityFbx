#include "stdafx.h"
#include "ImageInfo.h"
#include <iostream>

using namespace std;
using namespace boost::filesystem;

ImageInfo::ImageInfo(const path &imagePath)
{
	string extension = imagePath.extension().string();
	string validExtensions[] = { ".bmp", ".dds", ".jpg", ".png", ".tga" };
	if (find(begin(validExtensions), end(validExtensions), extension) == end(validExtensions)) // Check if the extension is valid
		throw runtime_error("Invalid image format (" + extension + ")");

	FILE* file;
	file = fopen(imagePath.string().c_str(), "rb");
	if (!file)
		throw runtime_error("Failed to open file (" + imagePath.string() + ")");

	bool success = false;
	if (boost::iequals(extension, ".bmp"))
		success = GetBmpInfo(file);
	else if (boost::iequals(extension, ".dds"))
		success = GetDdsInfo(file);
	else if (boost::iequals(extension, ".jpg"))
		success = GetJpgInfo(file);
	else if (boost::iequals(extension, ".png"))
		success = GetPngInfo(file);
	else if (boost::iequals(extension, ".tga"))
		success = GetTgaInfo(file);

	fclose(file);

	if (!success)
		throw runtime_error("Failed to get image info (" + imagePath.string() + ")");
}

bool ImageInfo::GetBmpInfo(FILE *file)
{
	unsigned short fileType;
	fread(&fileType, 2, 1, file);
	if (fileType != 0x4D42)
		return false;

	// Get dimensions
	fseek(file, 16, SEEK_CUR); // Seek to width info
	fread(&width, 4, 1, file);
	fread(&height, 4, 1, file);

	// Get bit count
	unsigned short bitCount;
	fseek(file, 2, SEEK_CUR); // Seek to bit count
	fread(&bitCount, 2, 1, file);
	hasAlpha = (bitCount == 32);

	return true;
}

bool ImageInfo::GetDdsInfo(FILE *file)
{
	char magicNumber[4];
	fread(magicNumber, 1, 4, file);
	if (strncmp(magicNumber, "DDS ", 4) != 0)
		return false;

	// Get dimensions
	fseek(file, 8, SEEK_CUR); // Seek to width
	fread(&width, 4, 1, file);
	fread(&height, 4, 1, file);

	fseek(file, 64, SEEK_CUR); // Seek to pixel format
	unsigned char code[4];
	fread(code, 1, 4, file);
	if (code[0] == 'D' && code[1] == 'X' && code[2] == 'T')
	{
		if (code[3] == '1')
			hasAlpha = false;
		else if (code[3] == '5')
			hasAlpha = true;
		else
			return false;
	}
	else
		return false;

	return true;
}

#define readbyte(a,b) do if(((a)=getc((b))) == EOF) return 0; while (0)
#define readword(a,b) do { int cc_=0,dd_=0; \
	if ((cc_ = getc((b))) == EOF \
		|| (dd_ = getc((b))) == EOF) return 0; \
	(a) = (cc_ << 8) + (dd_); \
	} while (0)

bool ImageInfo::GetJpgInfo(FILE *file)
{
	hasAlpha = false;

	int marker = 0;
	int dummy = 0;
	if (getc(file) != 0xFF || getc(file) != 0xD8)
		return false;

	while (true)
	{
		int discardedBytes = 0;
		readbyte(marker, file);
		while (marker != 0xFF)
		{
			discardedBytes++;
			readbyte(marker, file);
		}
		do readbyte(marker, file); while (marker == 0xFF);

		if (discardedBytes != 0)
			return false;

		switch (marker)
		{
			case 0xC0:
			case 0xC1:
			case 0xC2:
			case 0xC3:
			case 0xC5:
			case 0xC6:
			case 0xC7:
			case 0xC9:
			case 0xCA:
			case 0xCB:
			case 0xCD:
			case 0xCE:
			case 0xCF:
			{
				readword(dummy, file); // Usual parameter length count
				readbyte(dummy, file);
				readword(height, file);
				readword(width, file);
				readbyte(dummy, file);

				return true;
			}
			case 0xDA:
			case 0xD9:
				return 0;
			default:
			{
				int length;
				readword(length, file);
				if (length < 2)
					return false;

				length -= 2;
				while (length > 0)
				{
					readbyte(dummy, file);
					length--;
				}
			}
			break;
		}
	}

	return false;
}

bool ImageInfo::GetPngInfo(FILE *file)
{
	unsigned char header[8];
	fread(header, 1, 8, file);

	// Verify header
	if (header[0] != 0x89 || header[1] != 0x50 || header[2] != 0x4E || header[3] != 0x47 ||
		header[4] != 0x0D || header[5] != 0x0A || header[6] != 0x1A || header[7] != 0x0A)
		return false;

	// Width and height will have a reversed byte ordering
	fseek(file, 8, SEEK_CUR); // Seek to width
	int rWidth, rHeight;
	fread(&rWidth, 4, 1, file);
	fread(&rHeight, 4, 1, file);

	width = _byteswap_ulong(rWidth);
	height = _byteswap_ulong(rHeight);

	// Get color type
	fseek(file, 1, SEEK_CUR); // Seek to color type
	unsigned char colorType;
	fread(&colorType, 1, 1, file);
	hasAlpha = (colorType == 4 || colorType == 6);

	return true;
}

bool ImageInfo::GetTgaInfo(FILE *file)
{
	// Get dimensions
	fseek(file, 12, SEEK_SET); // Seek to width
	short sWidth, sHeight;
	fread(&sWidth, 2, 1, file);
	fread(&sHeight, 2, 1, file);
	width = (int)sWidth;
	height = (int)sHeight;

	// Get bit depth
	unsigned char depth;
	fread(&depth, 1, 1, file);
	hasAlpha = (depth == 32);

	return true;
}

