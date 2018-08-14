// \brief
//		Image IO

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "libpng\png.h"
#include "libpng\zlib.h"

#include "ImageIO.h"

struct FPixelFormatPair
{
	int32_t	MyPixelFormat;
	int32_t PNGPixelFormat;
};

static FPixelFormatPair sPNGPixelFormatPair[] = 
	{
		{ PIXEL_RGB, PNG_FORMAT_RGB },
		{ PIXEL_RGBA, PNG_FORMAT_RGBA },
		{ PIXEL_RGBA, PNG_FORMAT_LINEAR_RGB_ALPHA }
	};

static int32_t PNGFormatToEPixelFormat(int32_t InFormat)
{
	return InFormat;

	for (int32_t i = 0; i < sizeof(sPNGPixelFormatPair) / sizeof(sPNGPixelFormatPair[0]); i++)
	{
		if (sPNGPixelFormatPair[i].PNGPixelFormat == InFormat)
		{
			return sPNGPixelFormatPair[i].MyPixelFormat;
		}
	}

	return PIXEL_Unknown;
}

static int32_t EPixelFormatToPNGFormat(int32_t InFormat)
{
	return InFormat;

	for (int32_t i = 0; i < sizeof(sPNGPixelFormatPair) / sizeof(sPNGPixelFormatPair[0]); i++)
	{
		if (sPNGPixelFormatPair[i].MyPixelFormat == InFormat)
		{
			return sPNGPixelFormatPair[i].PNGPixelFormat;
		}
	}

	assert(0);
	return -1;
}

static bool VerifyPNGLibVersionInner()
{
	printf("libpng version: %s\n", PNG_LIBPNG_VER_STRING);
	printf("with zlib version: %s\n", ZLIB_VERSION);
	printf("library (%lu):%s\n", (unsigned long)png_access_version_number(), png_get_header_version(NULL));

	if (strcmp(png_libpng_ver, PNG_LIBPNG_VER_STRING))
	{
		printf("error: versions are different between png.h and png.c\n");
		printf("       png.h version:%s\n", PNG_LIBPNG_VER_STRING);
		printf("       png.c version:%s\n", png_libpng_ver);

		return false;
	}

	return true;
}

static bool VerifyPNGLibVersion()
{
	static bool bValid = VerifyPNGLibVersionInner();
	return bValid;
}

bool FImageIO::ReadImage(const char *InFilename, uint8_t *&OutBytes, uint32_t &OutWidth, uint32_t &OutHeight, int32_t &OutFormat)
{
	OutBytes = NULL;
	OutWidth = 0;
	OutHeight = 0;
	OutFormat = PIXEL_Unknown;

	if (!VerifyPNGLibVersion())
	{
		return false;
	}

	png_image image; /* The control structure used by libpng */

	/* Initialize the 'png_image' structure. */
	memset(&image, 0, (sizeof image));
	image.version = PNG_IMAGE_VERSION;

	/* The first argument is the file to read: */
	if (png_image_begin_read_from_file(&image, InFilename) != 0)
	{
		png_bytep buffer;

		/* Set the format in which to read the PNG file; this code chooses a
		* simple sRGB format with a non-associated alpha channel, adequate to
		* store most images.
		*/
		image.format = PNG_FORMAT_RGBA;

		/* Now allocate enough memory to hold the image in this format; the
		* PNG_IMAGE_SIZE macro uses the information about the image (width,
		* height and format) stored in 'image'.
		*/
		buffer = new png_byte[(PNG_IMAGE_SIZE(image))];

		/* If enough memory was available read the image in the desired format
		* then write the result out to the new file.  'background' is not
		* necessary when reading the image because the alpha channel is
		* preserved; if it were to be removed, for example if we requested
		* PNG_FORMAT_RGB, then either a solid background color would have to
		* be supplied or the output buffer would have to be initialized to the
		* actual background of the image.
		*
		* The fourth argument to png_image_finish_read is the 'row_stride' -
		* this is the number of components allocated for the image in each
		* row.  It has to be at least as big as the value returned by
		* PNG_IMAGE_ROW_STRIDE, but if you just allocate space for the
		* default, minimum, size using PNG_IMAGE_SIZE as above you can pass
		* zero.
		*
		* The final argument is a pointer to a buffer for the colormap;
		* colormaps have exactly the same format as a row of image pixels (so
		* you choose what format to make the colormap by setting
		* image.format).  A colormap is only returned if
		* PNG_FORMAT_FLAG_COLORMAP is also set in image.format, so in this
		* case NULL is passed as the final argument.  If you do want to force
		* all images into an index/color-mapped format then you can use:
		*
		*    PNG_IMAGE_COLORMAP_SIZE(image)
		*
		* to find the maximum size of the colormap in bytes.
		*/
		if (buffer != NULL &&
			png_image_finish_read(&image, NULL/*background*/, buffer,
				0/*row_stride*/, NULL/*colormap*/) != 0)
		{
			OutBytes = buffer;
			OutWidth = image.width;
			OutHeight = image.height;
			OutFormat = PNGFormatToEPixelFormat(image.format);
			
			return true;
		}
		else
		{
			/* Calling png_image_free is optional unless the simplified API was
			* not run to completion.  In this case if there wasn't enough
			* memory for 'buffer' we didn't complete the read, so we must free
			* the image:
			*/
			if (buffer == NULL)
			{
				png_image_free(&image);
			}
			else
			{
				delete[] buffer;
			}
		}

		/* Something went wrong reading the image.  libpng stores a
		* textual message in the 'png_image' structure:
		*/
		if (PNG_IMAGE_FAILED(image))
		{
			printf("ReadImage() failed, error: %s\n", image.message);
		}
	} // end if

	return false;
}

bool FImageIO::WriteImage(const char *InFilename, const uint8_t *InBytes, uint32_t InWidth, uint32_t InHeight, int32_t InFormat)
{
	if (!InFilename || !InBytes)
	{
		return false;
	}
	if (!VerifyPNGLibVersion())
	{
		return false;
	}

	png_image image; /* The control structure used by libpng */

	/* Initialize the 'png_image' structure. */
	memset(&image, 0, (sizeof image));
	image.version = PNG_IMAGE_VERSION;
	image.width = InWidth;  
	image.height= InHeight;	
	image.format = PNG_FORMAT_RGBA; // EPixelFormatToPNGFormat(InFormat);

	/* Now write the image out to the second argument.  In the write
	* call 'convert_to_8bit' allows 16-bit data to be squashed down to
	* 8 bits; this isn't necessary here because the original read was
	* to the 8-bit format.
	*/
	if (png_image_write_to_file(&image, InFilename, 0/*convert_to_8bit*/,
		InBytes, 0/*row_stride*/, NULL/*colormap*/) != 0)
	{
		return true;
	}

	/* Something went wrong writing the image.  libpng stores a
	* textual message in the 'png_image' structure:
	*/
	if (PNG_IMAGE_FAILED(image))
	{
		printf("ReadImage() failed, error: %s\n", image.message);
	}

	return false;
}
