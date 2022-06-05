#ifndef DRAWWINDOW_H_GUARD
#define DRAWWINDOW_H_GUARD

#include "render.h"
#include "framestate.h"

//ImGui Windows that draw themselves. Just for utility.
class DrawWindow
{
public:
	DrawWindow(Render* render, FrameData *frameData, FrameState &state):
		render(render),
		frameData(frameData),
		currState(state){};

	FrameState &currState;
	bool isVisible = true;

	virtual void Draw() = 0;
protected:
	Render *render;
	FrameData *frameData;
};

#endif /* DRAWWINDOW_H_GUARD */
