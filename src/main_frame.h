#ifndef MAINFRAME_H_GUARD
#define MAINFRAME_H_GUARD
#include "context_gl.h"
#include "render.h"
#include "main_pane.h"
#include "right_pane.h"
#include "box_pane.h"
#include "framedata.h"
#include "cg.h"
#include <glm/mat4x4.hpp>
#include <string>

class MainFrame
{
public:
	MainFrame(ContextGl *context);
	~MainFrame();
	
	void Draw();
	void UpdateBackProj(float x, float y);
	void HandleMouseDrag(int x, int y, bool dragRight, bool dragLeft);
	bool HandleKeys(uint64_t vkey);

	void RightClick(int x, int y);

private:
	ContextGl *context;
	float clearColor[3];
	int style_idx = 0;
	int zoom_idx = 3;
	bool smoothRender = false; 
	

	Render render;
	FrameData framedata;
	FrameState currState;
	CG cg;
	int curPalette;

	std::string currentFilePath;

	void DrawBack();
	void DrawUi();
	void Menu(unsigned int errorId);

	void RenderUpdate();
	void AdvancePattern(int dir);
	void AdvanceFrame(int dir);

	void WarmStyle();
	void ChangeClearColor(float r, float g, float b);

	int mDeltaX = 0, mDeltaY = 0;
	int x=0, y=0;
	

	MainPane mainPane;
	RightPane rightPane;
	BoxPane boxPane;
};


#endif /* MAINFRAME_H_GUARD */
