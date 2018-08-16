// \brief
//		save png image.
//

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "svpng.inc"


int
save_image_as_PNG
(
	const char *filename,
	int width, int height, int channels,
	const unsigned char *const data
)
{
	/*	variables	*/
	FILE *fout;
	
	/*	error check	*/
	if ((NULL == filename) ||
		(width < 1) || (height < 1) ||
		(channels != 3 && channels != 4) ||
		(data == NULL))
	{
		return 0;
	}

	/*	write it out	*/
	fout = fopen(filename, "wb");
	if (fout)
	{
		svpng(fout, width, height, data, channels == 4);
		fclose(fout);
		return 1;
	}

	return 0;
}
