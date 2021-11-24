#include "parts.h"
#include "misc.h"
#include "cg.h"

#include <algorithm>
#include <iostream>
#include <cassert>

#include <glm/gtc/matrix_transform.hpp>

struct DDS_PIXELFORMAT
{
	uint32_t    size;
	uint32_t    flags;
	uint32_t    fourCC;
	uint32_t    RGBBitCount;
	uint32_t    RBitMask;
	uint32_t    GBitMask;
	uint32_t    BBitMask;
	uint32_t    ABitMask;
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth;
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

Parts::Parts(CG* cg) : cg(cg), partVertices(Vao::F3F3, GL_STATIC_DRAW)
{

}

unsigned int* Parts::VeLoad(unsigned int* data, const unsigned int* data_end, int amount, int len)
{
	for (int id = 0; id < amount && data < data_end; id++) {
		Shape sh{};
		sh.type = data[0];
		if (sh.type == 3 || sh.type == 4) {
			sh.radius = data[3];
			sh.width = data[4];
			sh.vertexCount = data[5];
			sh.length = data[6];
		}
		else if (sh.type == 6) {
			sh.radius = data[3];
			sh.width = data[4];
			sh.vertexCount = data[5];
			sh.segment = data[6];
			sh.length = data[7];
		}
		shapes.insert({ id,sh });
		data += len;
	}
	return data;
}
//UNI only
unsigned int* Parts::VnLoad(unsigned int* data, const unsigned int* data_end, int amount)
{
	while (data < data_end) {
		unsigned int* buf = data;
		++data;

		if (!memcmp(buf, "VNST", 4)) {
			data += 8 * amount;
		}
		else if (!memcmp(buf, "VEED", 4)) {
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown VE level tag: " << tag << "\n";
		}
	}
	return data;
}

bool Decrappress(unsigned char* cdata, unsigned char* outData, size_t csize, size_t outSize)
{
	int accum = 0;
	size_t wi = 0;
	for (size_t i = 0; i < csize; ++i)
	{
		if (wi > outSize)
			return false;
		if (cdata[i] == 0)
		{
			++i;
			for (size_t j = 0; j < cdata[i + 1]; ++j)
				outData[wi++] = cdata[i];
			i += 1;
		}
		else
		{
			outData[wi++] = cdata[i];
		}
	}
	return true;
}

//Graphic data
unsigned int* Parts::PgLoad(unsigned int* data, const unsigned int* data_end, int id)
{
	PartGfx tex{};
	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		if (!memcmp(buf, "PGNM", 4)) {
			tex.name = (char*)data;
			//std::cout << id <<" PG: "<<(char*)data << "\n";
			data += 0x20 / 4;
		}
		else if (!memcmp(buf, "PGTP", 4)) {
			//tex.type = data[0];
			//std::cout <<"\tGraphic type"<<tex.type <<"\n";
			++data;
		}
		else if (!memcmp(buf, "PGTE", 4)) { //UNI
		 //two shorts? Size again?
			++data;
		}
		else if (!memcmp(buf, "PGT2", 4)) { //UNI
			unsigned int someSize = data[0]; // is size1 + 16?
			tex.w = data[1];
			tex.h = data[2];
			data += 3;

			if (*data == 21)
				tex.type = 21;
			else if (!memcmp(data, "DXT1", 4))
				tex.type = 1;
			else if (!memcmp(data, "DXT5", 4))
				tex.type = 5;
			else
				assert(0 && "Unknown texture type");

			data += 1;

			if (tex.type == 5 && someSize == tex.w * tex.h + 128) //uncompressed?
			{
				data += 2; //skip unknown shit
				tex.s3tc = ((unsigned char*)data) + 128;
				tex.dontDelete = true;
				data += tex.w * tex.h / 4 +0x20;
			}
			else
			{
				int cSize = data[4]; //From 'DDS ' to PGED
				int oSize = data[5]; //Uncompressed size.
				data += 6;

				unsigned char* cData = (unsigned char*)data;
				auto oData = new unsigned char[oSize];
				bool result = Decrappress(cData, oData, cSize, oSize);
				assert(result);
				DDS_HEADER* dds = (DDS_HEADER*)(oData + 4);
				tex.s3tc = oData + 128;

				cData += cSize;
				data = (unsigned int*)cData;
			}
		}
		else if (!memcmp(buf, "PGTX", 4)) {
			tex.w = data[0];
			tex.h = data[1];
			tex.bpp = data[2];
			tex.data = (char*)(data + 3);
			//std::cout <<"\t"<<tex.w <<"x" <<tex.h <<" "<<tex.bpp<<"bpp\n";

			assert(tex.bpp == 32);
			data += (tex.w * tex.h) + 3;
		}
		else if (!memcmp(buf, "PGED", 4)) {
			gfxMeta.insert({ id,tex });
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			//std::cout <<"\tUnknown PG level tag: " << tag <<"\n";
		}
	}
	return data;
}

//More like box cut-outs
unsigned int* Parts::PpLoad(unsigned int* data, const unsigned int* data_end, int id)
{
	CutOut pp{};
	std::string name;

	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		if (!memcmp(buf, "PPNM", 4)) {
			pp.name = (char*)data;
			//std::cout << id <<" PP: "<<(char*)data << "\n";
			data += 0x20 / 4;
		}
		else if (!memcmp(buf, "PPNA", 4)) { //UNI
		 //Non null terminated name. Sjis
			unsigned char* cdata = (unsigned char*)data;
			name.resize(*cdata);
			memcpy(name.data(), (cdata + 1), *cdata);
			//std::cout << id <<" PP: "<< name << "\n";
			cdata += *cdata + 1; //Length
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PPCC", 4)) {
			//XY. Transform coordinates seem inverted because they're vertex coordinates.
			//This fransform is applied earlier.
			memcpy(pp.xy, data, sizeof(int) * 2);
			data += 2;
		}
		else if (!memcmp(buf, "PPUV", 4)) {
			//Texture coords
			//LEFT, TOP, W, H
			memcpy(pp.uv, data, sizeof(int) * 4);
			data += 4;
		}
		else if (!memcmp(buf, "PPSS", 4)) {
			//W and H of coordinates. +Y is down as usual.
			memcpy(pp.wh, data, sizeof(int) * 2);
			data += 2;
		}
		else if (!memcmp(buf, "PPTE", 4)) { //UNI
		 //two shorts?
			++data;
		}
		else if (!memcmp(buf, "PPPA", 4)) { //UNI
			pp.colorSlot = *((char*)data);
			++data;
		}
		else if (!memcmp(buf, "PPTP", 4)) {
			//Texture reference id. Default is 0.
			/* if(name.empty())
				std::cout << id <<" PP: ----\n";
			std::cout << "\t"<<"PPTP " << *data << "\n"; */
			pp.texture = *data;
			++data;
		}
		else if (!memcmp(buf, "PPPP", 4)) {
			pp.shapeIndex = *data;
			++data;
		}
		else if (!memcmp(buf, "PPTX", 4)) {
			assert(0); //melty only?
			//No idea. Doesn't seem to do anything.
			++data;
		}
		else if (!memcmp(buf, "PPJP", 4)) {
			//Some XY offset for when the part "grabs" the player
			//Doesn't affect rendering so we don't care about it.
			data += 2;
		}
		else if (!memcmp(buf, "PPED", 4)) {
			cutOuts.insert({ id,pp });
			break;

		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown PP level tag: " << tag << "\n";
		}
	}
	return data;
}

//Part properties
unsigned int* Parts::PrLoad(unsigned int* data, const unsigned int* data_end, int groupId, int propId)
{
	PartProperty pr{};
	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		if (!memcmp(buf, "PRXY", 4)) {
			pr.x = ((int*)data)[0];
			pr.y = ((int*)data)[1];
			data += 2;
		}
		else if (!memcmp(buf, "PRAL", 4)) { //Opt
			unsigned char* cdata = (unsigned char*)data;
			pr.additive = *cdata;
			++cdata;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRRV", 4)) { //Opt
			unsigned char* cdata = (unsigned char*)data;
			pr.reverse = *cdata;
			++cdata;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRFL", 4)) { //Opt
			unsigned char* cdata = (unsigned char*)data;
			pr.filter = *cdata;
			++cdata;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRZM", 4)) {
			pr.scaleX = *((float*)data);
			pr.scaleY = *((float*)data + 1);
			data += 2;
		}
		else if (!memcmp(buf, "PRSP", 4)) {
			//Add color. bgra
			memcpy(pr.addColor, data, sizeof(char) * 4);
			++data;
		}
		else if (!memcmp(buf, "PRAN", 4)) {
			//Float, rotation, clockwise. 1.f = 360
			assert(0); //Deprecated?

			++data;
		}
		else if (!memcmp(buf, "PRPR", 4)) {
			//Priority. Higher value means draw first / lower on the stack.
			pr.priority = *data;
			pr.priority += propId / 1000.f; //AAAAAAA
			++data;
		}
		else if (!memcmp(buf, "PRID", 4)) {
			//Part id. Which thing to render
			pr.ppId = *data;
			++data;
		}
		else if (!memcmp(buf, "PRCL", 4)) {
			//Color key
			memcpy(pr.bgra, data, sizeof(char) * 4);
			++data;
		}
		else if (!memcmp(buf, "PRA3", 4)) { //UNI angle
		 //No idea what the first float is. Don't think it's a quaternion.
			memcpy(pr.rotation, data + 1, sizeof(float) * 3);
			data += 4;
		}
		else if (!memcmp(buf, "PRED", 4)) {
			groups[groupId].push_back(pr);
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown PR level tag: " << tag << "\n";
		}
	}
	return data;
}

//Contains many parts
unsigned int* Parts::P_Load(unsigned int* data, const unsigned int* data_end, int id)
{
	std::string name;
	bool hasData = false;
	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		if (!memcmp(buf, "PANM", 4)) {
			//Melty name
			name = (char*)data;
			data += 0x20 / 4;
		}
		else if (!memcmp(buf, "PANA", 4)) { //UNI
		 //Non null terminated name
			unsigned char* cdata = (unsigned char*)data;
			name.resize(cdata[0]); //Length at 0
			memcpy(name.data(), cdata + 1, cdata[0]);
			cdata += cdata[0] + 1;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRST", 4)) {
			groups.insert({ id,{} });
			int propId = data[0];
			hasData = true;
			++data;
			data = PrLoad(data, data_end, id, propId);
		}
		else if (!memcmp(buf, "P_ED", 4)) {
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown P_ level tag: " << tag << "\n";
		}
	}
	/* if(hasData)
		std::cout << id <<" _P: "<< name << "\n"; */
	return data;
}


unsigned int* Parts::MainLoad(unsigned int* data, const unsigned int* data_end)
{
	while (data < data_end) {
		unsigned int* buf = data;
		++data;

		if (!memcmp(buf, "P_ST", 4)) {
			unsigned int p_id = *data;
			++data;
			data = P_Load(data, data_end, p_id);
		}
		else if (!memcmp(buf, "PPST", 4)) {
			int id = data[0];
			++data;
			data = PpLoad(data, data_end, id);
		}
		else if (!memcmp(buf, "PGST", 4)) {
			int id = data[0];
			++data;
			data = PgLoad(data, data_end, id);
		}
		else if (!memcmp(buf, "VEST", 4)) { //UNI
			int amount = data[0];
			int len = data[1];
			data += 2;
			data = VeLoad(data, data_end, amount, len);
			data = VnLoad(data, data_end, amount);
		}
		else if (!memcmp(buf, "_END", 4)) {
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown top level tag: " << tag << "\n";
		}
	}
	return data;
}

bool Parts::Load(const char* name)
{
	char* data;
	unsigned int size;
	if (!ReadInMem(name, data, size))
		return false;

	if (memcmp(data, "PAniDataFile", 12))
	{
		delete[] data;
		return false;
	}

	unsigned int* d = (unsigned int*)(data + 0x20);
	unsigned int* d_end = (unsigned int*)(data + size);
	if (memcmp(d, "_STR", 4)) {
		delete[] data;
		return false;
	}

	delete[] this->data;
	this->data = data;
	for (auto& tex : textures)
		delete tex;
	textures.clear();
	cutOuts.clear();
	gfxMeta.clear();
	groups.clear();
	partVertices.Clear();
	MainLoad(d + 1, d_end);
	std::cout << std::endl;

	//Load textures
	for (auto& gfxKv : gfxMeta)
	{
		auto& gfx = gfxKv.second;
		textures.push_back(new Texture);
		if (gfx.s3tc)
		{
			if (gfx.type == 21)
			{
				textures.back()->LoadDirect((char*)gfx.s3tc, gfx.w, gfx.h);
			}
			else
			{
				size_t size;
				if (gfx.type == 5)
					size = gfx.w * gfx.h;
				else if (gfx.type == 1)
					size = (gfx.w * gfx.h) * 3 / 6;
				else
					assert(0);

				textures.back()->LoadCompressed((char*)gfx.s3tc, gfx.w, gfx.h, size, gfx.type);
			}
			if (!gfx.dontDelete)
				delete[](gfx.s3tc - 128);
		}
		else
			textures.back()->LoadDirect(gfx.data, gfx.w, gfx.h);
		gfx.textureIndex = textures.back()->id;
	}

	//Load vertex data in vao.
	std::vector<float> vertexData;
	vertexData.reserve(128 * 5);
	for (auto& partKv : cutOuts)
	{
		auto& part = partKv.second;
		constexpr int tX[] = { 0,1,1, 1,0,0 };
		constexpr int tY[] = { 0,0,1, 1,1,0 };
		constexpr int tXI[] = { 0,1,1, 1,0,0 };
		constexpr int tYI[] = { 1,1,0, 0,0,1 };
		struct {
			float x, y, z, s, t, u;
		}point[6];
		float width = 256;//gfxMeta[part.texture].w/2.f;
		float height = 256;//gfxMeta[part.texture].h/2.f;
		if (gfxMeta.count(part.texture) == 0)
		{
			//std::cout << "There's no graphic id "<< part.texture<<" requested by part id "<<partKv.first<<"\n";
			continue;
		}
		if (part.shapeIndex && shapes.count(part.shapeIndex) != 0 && (shapes[part.shapeIndex].type == 4 || shapes[part.shapeIndex].type == 3)) {
			Shape s = shapes[part.shapeIndex];
			auto size = vertexData.size();
			vertexData.resize(size + 6 * s.vertexCount * 6);
			float angle = 0;
			float delta = glm::pi<float>() * s.length / 5000 / s.vertexCount;
			switch (shapes[part.shapeIndex].type) {
			case 3:
				for (int i = 0; i < s.vertexCount; i++) {
					for (int j = 0; j < 6; j++)
					{
						point[j].x = float(s.radius * glm::sin(angle + delta * tY[j]));
						point[j].y = -float(s.radius * glm::cos(angle + delta * tY[j]));
						point[j].z = float(s.width * -(tX[j] * 2 - 1));
						point[j].s = float(part.uv[0] + part.uv[2] * (tX[j])) / width;
						point[j].t = float(part.uv[1] + part.uv[3] * (i + tY[j]) / s.vertexCount) / height;
						point[j].u = float(part.uv[0] + part.uv[2] * (1 - tX[j])) / width;
					}
					angle += delta;
					memcpy(&vertexData[size + 6 * 6 * i], point, sizeof(point));
				}
				break;
			case 4:
				for (int i = 0; i < s.vertexCount; i++) {
					for (int j = 0; j < 6; j++)
					{
						point[j].x = float((s.radius - (1 - tX[j]) * s.width) * glm::sin(angle + delta * tY[j]));
						point[j].y = -float((s.radius - (1 - tX[j]) * s.width) * glm::cos(angle + delta * tY[j]));
						point[j].z = 0;
						point[j].s = float(part.uv[0] + part.uv[2] * (tX[j])) / width;
						point[j].t = float(part.uv[1] + part.uv[3] * (i + tY[j]) / s.vertexCount) / height;
						point[j].u = float(part.uv[0] + part.uv[2] * (1 - tX[j])) / width;
					}
					angle += delta;
					memcpy(&vertexData[size + 6 * 6 * i], point, sizeof(point));
				}
				break;
			//WIP
			case 6:
				for (int i = 0; i < s.vertexCount; i++) {
					for (int j = 0; j < s.segment; j++) {
						for (int k = 0; k < 6; k++)
						{
						}
						angle += delta;
						memcpy(&vertexData[size + 6 * (6 * s.segment * i + j)], point, sizeof(point));
					}
				}
				break;
			}
		}
		else {
			for (int i = 0; i < 6; i++)
			{
				point[i].x = -part.xy[0] + part.wh[0] * tX[i];
				point[i].y = -part.xy[1] + part.wh[1] * tY[i];
				point[i].z = 0;
				point[i].s = float(part.uv[0] + part.uv[2] * tX[i]) / width;
				point[i].t = float(part.uv[1] + part.uv[3] * tY[i]) / height;
				point[i].u = float(part.uv[0] + part.uv[2] * (1 - tX[i])) / width;
			}
			auto size = vertexData.size();
			vertexData.resize(size + 6 * 6);
			memcpy(&vertexData[size], point, sizeof(point));
		}
	}
	int index = 0;
	for (auto& cutOut : cutOuts)
	{
		if (gfxMeta.count(cutOut.second.texture) == 0)
			continue;
		if (cutOut.second.shapeIndex && shapes.count(cutOut.second.shapeIndex) != 0 && (shapes[cutOut.second.shapeIndex].type == 4 || shapes[cutOut.second.shapeIndex].type == 3)) {
			cutOut.second.vaoIndex = partVertices.Prepare(6 * 6 * shapes[cutOut.second.shapeIndex].vertexCount * sizeof(float), &vertexData[index]);
			index += 6 * 6 * shapes[cutOut.second.shapeIndex].vertexCount;
		}
		else {
			cutOut.second.vaoIndex = partVertices.Prepare(6 * 6 * sizeof(float), &vertexData[index]);
			index += 6 * 6;
		}
	}

	//Sort parts in group by priority;
	for (auto& group : groups)
	{
		auto& v = group.second;
		std::stable_sort(v.rbegin(), v.rend(), [](const PartProperty& a, const PartProperty& b) {
			return a.priority < b.priority;
			});
	}

	partVertices.Load();
	loaded = true;
	return true;
}

void Parts::Draw(int pattern,
	std::function<void(glm::mat4)> setMatrix,
	std::function<void(float, float, float)> setAddColor,
	std::function<void(bool)> setFlip, float color[4])
{
	curTexId = -1;
	if (groups.count(pattern))
	{
		partVertices.Bind();
		for (auto& part : groups[pattern])
		{
			if (cutOuts.count(part.ppId) == 0)
				continue;
			auto cutout = cutOuts[part.ppId];

			constexpr float tau = glm::pi<float>() * 2.f;
			glm::mat4 view = glm::mat4(1.f);

			view = glm::translate(view, glm::vec3(part.x, part.y, 0.f));
			view = glm::rotate(view, -part.rotation[1] * tau, glm::vec3(0.0, 1.f, 0.f));
			view = glm::rotate(view, -part.rotation[0] * tau, glm::vec3(1.0, 0.f, 0.f));
			view = glm::rotate(view, part.rotation[2] * tau, glm::vec3(0.0, 0.f, 1.f));

			setFlip(part.reverse);

			view = glm::scale(view, glm::vec3(part.scaleX, part.scaleY, 1.f));

			setMatrix(view);
			if (screenShot)
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
			else if (part.additive)
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			else
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			float newColor[4];
			memcpy(newColor, color, sizeof(float) * 4);
			newColor[0] *= part.bgra[2] / 255.f;
			newColor[1] *= part.bgra[1] / 255.f;
			newColor[2] *= part.bgra[0] / 255.f;
			newColor[3] *= part.bgra[3] / 255.f;
			if (cutout.colorSlot != 0) {
				unsigned int color = cg->getColorFromPal(cutout.colorSlot);
				if (color) {
					newColor[0] *= (color & 0xFF) / 255.f;
					newColor[1] *= ((color >> 8) & 0xFF) / 255.f;
					newColor[2] *= ((color >> 16) & 0xFF) / 255.f;
					newColor[3] *= (color >> 24) / 255.f;
				}

			}
			glVertexAttrib4fv(2, newColor);
			setAddColor(part.addColor[2] / 255.f, part.addColor[1] / 255.f, part.addColor[0] / 255.f);
			DrawPart(part.ppId);
		}
	}
}

void Parts::DrawPart(int i)
{
	if (cutOuts.count(i) > 0 && gfxMeta.count(cutOuts[i].texture))
	{
		if (curTexId != gfxMeta[cutOuts[i].texture].textureIndex)
		{
			curTexId = gfxMeta[cutOuts[i].texture].textureIndex;
			glBindTexture(GL_TEXTURE_2D, curTexId);
		}
		partVertices.Draw(cutOuts[i].vaoIndex);
	}
}

Parts::PartGfx* Parts::GetTexture(unsigned int n)
{
	if (gfxMeta.count(n))
	{
		return &gfxMeta[n];
	}
	return nullptr;
}