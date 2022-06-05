#ifndef FRAME_STATE_H_GUARD
#define FRAME_STATE_H_GUARD
#include "framedata.h"
#include <linear_allocator.hpp>

struct FrameState
{
	int pattern = 0;
	int frame = 0;
	std::vector<Layer> *layers = nullptr;
	bool animating = 0;
	int animeSeq = 0;
	int selectedLayer = 0;

	FrameState();
	~FrameState();

	struct CopyData{
		Frame_AS as{};
		Frame_AF_T<LinearAllocator> af{};
		Frame_T<LinearAllocator> frame{};
		Sequence_T<LinearAllocator> pattern{};
		std::vector<Frame_T<LinearAllocator>, LinearAllocator<Frame_T<LinearAllocator>>> frames{};

		Frame_AT at {};
		std::vector<Frame_EF,LinearAllocator<Frame_EF>> efGroup{};
		std::vector<Frame_IF,LinearAllocator<Frame_IF>> ifGroup{};
		Frame_IF ifSingle{};
		Frame_EF efSingle{};

		BoxList_T<LinearAllocator> boxes;
		Hitbox box;
	} *copied;

private: 
	void *sharedMemHandle = nullptr;
	void *sharedMem = nullptr;
};

#endif /* FRAME_STATE_H_GUARD */
