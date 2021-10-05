#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED
#include <string>
#include "cg.h"
#include <memory>

extern int pal;

class Texture
{
public: //public only for ease of reading access, do not write anything.
	std::unique_ptr<ImageData> image;
	unsigned int id; //OpenGL texture id.
	bool isLoaded;
	bool isApplied;
	int w, h;
	int offsetX = 0;
	int offsetY = 0;
	
public:
	Texture();
	Texture(Texture&& texture);
	~Texture();

	
	void Load(ImageData *data);
	void Apply(bool repeat = false, bool linearFilter = false);
	void LoadDirect(char *data, int w, int h, bool bgr = true);
	void LoadCompressed(char *data, int w, int h, int compressedSize, int type);
	void Unapply();
	void Unload();
};

#endif // IMAGE_H_INCLUDED
