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
	AddonColor_COUNT
};
namespace Addon{
	extern ImVec4 Colors[AddonColor_COUNT];
	const char* GetStyleColorName(AddonColor idx);
	void ClassicColors();
}
#endif