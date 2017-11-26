#include "AddonColors.h"
const char * Addon::GetStyleColorName(AddonColor idx)
{
	switch (idx)
	{
	case AddonColor_PositiveText: return "PositiveText";
	case AddonColor_NegativeText: return "NegativeText";
	case AddonColor_DisabledHeader: return "DisabledHeader";
	case AddonColor_DisabledHeaderActive: return "DisabledHeaderActive";
	case AddonColor_DisabledHeaderHovered: return "DisabledHeaderHovered";
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
}
