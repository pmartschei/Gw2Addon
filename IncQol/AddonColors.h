#ifndef ADDON_COLORS_H
#define ADDON_COLORS_H
#include <imgui.h>

enum AddonColor
{
	AddonColor_PositiveText,
	AddonColor_NegativeText,
	AddonColor_DisabledHeader,
	AddonColor_DisabledHeaderActive,
	AddonColor_DisabledHeaderHovered,
	AddonColor_FrameBgHighlighted,
	AddonColor_TextHighlighted,
	AddonColor_RarityJunk,
	AddonColor_RarityBasic,
	AddonColor_RarityFine,
	AddonColor_RarityMasterwork,
	AddonColor_RarityRare,
	AddonColor_RarityExotic,
	AddonColor_RarityAscended,
	AddonColor_RarityLegendary,
	AddonColor_COUNT
};
namespace Addon{
	extern ImVec4 Colors[AddonColor_COUNT];
	const char* GetStyleColorName(int idx);
	void ClassicColors();
}
#endif