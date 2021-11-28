#include "render.h"
#include "main.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

//Error
#include <windows.h>
#include <glad/glad.h>
#include <sstream>
#include <iostream>

#include "hitbox.h"
#include "misc.h"

constexpr int maxBoxes = 33;

const char* simpleSrcVert = R"(
#version 120
attribute vec3 Position;
attribute vec3 Color;

varying vec4 Frag_Color;

uniform mat4 ProjMtx;
uniform float Alpha;

void main()
{
	Frag_Color = vec4(Color, Alpha);
	gl_Position = ProjMtx * vec4(Position, 1);
}
)";

const char* simpleSrcFrag = R"(
#version 120
varying vec4 Frag_Color;

void main()
{
	gl_FragColor = Frag_Color;
}
)";

const char* texturedSrcVert = R"(
#version 130
attribute vec3 Position;
attribute vec4 UV;
attribute vec4 Color;

varying vec2 Frag_UV;
varying vec4 Frag_Color;

uniform mat4 ProjMtx;
uniform int Flip;

void main()
{
	Frag_UV = vec2((Flip&1)>0?UV.p:UV.s,(Flip&2)>0?UV.q:UV.t);
	Frag_Color = Color;
	gl_Position = ProjMtx * vec4(Position, 1);
}
)";

const char* texturedSrcFrag = R"(
#version 120
uniform sampler2D Texture;

varying vec2 Frag_UV;
varying vec4 Frag_Color;

uniform vec3 AddColor;

void main()
{
	vec4 col = texture2D(Texture, Frag_UV.st);
	col.rgb += AddColor;
	gl_FragColor = col * Frag_Color;
}
)";

Render::Render(CG* cg, Parts* parts):
cg(cg),
parts(parts),
filter(false),
vSprite(Vao::F2F2, GL_DYNAMIC_DRAW),
vGeometry(Vao::F3F3, GL_STREAM_DRAW),
imageVertex{
	256, 256, 	0, 0,
	512, 256,  	1, 0, 
	512, 512,  	1, 1, 

	512, 512, 	1, 1,
	256, 512,  	0, 1,
	256, 256,  	0, 0,
},
colorRgba{1,1,1,1},
quadsToDraw(0),
blendingMode(normal)
{
	std::cerr<<"Simple\n";
	sSimple.BindAttrib("Position", 0);
	sSimple.BindAttrib("Color", 1);
	//sSimple.LoadShader("shd/simple.vert", "shd/simple.frag");
	sSimple.LoadShader(simpleSrcVert, simpleSrcFrag, true);

	sSimple.Use();

	std::cerr<<"Textured\n";
	sTextured.BindAttrib("Position", 0);
	sTextured.BindAttrib("UV", 1);
	sTextured.BindAttrib("Color", 2);
	//sTextured.LoadShader("shd/textured.vert", "shd/textured.frag");
	sTextured.LoadShader(texturedSrcVert, texturedSrcFrag, true);
	
	lAlphaS = sSimple.GetLoc("Alpha");
	lProjectionS = sSimple.GetLoc("ProjMtx");
	lProjectionT = sTextured.GetLoc("ProjMtx");
	lAddColorT = sTextured.GetLoc("AddColor");
	lFlip = sTextured.GetLoc("Flip");

	vSprite.Prepare(sizeof(imageVertex), imageVertex);
	vSprite.Load();

	float lines[]
	{
		-10000, 0, -1,	1,1,1,
		10000, 0, -1,	1,1,1,
		0, 10000, -1,	1,1,1,
		0, -10000, -1,	1,1,1,
	};
	

	geoParts[LINES] = vGeometry.Prepare(sizeof(lines), lines);
	geoParts[BOXES] = vGeometry.Prepare(sizeof(float)*6*4*maxBoxes, nullptr);
	vGeometry.Load();
	vGeometry.InitQuads(geoParts[BOXES]);

	UpdateProj(clientRect.x, clientRect.y);

	glViewport(0, 0, clientRect.x, clientRect.y);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

void Render::SetScale(float newScale)
{
	iScale = newScale;
	scale = iScale;
	/* if(curPat)
		scale *= 0.5f; */
}

constexpr float tau = glm::pi<float>()*2.f;
void Render::Draw()
{
	static unsigned int lastError = 0;
	int err = glGetError();
	if(err && lastError != err)
	{
		lastError = err;
		std::stringstream ss;
		ss << "GL Error: 0x" << std::hex << err << "\n";
		std::cerr << ss.str()<<"\n";
	}

	//Lines
	globalView = glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, 1.f));
	globalView = glm::translate(globalView, glm::vec3(x,y,0.f));
	SetMatrix(lProjectionS, globalView);
	if(drawLines)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1f(lAlphaS, 0.25f);
		vGeometry.Bind();
		vGeometry.Draw(geoParts[LINES], 0, GL_LINES);
	}

	//Draw each layer
	
	int size = layers.size();
	glDisable(GL_DEPTH_TEST);
	for(int i = 0; i < size; ++i)
	{
		auto &layer = layers[i];
		auto &texture = textures[i];
		if(!texture.isApplied && layer.usePat == 0)
			continue;

		glm::mat4 view = glm::scale(glm::mat4(1.f), glm::vec3(layer.scale[0], layer.scale[1], 1.f));
		view = glm::rotate(view, layer.rotation[2]*tau, glm::vec3(0.0, 0.f, 1.f));
		view = glm::rotate(view, layer.rotation[1]*tau, glm::vec3(0.0, 1.f, 0.f));
		view = glm::rotate(view, layer.rotation[0]*tau, glm::vec3(1.0, 0.f, 0.f));
		

		sTextured.Use();
		if(texture.isApplied)
		{
			view = glm::translate(view, glm::vec3(-128+layer.offset_x, -224+layer.offset_y, 0.f));
			glUniform3f(lAddColorT, 0.f,0.f,0.f);
			//if(i==)
			SetFlip(lFlip, 0);
			SetMatrix(lProjectionT, globalView * view);
			//SetMatrixPersp(lProjectionT, view);
			glBindTexture(GL_TEXTURE_2D, texture.id);
			switch (layer.blend_mode)
			{
				case 2:
					blendingMode = Render::additive;
					break;
				case 3:
					blendingMode = Render::substractive;
					break;
				default:
					blendingMode = Render::normal;
					break;
			}
			SetBlendingMode();
			glDisableVertexAttribArray(2);
			SetImageColor(layer.rgba);
			glVertexAttrib4fv(2, colorRgba);
			vSprite.Bind();
			AdjustImageQuad(texture.offsetX, texture.offsetY, texture.w, texture.h);
			vSprite.UpdateBuffer(0, imageVertex);
			vSprite.Draw(0);
		}
		else
		{
			view = glm::translate(view, glm::vec3(layer.offset_x, layer.offset_y, 0.f));
			auto rview = projection;
			rview = glm::scale(rview, glm::vec3(scale, scale, 1.f));
			rview = glm::translate(rview, glm::vec3(x, y, 0));
			rview *= view;
			rview = glm::translate(rview, glm::vec3(0, 0, 1024.f));
			rview *= invOrtho;

			glDisableVertexAttribArray(2);
			parts->Draw(layer.spriteId, [this, &rview](glm::mat4 m){
					SetMatrixPersp(lProjectionT, m, rview);
				},
				[this](float r, float g, float b){
					glUniform3f(lAddColorT,r,g,b);
				},
				[this](char flip) {
					SetFlip(lFlip, flip);
				},
				colorRgba
			);
		}
	}
	glEnable(GL_DEPTH_TEST);
	
	//Reset state
	glBlendEquation(GL_FUNC_ADD);
	sSimple.Use();
	if(drawBoxes)
	{
		vGeometry.Bind();
		if(screenShot){
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
		}
		else{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glUniform1f(lAlphaS, 0.6f);
		vGeometry.DrawQuads(GL_LINE_LOOP, quadsToDraw);
		glUniform1f(lAlphaS, 0.3f);
		vGeometry.DrawQuads(GL_TRIANGLE_FAN, quadsToDraw);
		
	}
}

void Render::SetFlip(int lFlip, char flip)
{
	glUniform1i(lFlip, flip);
}

void Render::SetMatrix(int lProjection, glm::mat4 view)
{
	glUniformMatrix4fv(lProjection, 1, GL_FALSE, glm::value_ptr(projection*view));
}

void Render::SetMatrixPersp(int lProjection, glm::mat4 view, glm::mat4 pre)
{
	glUniformMatrix4fv(lProjection, 1, GL_FALSE, glm::value_ptr(pre*perspective*view));
}

void Render::UpdateProj(float w, float h)
{
	if(w == 0 || h == 0)
		return;
	wd = w;
	hd = h;
	
	projection = glm::ortho<float>(0, w, h, 0, -1024.f, 1024.f);
	invOrtho = glm::inverse(projection);

	//perspective = glm::perspective<float>(90, w / h, 1, 2048);
	constexpr float pi = glm::pi<float>();

	constexpr float dist = 1024;
	constexpr float dist2 = dist*2; 
	constexpr float f = 1.3;
	perspective = glm::translate(glm::mat4(1.f), glm::vec3(-1, 1, 0)) *
		glm::frustum<float>(-w/dist2, w/dist2, h/dist2, -h/dist2, 1*f, dist*2*f);
	//perspective = glm::perspective<float>(90, w / h, 1, 2048);
	perspective = glm::translate(perspective, glm::vec3(0.f, 0.f, -dist*f));
}

void Render::SwitchImage(std::vector<Layer> *layers_)
{	
	if(!layers_)
	{
		layers.clear();
		return;
	}
	
	bool identical = true;
	if(layers.size() == layers_->size())
	{
		for(int i = 0; i < layers.size(); ++i)
			if( layers[i].spriteId != (*layers_)[i].spriteId ||
				layers[i].usePat != (*layers_)[i].usePat)
			{
				identical = false;
				break;
			}
	}
	else
		identical = false;
	
	layers = *layers_;
	if(!identical)
	{
		auto curSize = layers.size();
		textures.clear();
		textures.resize(curSize);
		for(int i = 0; i < curSize; ++i)
		{
			auto &layer = layers[i];
			auto &texture = textures[i];
			if(layer.usePat)
			{
				//scale??
				continue;
/* 				auto gfx = parts->GetTexture(layer.spriteId);
				if(gfx)
				{
					assert(gfx->s3tc);
					size_t size;
					if(gfx->type == 21)
					{
						texture.LoadDirect((char*)gfx->s3tc, gfx->w, gfx->h);
					}
					else
					{
						if(gfx->type == 5)
							size = gfx->w*gfx->h;
						else if(gfx->type == 1)
							size = (gfx->w*gfx->h)*3/6;
						else
							assert(0);
						texture.LoadCompressed((char*)gfx->s3tc, gfx->w, gfx->h,size, gfx->type);
					}
				} */
			}
			else
			{
				ImageData *image = cg->draw_texture(layer.spriteId, false, false);
				if(!image)
					continue;
				texture.Load(image);
				texture.Apply(false, filter);
				texture.Unload(); //Deletes the resource
			}
		}
	}
}

void Render::AdjustImageQuad(int x, int y, int w, int h)
{
	w+=x;
	h+=y;

	imageVertex[0] = imageVertex[16] = imageVertex[20] = x;
	imageVertex[4] = imageVertex[8] = imageVertex[12] = w;

	imageVertex[1] = imageVertex[5] = imageVertex[21] = y;
	imageVertex[9] = imageVertex[13] = imageVertex[17] = h;
}

void Render::GenerateHitboxVertices(const BoxList &hitboxes)
{
	int size = hitboxes.size();
	if(size <= 0)
	{
		quadsToDraw = 0;
		return;
	}

	const float *color;
	//red, green, blue, z order
	constexpr float collisionColor[] 	{1, 1, 1, 1};
	constexpr float greenColor[] 		{0.2, 1, 0.2, 2};
	constexpr float shieldColor[] 		{0, 0, 1, 3}; //Not only for shield
	constexpr float clashColor[]		{1, 1, 0, 4};
	constexpr float projectileColor[] 	{0, 1, 1, 5}; //飛び道具
	constexpr float purple[] 			{0.5, 0, 1, 6}; //特別
	constexpr float redColor[] 			{1, 0.2, 0.2, 7};
	constexpr float hiLightColor[]		{1, 0.5, 1, 10};

	constexpr int tX[] = {0,1,1,0};
	constexpr int tY[] = {0,0,1,1};

	int floats = size*4*6; //4 Vertices with 6 attributes.
	if(clientQuads.size() < floats)
		clientQuads.resize(floats);
	
	int dataI = 0;
	for(const auto &boxPair : hitboxes)
	{
		int i = boxPair.first;
		const Hitbox& hitbox = boxPair.second;

		if (highLightN == i)
			color = hiLightColor;
		else if(i==0)
			color = collisionColor;
		else if (i >= 1 && i <= 8)
			color = greenColor;
		else if(i >=9 && i <= 10)
			color = shieldColor;
		else if(i == 11)
			color = clashColor;
		else if(i == 12)
			color = projectileColor;
		else if(i>12 && i<=24)
			color = purple;
		else
			color = redColor;

		
		for(int j = 0; j < 4*6; j+=6)
		{
			//X, Y, Z, R, G, B
			clientQuads[dataI+j+0] = hitbox.xy[0] + (hitbox.xy[2]-hitbox.xy[0])*tX[j/6];
			clientQuads[dataI+j+1] = hitbox.xy[1] + (hitbox.xy[3]-hitbox.xy[1])*tY[j/6];
			clientQuads[dataI+j+2] = color[3]+1000.f;
			clientQuads[dataI+j+3] = color[0];
			clientQuads[dataI+j+4] = color[1];
			clientQuads[dataI+j+5] = color[2];
		}
		dataI += 4*6;
	}
	quadsToDraw = size;
	vGeometry.UpdateBuffer(geoParts[BOXES], clientQuads.data(), dataI*sizeof(float));
}

void Render::DontDraw()
{
	quadsToDraw = 0;
}

void Render::SetImageColor(float *rgba) //deprecated?
{
	if(rgba)
	{
		for(int i = 0; i < 4; i++)
			colorRgba[i] = rgba[i];
	}
	else
	{
		for(int i = 0; i < 4; i++)
			colorRgba[i] = 1.f;
	}
}

void Render::SetBlendingMode()
{
	switch (blendingMode)
	{
	default:
	case normal:
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case additive:
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case substractive:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		break;
	}
}