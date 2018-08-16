// \brief
//		Image IO

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "SOIL.h"
#include "ImageIO.h"


uint32_t FImageIO::BytesPerPixel(int32_t InFormat)
{
	uint32_t BytesCount = 0;
	switch (InFormat)
	{
	case PIXEL_RGB:
		BytesCount = 3; break;
	case PIXEL_RGBA:
		BytesCount = 4; break;
	default:
		BytesCount = 0; break;
	}

	return BytesCount;
}

bool FImageIO::ReadImage(const char *InFilename, uint8_t *&OutBytes, uint32_t &OutWidth, uint32_t &OutHeight, int32_t &OutFormat)
{
	OutBytes = NULL;
	OutWidth = 0;
	OutHeight = 0;
	OutFormat = PIXEL_Unknown;

	int width, height, channels;
	unsigned char *buffer = SOIL_load_image(InFilename, &width, &height, &channels, SOIL_LOAD_AUTO);
	if (buffer)
	{
		switch (channels)
		{
		case 3:
			OutFormat = PIXEL_RGB; break;
		case 4:
			OutFormat = PIXEL_RGBA; break;
		default:
			OutFormat = PIXEL_Unknown; break;
		}

		do 
		{
			if (OutFormat == PIXEL_Unknown)
			{
				printf("Unknown pixel format of %s\n", InFilename);
				break;
			}

			const uint32_t kBytesCount = width * height * channels;
			OutBytes = new uint8_t[kBytesCount];
			if (!OutBytes)
			{
				break;
			}

			OutWidth = width;
			OutHeight = height;
			memcpy(OutBytes, buffer, kBytesCount);

			SOIL_free_image_data(buffer);
			return true;
		} while (0);
	
		SOIL_free_image_data(buffer);
	}

	printf("ReadImage Failed: %s\n", SOIL_last_result());
	return false;
}

bool FImageIO::WriteImage(const char *InFilename, const uint8_t *InBytes, uint32_t InWidth, uint32_t InHeight, int32_t InFormat)
{
	if (!InFilename)
	{
		return false;
	}

	// check saved type
	int image_type = SOIL_SAVE_TYPE_PNG;
	{
		size_t len = strlen(InFilename);
		if (len > 4)
		{
			const char *postfix = InFilename + (len - 4);
			if (!_stricmp(postfix, ".bmp"))
			{
				image_type = SOIL_SAVE_TYPE_BMP;
			}
			else if (!_stricmp(postfix, ".dds"))
			{
				image_type = SOIL_SAVE_TYPE_DDS;
			}
			else if (!_stricmp(postfix, ".tag"))
			{
				image_type = SOIL_SAVE_TYPE_TGA;
			}
		}
	}

	// check format
	int channels = 0;
	switch (InFormat)
	{
	case PIXEL_RGB:
		channels = 3; break;
	case PIXEL_RGBA:
		channels = 4; break;
	default:
		channels = 0; break;
	}
	if (channels <= 0)
	{
		return false;
	}

	int result = SOIL_save_image(InFilename, image_type, InWidth, InHeight, channels, InBytes);
	return !!result;
}
