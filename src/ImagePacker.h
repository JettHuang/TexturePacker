// \brief
//		Image packer
// Algorithm:
//		ref: https://blog.csdn.net/hherima/article/details/38560929
//		Divide the rectangle as following recursively. 
//      right zone & left zone will be divided when instert a new image.
//       *------------------*----------------------*
//       |   image 0        |       right zone     |   
//       |                  |                      |
//       *-----------------------------------------*
//       |                                         |
//       |            left zone                    |
//       |                                         |
//       |                                         |
//       *-----------------------------------------*
//

#pragma once

#include <cstdint>
#include <string>


// Image packer
class FImagePacker
{
public:
	// \brief
	//		pack a group images.
	// \params
	//		InImageFilenames    a array of images's file name
	//		InCount				count of array
	//		InWidth, InHeight	
	//		InHMargin			horizontal margin for image
	//		InVMargin			vertical margin for image
	// return how many images are packed.
	static uint32_t PackImages(const char *InImageFilenames[], uint32_t InCount, uint32_t InWidth, uint32_t InHeight, uint32_t InHMargin, uint32_t InVMargin, char *InBigImageFilename);
};
