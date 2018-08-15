// \brief
//		Image loader, supports:
//  1. png
//

#pragma once

#include <cstdint>

enum EPixelFormat
{
	PIXEL_Unknown = 0,
	PIXEL_RGB,
	PIXEL_RGBA,
	FIXEL_MAX
};


class FImageIO
{
public:
	static uint32_t BytesPerPixel(int32_t InFormat);

	// \brief
	//  NOTE: use delete[] to free the OutBytes memeory.
	static bool ReadImage(const char* InFilename, uint8_t *&OutBytes, uint32_t &OutWidth, uint32_t &OutHeight, int32_t &OutFormat);
	static bool WriteImage(const char* InFilename, const uint8_t *InBytes, uint32_t InWidth, uint32_t InHeight, int32_t InFormat);
};

