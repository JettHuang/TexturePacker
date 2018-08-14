// main.cpp
//

#include <cstdio>
#include "ImageIO.h"


int main(int argc, char *argv[])
{
	uint8_t *pixels = NULL;
	uint32_t width, height;
	int32_t format;

	if (FImageIO::ReadImage("toucan.png", pixels, width, height, format))
	{
		FImageIO::WriteImage("toucan_copy.png", pixels, width, height, format);
	}

	delete[] pixels;

	return 0;
}
