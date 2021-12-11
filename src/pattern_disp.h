#include <imgui.h>
#include <imgui_input_text.h>
#include "framedata.h"

//returns has to update
inline bool PatternDisplay(Sequence *seq)
{
	constexpr float spacing = 80;
	bool update = false;
	
	if(ImGui::InputText("Pattern name", &seq->name))
		update = true;
	if(ImGui::InputText("Code name", &seq->codeName))
		update = true;
	
	ImGui::SetNextItemWidth(40.f);
	ImGui::InputInt("PSTS", &seq->psts, 0, 0);

	ImGui::SetNextItemWidth(40.f);
	ImGui::InputInt("Level", &seq->level, 0, 0);

	ImGui::SetNextItemWidth(80.f);
	//ImGui::InputInt("PFLG", &seq->flag, 0, 0); Unused by the game.
	ImGui::InputInt("PUPS", &seq->pups, 0, 0);
	return update;
}