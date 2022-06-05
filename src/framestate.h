#ifndef FRAME_STATE_H_GUARD
#define FRAME_STATE_H_GUARD
#include "framedata.h"

struct FrameState
{
	int pattern;
	int frame;
	std::vector<Layer> *layers;
	bool animating;
	int animeSeq;
	int selectedLayer = 0;

	FrameState();
	~FrameState();

	struct CopyData{
		Frame_AS copiedAs;
		Frame_AF copiedAf;
		Frame copyFrame;
	} *copy;

private:
	void *sharedMemHandle = nullptr;
	void *sharedMem = nullptr;

	//Offsets in memory.

	
};

#endif /* FRAME_STATE_H_GUARD */
