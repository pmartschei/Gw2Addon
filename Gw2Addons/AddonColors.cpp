#include "AddonColors.h"
ImVec4 Addon::Colors[AddonColor_COUNT];
const char * Addon::GetStyleColorName(int idx)
{
	switch (idx)
	{
	case AddonColor_PositiveText: return "PositiveText";
	case AddonColor_NegativeText: return "NegativeText";
	case AddonColor_DisabledHeader: return "DisabledHeader";
	case AddonColor_DisabledHeaderActive: return "DisabledHeaderActive";
	case AddonColor_DisabledHeaderHovered: return "DisabledHeaderHovered";
	case AddonColor_FrameBgHighlighted: return "FrameBgHighlighted";
	case AddonColor_TextHighlighted: return "TextHighlighted";
	}
	return "Unknown";
}

void Addon::ClassicColors()
{
	Colors[AddonColor_PositiveText] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	Colors[AddonColor_NegativeText] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	Colors[AddonColor_DisabledHeader] = ImVec4(1.0f, 0.45f, 0.20f, 1.0f);
	Colors[AddonColor_DisabledHeaderActive] = ImVec4(1.0f, 0.65f, 0.40f, 1.0f);
	Colors[AddonColor_DisabledHeaderHovered] = ImVec4(1.0f, 0.55f, 0.30f, 1.0f);
	Colors[AddonColor_FrameBgHighlighted] = ImVec4(0.8f, 0.85f, 1.0f, 1.0f);
	Colors[AddonColor_TextHighlighted] = ImVec4(0.0f,0.0f,0.0f, 1.0f);
}
