#ifndef PARTS_H_GUARD
#define PARTS_H_GUARD

#include "image.h"
#include "cg.h"
#include "texture.h"
#include "vao.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <glm/mat4x4.hpp>

class Parts {
public:
	CG* cg;
	char* data = nullptr;

	struct Shape
	{
		int type;
		int vertexCount;
		int vertexCount2;
		int length;
		int length2;
		int radius;
		int dRadius;
		int width;
		int dz;
	};

	//Box cut-out
	struct CutOut
	{
		char* name;

		int uv[4];
		int xy[2];
		int wh[2];
		int texture = 0;
		//int type;
		int jump;
		int vaoIndex;
		unsigned char colorSlot;
		int shapeIndex;
	};

	struct PartGfx {
		char* name;
		char* data;
		int w, h;
		int bpp;
		int type;
		int textureIndex;
		unsigned char* s3tc = nullptr;
		bool dontDelete = false;
	};

	struct PartProperty {
		float priority;
		float rotation[3];
		float x;
		float y;
		float scaleX = 1.f;
		float scaleY = 1.f;
		int ppId;
		bool additive;
		bool filter;
		char flip;
		unsigned char bgra[4] = { 255,255,255,255 };
		unsigned char addColor[4] = { 0,0,0,0 };
	};

	//Has id and a list of parts

	std::unordered_map<int, std::vector<PartProperty>> groups;
	std::unordered_map<int, CutOut> cutOuts;
	std::unordered_map<int, Shape> shapes;
	std::unordered_map<int, PartGfx> gfxMeta;
	std::vector<Texture*> textures;
	Vao partVertices;
	int curTexId;

	unsigned int* MainLoad(unsigned int* data, const unsigned int* data_end);
	unsigned int* P_Load(unsigned int* data, const unsigned int* data_end, int id);
	unsigned int* PrLoad(unsigned int* data, const unsigned int* data_end, int groupId, int propId);
	unsigned int* PpLoad(unsigned int* data, const unsigned int* data_end, int id);
	unsigned int* PgLoad(unsigned int* data, const unsigned int* data_end, int id);
	unsigned int* VeLoad(unsigned int* data, const unsigned int* data_end, int amount, int len);
	unsigned int* VnLoad(unsigned int* data, const unsigned int* data_end, int amount);
	void DrawPart(int id);

public:
	bool loaded = false;

	Parts(CG* cg);
	bool Load(const char* name);

	PartGfx* GetTexture(unsigned int n);
	void Draw(int pattern, std::function<void(glm::mat4)> setMatrix,
		std::function<void(float, float, float)> setAddColor,
		std::function<void(char)> setFlip, float color[4]);

};

#endif /* PARTS_H_GUARD */
