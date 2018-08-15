// \brief
//		Image Packer
//
//

#include <cstdio>
#include <cassert>
#include <string>
#include <vector>

#include "ImagePacker.h"
#include "ImageIO.h"


static void CopyRectangleMemory(uint8_t *pDst, uint32_t InDstX, uint32_t InDstY, uint32_t InDstLineBytes,
	const uint8_t *pSrc, uint32_t InSrcWidth, uint32_t InSrcHeight);

// FImage
class FImage
{
public:
	~FImage();

	bool IsValid() const { return pixelData != NULL; }
	const std::string& Filename() const { return filename; }
	const uint8_t* Data() const { return pixelData; }
	uint8_t* Data() { return pixelData; };
	uint32_t Width() const { return width; }
	uint32_t Height() const { return height; }
	int32_t Format() const { return format; }

	static FImage* Create(uint32_t InW, uint32_t InH, int32_t InFormat);
	static FImage* LoadFromFile(const char *InFilename);

protected:
	FImage();
	FImage(const FImage &InOther) { /* do nothing */}
	FImage& operator =(const FImage &InOther) { /* do nothing*/ }

private:
	std::string	filename;		// optional
	uint8_t		*pixelData;
	uint32_t	width;
	uint32_t	height;
	int32_t		format;
};

FImage::FImage()
	: pixelData(NULL)
	, width(0)
	, height(0)
	, format(PIXEL_Unknown)
{
}

FImage::~FImage()
{
	delete[] pixelData; pixelData = NULL;
}

FImage* FImage::Create(uint32_t InW, uint32_t InH, int32_t InFormat)
{
	FImage *pNewImage = NULL;

	do {
		const uint32_t PixelBytes = FImageIO::BytesPerPixel(InFormat);
		const uint32_t TotalBytes = InW * InH * PixelBytes;

		if (TotalBytes <= 0) { break; }

		uint8_t *pData = new uint8_t[TotalBytes];
		if (!pData) { break; }

		memset(pData, 0, TotalBytes);

		pNewImage = new FImage();
		if (!pNewImage)
		{
			delete[] pData;
			break;
		}

		pNewImage->filename = "memory";
		pNewImage->pixelData = pData;
		pNewImage->width = InW;
		pNewImage->height = InH;
		pNewImage->format = InFormat;
	} while (0);

	return pNewImage;
}

FImage* FImage::LoadFromFile(const char *InFilename)
{
	FImage *pNewImage = NULL;
	
	do
	{
		uint8_t	   *pData = NULL;
		uint32_t	width = 0;
		uint32_t	height = 0;
		int32_t		format = 0;
		if (!FImageIO::ReadImage(InFilename, pData, width, height, format))
		{
			break;
		}

		pNewImage = new FImage();
		if (!pNewImage)
		{
			delete[] pData;
			break;
		}

		pNewImage->filename = InFilename;
		pNewImage->pixelData = pData;
		pNewImage->width = width;
		pNewImage->height = height;
		pNewImage->format = format;
	} while (0);

	return pNewImage;
}

// class merge context
class FImageMergeContext
{
public:
	struct FImageTileMeta 
	{
	public:
		FImageTileMeta()
			: pOriginImage(NULL)
			, X(0)
			, Y(0)
		{}

		FImageTileMeta(FImage *InOrigin, uint32_t InX, uint32_t InY)
			: pOriginImage(InOrigin)
			, X(InX)
			, Y(InY)
		{}

		FImage	*pOriginImage;
		uint32_t X; // position in the merged image.
		uint32_t Y;
	};

	FImageMergeContext(uint32_t InWidth, uint32_t InHeight, uint32_t InHMargin, uint32_t InVMargin);
	~FImageMergeContext();

	bool DoMerge(const std::vector<FImage*> &InImages);
	const FImage* GetMergedImage() const { return pMergedImage; }
	const std::vector<FImageTileMeta>& GetTileMeta() const { return ImageTileMetas; }

protected:
	class FZoneNode
	{
	public:
		FZoneNode()
			: X(0), Y(0), W(0), H(0)
			, pRightChild(NULL)
			, pLeftChild(NULL)
			, bOccupied(false)
		{}

		FZoneNode(uint32_t InX, uint32_t InY, uint32_t InW, uint32_t InH)
			: X(InX), Y(InY), W(InW), H(InH)
			, pRightChild(NULL)
			, pLeftChild(NULL)
			, bOccupied(false)
		{}

		~FZoneNode()
		{
			delete pRightChild;
			delete pLeftChild;
		}

		uint32_t	X;
		uint32_t	Y;
		uint32_t	W;
		uint32_t	H;
		FZoneNode	*pRightChild;
		FZoneNode	*pLeftChild;
		bool		bOccupied;
	};

	static bool InsertImage(FZoneNode *InZone, FImage *InImage, uint32_t InHMargin, uint32_t InVMargin, FImageTileMeta &OutMeta);
	static FZoneNode *FindZoneRecursively(FZoneNode *InParent, const uint32_t InW, const uint32_t InH);

	void Purge();
private:
	uint32_t	Width;
	uint32_t	Height;
	uint32_t	HMargin;
	uint32_t	VMargin;
	FImage		*pMergedImage;
	std::vector<FImageTileMeta>  ImageTileMetas;
};

FImageMergeContext::FImageMergeContext(uint32_t InWidth, uint32_t InHeight, uint32_t InHMargin, uint32_t InVMargin)
	: Width(InWidth)
	, Height(InHeight)
	, HMargin(InHMargin)
	, VMargin(InVMargin)
	, pMergedImage(NULL)
{

}

FImageMergeContext::~FImageMergeContext()
{
	Purge();
}

bool FImageMergeContext::InsertImage(FZoneNode *InZone, FImage *InImage, uint32_t InHMargin, uint32_t InVMargin, FImageTileMeta &OutMeta)
{
	if (!InZone || !InImage)
	{
		return false;
	}

	const uint32_t kTileWidth = InImage->Width() + InHMargin + InHMargin;
	const uint32_t kTileHeight = InImage->Height() + InVMargin + InVMargin;

	FZoneNode *pDstZone = FindZoneRecursively(InZone, kTileWidth, kTileHeight);
	if (!pDstZone)
	{
		return false;
	}
	
	assert(pDstZone->bOccupied == false);
	pDstZone->bOccupied = true;
	// split it into 2 zones
	pDstZone->pRightChild = new FZoneNode(pDstZone->X + kTileWidth, pDstZone->Y, pDstZone->W - kTileWidth, kTileHeight);
	pDstZone->pLeftChild = new FZoneNode(pDstZone->X, pDstZone->Y + kTileHeight, pDstZone->W, pDstZone->H - kTileHeight);

	// add the meta data
	OutMeta = FImageTileMeta(InImage, pDstZone->X + InHMargin, pDstZone->Y + InVMargin);
	return true;
}

FImageMergeContext::FZoneNode *FImageMergeContext::FindZoneRecursively(FZoneNode *InParent, const uint32_t InW, const uint32_t InH)
{
	if (!InParent || InW > InParent->W || InH > InParent->H)
	{
		return NULL;
	}

	if (!InParent->bOccupied)
	{
		return InParent;
	}

	// first find the right zone.
	FZoneNode *pZone = FindZoneRecursively(InParent->pRightChild, InW, InH);
	if (!pZone)
	{
		pZone = FindZoneRecursively(InParent->pLeftChild, InW, InH);
	}

	return pZone;
}

void FImageMergeContext::Purge()
{
	ImageTileMetas.clear();
	if (pMergedImage)
	{
		delete pMergedImage; pMergedImage = NULL;
	}
}

bool FImageMergeContext::DoMerge(const std::vector<FImage*> &InImages)
{
	Purge();

	if (InImages.size() <= 0)
	{
		return false;
	}

	// check the consistency of images's format.
	assert(InImages[0]);
	const int32_t kFormat = InImages[0]->Format();
	bool bFormatSame = true;
	for (size_t k = 1; k < InImages.size(); k++)
	{
		FImage *pImage = InImages[k];
		assert(pImage);
		if (pImage->Format() != kFormat)
		{
			bFormatSame = false;
			break;
		}
	} // end for k
	if (!bFormatSame)
	{
		return false;
	}

	// allocate the zones
	FZoneNode Zone(0, 0, Width, Height);
	for (size_t k = 0; k < InImages.size(); k++)
	{
		FImage *pImage = InImages[k];
		assert(pImage);

		FImageTileMeta TileMeta;
		if (InsertImage(&Zone, pImage, HMargin, VMargin, TileMeta))
		{
			ImageTileMetas.push_back(TileMeta);
		}
	} // end for k

	// make a merged image & fill in.
	pMergedImage = FImage::Create(Width, Height, kFormat);
	if (!pMergedImage)
	{
		return false;
	}

	uint32_t PixelBytes = FImageIO::BytesPerPixel(kFormat);
	for (size_t k = 0; k < ImageTileMetas.size(); k++)
	{
		const FImageTileMeta &TileMeta = ImageTileMetas[k];
		assert(TileMeta.pOriginImage);

		CopyRectangleMemory(pMergedImage->Data(), TileMeta.X * PixelBytes, TileMeta.Y, pMergedImage->Width() * PixelBytes,
			TileMeta.pOriginImage->Data(), TileMeta.pOriginImage->Width() * PixelBytes, TileMeta.pOriginImage->Height());
	} // end for k

	return true;
}

uint32_t FImagePacker::PackImages(const char *InImageFilenames[], uint32_t InCount, uint32_t InWidth, uint32_t InHeight,
						uint32_t InHMargin, uint32_t InVMargin, char *InBigImageFilename)
{
	std::vector<FImage*> Images;

	for (uint32_t k = 0; k < InCount; k++)
	{
		FImage *pImage = FImage::LoadFromFile(InImageFilenames[k]);
		if (!pImage)
		{
			printf("Failed to load image file %s\n", InImageFilenames[k]);
			continue;
		}

		Images.push_back(pImage);
	} // end for k

	FImageMergeContext Merger(InWidth, InHeight, InHMargin, InVMargin);

	if (Merger.DoMerge(Images))
	{
		const FImage* pMergedImage = Merger.GetMergedImage();
		const std::vector<FImageMergeContext::FImageTileMeta>& TileMetas = Merger.GetTileMeta();

		if (pMergedImage)
		{
			bool bSuccess = FImageIO::WriteImage(InBigImageFilename, pMergedImage->Data(), pMergedImage->Width(), pMergedImage->Height(), pMergedImage->Format());
			if (!bSuccess)
			{
				printf("Failed to save merged image to file: %s\n", InBigImageFilename);
			}
			else
			{
				// output the image tiles information
				printf("Image Atlas Information:\n");
				for (size_t k = 0; k < TileMetas.size(); k++)
				{
					const FImageMergeContext::FImageTileMeta &Meta = TileMetas[k];
					printf("IMAGE: %s\n", Meta.pOriginImage->Filename().c_str());
					printf("     { x:%ud, y:%ud, w:%ud, h:%ud }\n", Meta.X, Meta.Y, Meta.pOriginImage->Width(), Meta.pOriginImage->Height());
				} // end for k
			
				return TileMetas.size();
			}
		} // end if
	}

	for (uint32_t k = 0; k < Images.size(); k++)
	{
		FImage *pImage = Images[k];
		assert(pImage);
		delete pImage;
	} // end for k
	Images.clear();

	return 0;
}

static void CopyRectangleMemory(uint8_t *pDst, uint32_t InDstX, uint32_t InDstY, uint32_t InDstLineBytes,
	const uint8_t *pSrc, uint32_t InSrcWidth, uint32_t InSrcHeight)
{
	assert((InDstLineBytes - InDstX) >= InSrcWidth);

	uint8_t *pDstLine = &pDst[InDstY * InDstLineBytes + InDstX];
	const uint8_t *pSrcLine = pSrc;
	for (uint32_t k = 0; k < InSrcHeight; k++)
	{
		memcpy(pDstLine, pSrcLine, InSrcWidth);
		pDstLine += InDstLineBytes;
		pSrcLine += InSrcWidth;
	} // end for k
}