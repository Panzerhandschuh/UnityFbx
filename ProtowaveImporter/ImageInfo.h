#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <string>
#include <exception>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>

class ImageInfo
{
public:
	int width, height;
	bool hasAlpha;

	ImageInfo(const boost::filesystem::path &imgPath);
private:
	bool GetBmpInfo(FILE *file);
	bool GetDdsInfo(FILE *file);
	bool GetJpgInfo(FILE *file);
	bool GetPngInfo(FILE *file);
	bool GetTgaInfo(FILE *file);
};

#endif