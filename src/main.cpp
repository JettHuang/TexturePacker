// main.cpp
//

#include <cstdio>
#include "ImageIO.h"
#include "ImagePacker.h"


int main(int argc, char *argv[])
{
// test image IO.
#if 0
	uint8_t *pixels = NULL;
	uint32_t width, height;
	int32_t format;

	if (FImageIO::ReadImage("img_test.png", pixels, width, height, format))
	{
		FImageIO::WriteImage("img_test_copy.bmp", pixels, width, height, format);
	}

	delete[] pixels;
#endif

// test packer
#if 1
	const char *ImageFiles[] = 
	{
		"img_test.png",
		"CloseNormal.png",
		"CloseSelected.png",
		"CocosCreator.png",
		"PurpleMonster.png"
	};

	FImagePacker::PackImages(ImageFiles, sizeof(ImageFiles) / sizeof(ImageFiles[0]), 512, 512, 1, 1, "MergedResult.png");
#endif

	return 0;
}
