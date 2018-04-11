#ifndef ITEM_FLAGS_H
#define ITEM_FLAGS_H

enum ItemFlags {
	AccountBoundOnAcquire = 0x1,
	Activity = 0x2,
	Dungeon = 0x4,
	Pve = 0x8,
	PvP = 0x10,
	PvPLobby = 0x20,
	UnknownLocation = 0x40,
	WvW = 0x80,
	DeleteWarning = 0x100,
	HideSuffix = 0x200,
	UnknownFlag400 = 0x400,
	UnknownFlag800 = 0x800,
	UnknownFlag1000 = 0x1000,
	UnknownFlag2000 = 0x2000,
	NoSell = 0x4000, 
	NotUpgradeable = 0x8000,
	UnknownFlag10000 = 0x10000,
	SoulboundOnUse = 0x20000,
	Unique = 0x40000,
	UnknownFlag80000 = 0x80000,
	UnknownFlag100000 = 0x100000,
	SAB = 0x200000,
};

inline ItemFlags operator^(ItemFlags lhs, ItemFlags rhs) {
	return (ItemFlags)((uint32_t)lhs ^ (uint32_t)rhs);
}
inline ItemFlags operator|(ItemFlags lhs, ItemFlags rhs) {
	return (ItemFlags)((uint32_t)lhs | (uint32_t)rhs);
}

#endif