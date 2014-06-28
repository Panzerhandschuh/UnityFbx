#include "stdafx.h"
#include "ImageInfo.h"

bool ImageInfo::GetImageInfo(const path &imagePath, ImageData &img)
{
	if (!IsValidImage(imagePath))
		return false;

	string extension = imagePath.extension().string();
	FILE* file;
	file = fopen(imagePath.string().c_str(), "rb");

	bool success = false;
	if (boost::iequals(extension, ".bmp"))
		success = GetBmpInfo(file, img);
	else if (boost::iequals(extension, ".dds"))
		success = GetDdsInfo(file, img);
	else if (boost::iequals(extension, ".jpg"))
		success = GetJpgInfo(file, img);
	else if (boost::iequals(extension, ".png"))
		success = GetPngInfo(file, img);
	else if (boost::iequals(extension, ".tga"))
		success = GetTgaInfo(file, img);

	fclose(file);

	return success;
}

bool ImageInfo::IsValidImage(const path &imagePath)
{
	if (imagePath.empty())
		return false;
	string extension = imagePath.extension().string();
	if (boost::iequals(extension, ".bmp") || boost::iequals(extension, ".dds") || 
		boost::iequals(extension, ".jpg") || boost::iequals(extension, ".png") ||
		boost::iequals(extension, ".tga"))
		return true;
	else
		return false;
}

bool ImageInfo::GetBmpInfo(FILE *file, ImageData &img)
{
	unsigned short fileType;
	fread(&fileType, 2, 1, file);
	if (fileType != 0x4D42)
		return false;

	// Get dimensions
	fseek(file, 16, SEEK_CUR); // Seek to width info
	fread(&img.width, 4, 1, file);
	fread(&img.height, 4, 1, file);

	// Get bit count
	unsigned short bitCount;
	fseek(file, 2, SEEK_CUR); // Seek to bit count
	fread(&bitCount, 2, 1, file);
	if (bitCount == 32)
		img.hasAlpha = true;
	else
		img.hasAlpha = false;
	//cout << "Bit Count: " << bitCount << endl;

	return true;
}

bool ImageInfo::GetDdsInfo(FILE *file, ImageData &img)
{
	char magicNumber[4];
	fread(magicNumber, 1, 4, file);
	if (strncmp(magicNumber, "DDS ", 4) != 0)
		return false;

	// Get dimensions
	fseek(file, 8, SEEK_CUR); // Seek to width
	fread(&img.width, 4, 1, file);
	fread(&img.height, 4, 1, file);

	fseek(file, 64, SEEK_CUR); // Seek to pixel format
	unsigned char code[4];
	fread(code, 1, 4, file);
	if (code[0] == 'D' && code[1] == 'X' && code[2] == 'T')
	{
		if (code[3] == '1')
			img.hasAlpha = false;
		else if (code[3] == '5')
			img.hasAlpha = true;
		else
			return false;
	}
	else
		return false;

	return true;
}

#define readbyte(a,b) do if(((a)=getc((b))) == EOF) return 0; while (0)
#define readword(a,b) do { int cc_=0,dd_=0; \
                          if((cc_=getc((b))) == EOF \
        		  || (dd_=getc((b))) == EOF) return 0; \
                          (a) = (cc_<<8) + (dd_); \
                          } while(0)

bool ImageInfo::GetJpgInfo(FILE *file, ImageData &img)
{
	img.hasAlpha = false;

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
				readword((img.height), file);
				readword((img.width), file);
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

	// Get entire file in bytes
	//fseek(file, 0, SEEK_END);
	//long dataSize = ftell(file);
	//fseek(file, 0, SEEK_SET);
	//std::vector<unsigned char> data(dataSize);
	//fread(&data[0], dataSize, 1, file);

	//int i = 0; // Keeps track of the position within the file
	//// Check for valid JPEG image
	//if(data[i] == 0xFF && data[i + 1] == 0xD8 && data[i + 2] == 0xFF && data[i + 3] == 0xE0)
	//{
	//	i += 4;
	//	// Check for valid JPEG header (null terminated JFIF)
	//	if(data[i + 2] == 'J' && data[i + 3] == 'F' && data[i + 4] == 'I' && data[i + 5] == 'F' && data[i + 6] == 0x00)
	//	{
	//		// Retrieve the block length of the first block since the first block will not contain the size of file
	//		unsigned short blockLength = data[i] * 256 + data[i + 1];
	//		while(i < dataSize)
	//		{
	//			i += blockLength; // Increase the file index to get to the next block
	//			if(i >= dataSize)
	//				return false; // Check to protect against segmentation faults
	//			if(data[i] != 0xFF)
	//				return false; // Check that we are truly at the start of another block
	//			if(data[i + 1] == 0xC0)
	//			{
	//				// 0xFFC0 is the "Start of frame" marker which contains the file size
	//				// The structure of the 0xFFC0 block is quite simple [0xFFC0][ushort length][uchar precision][ushort x][ushort y]
	//				img.height = data[i + 5] * 256 + data[i + 6];
	//				img.width = data[i + 7] * 256 + data[i + 8];
	//				return true;
	//			}
	//			else
	//			{
	//				i += 2; // Skip the block marker
	//				blockLength = data[i] * 256 + data[i + 1]; //Go to the next block
	//			}
	//		}
	//		return false; // If this point is reached then no size was found
	//	}
	//	else // Not a valid JFIF string
	//		return false;
	//}
	//else // Not a valid SOI header
	//	return false;
}

bool ImageInfo::GetPngInfo(FILE *file, ImageData &img)
{
	unsigned char header[8];
	fread(header, 1, 8, file);
	
	// Verify header
	if (header[0] != 0x89 || header[1] != 0x50 || header[2] != 0x4E || header[3] != 0x47 || 
		header[4] != 0x0D || header[5] != 0x0A || header[6] != 0x1A || header[7] != 0x0A)
	{
		cout << "asdf";
		return false;
	}

	// Width and height will have a reversed byte ordering
	fseek(file, 8, SEEK_CUR); // Seek to width
	int rWidth, rHeight;
	fread(&rWidth, 4, 1, file);
	fread(&rHeight, 4, 1, file);

	img.width = _byteswap_ulong(rWidth);
	img.height = _byteswap_ulong(rHeight);

	// Get color type
	fseek(file, 1, SEEK_CUR); // Seek to color type
	unsigned char colorType;
	fread(&colorType, 1, 1, file);
	if (colorType == 4 || colorType == 6)
		img.hasAlpha = true;
	else
		img.hasAlpha = false;

	return true;
}

bool ImageInfo::GetTgaInfo(FILE *file, ImageData &img)
{
	// Get dimensions
	fseek(file, 12, SEEK_SET); // Seek to width
	short sWidth, sHeight;
	fread(&sWidth, 2, 1, file);
	fread(&sHeight, 2, 1, file);
	img.width = (int)sWidth;
	img.height = (int)sHeight;

	// Get bit depth
	unsigned char depth;
	fread(&depth, 1, 1, file);
	//cout << (int)depth << " " << img.width << " " << img.height << endl;
	if (depth == 32)
		img.hasAlpha = true;
	else
		img.hasAlpha = false;

	return true;
}

