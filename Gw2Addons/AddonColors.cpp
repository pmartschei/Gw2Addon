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
	case AddonColor_RarityJunk: return "RarityJunk";
	case AddonColor_RarityBasic: return "RarityBasic";
	case AddonColor_RarityFine: return "RarityFine";
	case AddonColor_RarityMasterwork: return "RarityMasterwork";
	case AddonColor_RarityRare: return "RarityRare";
	case AddonColor_RarityExotic: return "RarityExotic";
	case AddonColor_RarityAscended: return "RarityAscended";
	case AddonColor_RarityLegendary: return "RarityLegendary";
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
	Colors[AddonColor_RarityJunk] = ImVec4(170.0f/255.0f, 170.0f / 255.0f, 170.0f / 255.0f, 1.0f);
	Colors[AddonColor_RarityBasic] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	Colors[AddonColor_RarityFine] = ImVec4(79.0f / 255.0f, 157.0f / 255.0f, 254.0f / 255.0f, 1.0f);
	Colors[AddonColor_RarityMasterwork] = ImVec4(45.0f / 255.0f,197.0f / 255.0f,14.0f / 255.0f, 1.0f);
	Colors[AddonColor_RarityRare] = ImVec4(1.0f, 229.0f/255.0f, 31.0f/255.0f, 1.0f);
	Colors[AddonColor_RarityExotic] = ImVec4(253.0f/255.0f, 165.0f/255.0f, 0.0f, 1.0f);
	Colors[AddonColor_RarityAscended] = ImVec4(252.0f/255.0f, 62.0f / 255.0f, 144.0f / 255.0f, 1.0f);
	Colors[AddonColor_RarityLegendary] = ImVec4(153.0f / 255.0f, 51.0f / 255.0f, 1.0f, 1.0f);
}
