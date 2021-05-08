#ifndef FRAMEDATA_H_GUARD
#define FRAMEDATA_H_GUARD

#include <string>
#include <vector>

#include "hitbox.h"

struct Frame_AF {
	// rendering data
	bool active;

	int spriteId;
	bool usePat;

	int		offset_y;
	int		offset_x;

	int		duration;

	/* Animation flag
	Default: End
	1: Next
	2: Jump to frame
	*/
	int		aniFlag;


	int		blend_mode;

	float	rgba[4]{1,1,1,1};

	float rotation[3]{}; //XYZ

	float scale[2]{1,1};//xy

	//Depends on aniflag.
	//If (0)end, it jumps to the number of the sequence
	//If (2)jump, it jumps to the number of the frame of the seq.
	//It seems to do nothing if the aniflag is 1(next).
	int jump;

	
	int landJump; //Jumps to this frame if landing.
	bool interpolation; //Appears and the end of some effects. Seems to be a boolean value
	int priority; // Default is 0. Used in throws and dodge.
	int loopCount; //Times to loop, it's the frame just before the loop.
	int loopEnd; //The frame number is not part of the loop.
	
	int AFRT;
};

struct Frame_AS {
	// state data
	int		speed_flags;
	int		speed_horz;
	int		speed_vert;
	int		accel_horz;
	int		accel_vert;

	int		ASMV;

	int		stand_state;

	int		cancel_flags;
};

struct Frame_AT {
	bool		active;

	unsigned int guard_flags;
	unsigned int otherFlags;

	int correction;
	//default = 0, is set only if lower
	//1 multiplicative
	//2 substractive
	int correction_type; 

	int damage;
	int red_damage;
	int guard_damage;
	int meter_gain;

	//Stand, Air, Crouch
	int guardVector[3];
	int hitVector[3];
	int gVFlags[3];
	int hVFlags[3];

	int hitEffect;
	int soundEffect; //Changes the audio

	int addedEffect; //Lasting visual effect after being hit

	bool hitgrab;

	//Affects untech time and launch vector, can be negative.
	float extraGravity;

	int breakTime;
	int untechTime;
	int hitStopTime; //Default value zero. Overrides common values.
	int hitStop; //From common value list
};

struct Frame_EF {
	int		command;
	int		parameter;
	int		values[12];
};

struct Frame_IF {
	int		command;
	int		values[12];
};

struct Frame {
	Frame_AF	AF;

	Frame_AS	*AS = nullptr;
	Frame_AT	*AT = nullptr;

	Frame_EF	*EF[8]{};
	Frame_IF	*IF[8]{};

	BoxList hitboxes;
	int nHitbox;

	//int FSNA; Highest attack box index + 1
	//int FSNH; Highest hantei box index + 1
	int FSNE;
	int FSNI;
};

struct Sequence {
	// sequence property data
	std::string	name;
	std::string codeName;
	
	int psts;
	int level;
	int flag;

	bool empty;
	bool initialized;

	std::vector<Frame> frames;
	std::vector<Frame_AT> AT;
	std::vector<Frame_AS> AS;
	std::vector<Frame_EF> EF;
	std::vector<Frame_IF> IF;

	Sequence();
};

class FrameData {
private:
	unsigned int	m_nsequences;

	
public:

	bool		m_loaded;
	Sequence	*m_sequences;
	bool		load(const char *filename);

	//Probably unnecessary.
	//bool load_move_list(Pack *pack, const char *filename);

	int get_sequence_count();

	Sequence* get_sequence(int n);
	std::string GetDecoratedName(int n);

	void Free();

	FrameData();
	~FrameData();
};


#endif /* FRAMEDATA_H_GUARD */
