#ifndef RENDER_H_GUARD
#define RENDER_H_GUARD

#include "cg.h"
#include "parts.h"
#include "texture.h"
#include "shader.h"
#include "vao.h"
#include "hitbox.h"
#include "framedata.h"
#include <vector>
#include <unordered_map>
#include <glm/mat4x4.hpp>

class Render
{
private:
	glm::mat4 projection, perspective, invOrtho;
	glm::mat4 globalView = glm::mat4(1.f);
	
	CG *cg;
	Parts *parts;
	Vao vSprite;
	Vao vGeometry;
	enum{
		LINES = 0,
		BOXES,
		GEO_SIZE
	};
	int geoParts[GEO_SIZE];
	float imageVertex[6*4];
	std::vector<float> clientQuads;
	int quadsToDraw;

	int lProjectionS, lProjectionT;
	int lAlphaS, lAddColorT;
	int lFlip;
	Shader sSimple;
	Shader sTextured;
	std::vector<Texture> textures;
	std::vector<Layer> layers;
	float colorRgba[4];

	void AdjustImageQuad(int x, int y, int w, int h);
	void SetFlip(int lFlip, bool flip);
	void SetMatrix(int location, glm::mat4 view);
	void SetMatrixPersp(int location, glm::mat4 view, glm::mat4 pre);
	void SetBlendingMode();

	float wd = 1.f, hd = 1.f;
	
	float iScale;

public:
	bool drawLines = true, drawBoxes = true;
	bool filter;
	int x = 0;
	int y = 0;
	float scale = 1;

	int highLightN = -1;
	
	Render(CG* cg, Parts* parts);
	void Draw();
	void UpdateProj(float w, float h);

	void GenerateHitboxVertices(const BoxList &hitboxes);
	void SwitchImage(std::vector<Layer> *layers);
	void DontDraw();
	void SetImageColor(float *rgbaArr);
	void SetScale(float scale);

	enum blendType{
		normal,
		additive,
		substractive
	};

	blendType blendingMode;
};

#endif /* RENDER_H_GUARD */
