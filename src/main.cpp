// main.cpp
//

#include <cstdio>
#include "ImageIO.h"


int main(int argc, char *argv[])
{
	uint8_t *pixels = NULL;
	uint32_t width, height;
	int32_t format;

	if (FImageIO::ReadImage("img_test_copy.bmp", pixels, width, height, format))
	{
		FImageIO::WriteImage("img_test_copy2.bmp", pixels, width, height, format);
	}

	delete[] pixels;

	return 0;
}
