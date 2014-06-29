#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>

using namespace std;
using namespace boost::filesystem;

class ImageInfo
{
public:
	struct ImageData
	{
		int width;
		int height;
		bool hasAlpha;
	};
	static bool GetImageInfo(const path &imgPath, ImageData &img);
	static bool IsValidImage(const path &imgPath);
private:
	static bool GetBmpInfo(FILE *file, ImageData &img);
	static bool GetDdsInfo(FILE *file, ImageData &img);
	static bool GetJpgInfo(FILE *file, ImageData &img);
	static bool GetPngInfo(FILE *file, ImageData &img);
	static bool GetTgaInfo(FILE *file, ImageData &img);
};

#endif