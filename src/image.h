#ifndef IMAGE_H_GUARD
#define IMAGE_H_GUARD

struct ImageData
{
	unsigned char *pixels = nullptr;
	int width;
	int height;
	bool is8bpp;
	bool bgr;
	int offsetX;
	int offsetY;
	int csize = 0;
	int compressionType = 0;

	~ImageData()
	{
		//delete[] pixels;
	}
};


#endif /* IMAGE_H_GUARD */
