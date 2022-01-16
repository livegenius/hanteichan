#ifndef MAINPANE_H_GUARD
#define MAINPANE_H_GUARD
#include "draw_window.h"
#include "framedata.h"
#include "render.h"
#include <string>
#include <list>

//This is the main pane on the left
class MainPane : DrawWindow
{
public:
	MainPane(Render* render, FrameData *frameData, FrameState &fs);
	void Draw();

	void RegenerateNames();

private:
	Sequence copiedPattern{};
	
	std::vector<Frame> copiedFrames;
	Frame_AS copiedAs{};
	Frame_AF copiedAf{};
	Frame copyFrame{};
	bool copyThisFrame = true;
	std::string *decoratedNames;

	bool rangeWindow = false;
	int ranges[2]{};
	
	int afjcRangeVal = 0;

	struct SequenceWId{
		int id;
		Sequence seq;
	};

	std::list<SequenceWId> patCopyStack;
	void PopCopies();

};


#endif /* MAINPANE_H_GUARD */
