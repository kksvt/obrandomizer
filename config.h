#pragma once
#include "rng.h"
#include "obse_headers.h"

enum OblivionCfgValueType {
	FV_NONE = 0,
	FV_UINT,
	FV_SINT8,
	FV_BOOL,
	FV_FLOAT,
};

struct OblivionCfgFieldsOffset {
	const char* name;
	SInt32 offset;
	OblivionCfgValueType type;
};

struct OblivionCfgSection {
	const char* name;
	const OblivionCfgFieldsOffset* arr;
};

class OblivionCfg {
private:
	bool oSeedRead;
	char seedFile[256];
public:
	OblivionCfg();
	bool ReadCfgFromFile(const char* name);
	bool ReadExcludesFromFile(const char* name);
	bool HasSeed();
	//with the current implementation, this doesnt make hell of a sense
	//perhaps one day this config will actually be well-written
	template <typename T>
	T GetSettingValueByOffset(SInt32 offset) {
		return *(T*)((SInt32)this + offset);
	}
	OblivionCfgValueType GetSettingType(const char* name, SInt32* offset);
	bool InitSeedRandomizationData(std::unordered_map<UInt32, UInt32>& allRandomized);
	bool WriteSeedRandomizationData(UInt32 from, UInt32 to);
	//[Misc]
	UInt32 oSeed;
	bool oSaveSeedData;
	bool oExcludeQuestItems;
	bool oDelayStart;
	SInt8 oInstallCrashFix;
	SInt8 oHitEffect;
	SInt8 oRandSpells;

	//[Loot]
	SInt8 oRandInventory;
	SInt8 oRandContainers;
	SInt8 oWorldItems;
	SInt8 oAddItems;
	SInt8 oDeathItems;
	bool oRandGold;
	bool oExcludeUnplayableItems;

	//[Actor]
	SInt8 oRandomizeAttrib;
	SInt8 oRandomizeStats;
	bool oRestoreBaseAttributes;
	SInt8 oVampire;
	bool oScaleActors;
	double oScaleMin;
	double oScaleMax;

	//[Creatures]
	SInt8 oRandCreatures;
	bool oUseEssentialCreatures;
	bool oSkipHorses;

	UInt8 randId;

	//skip
	bool skipMod[0xFF]; //forms from these mods will not be added to the lists
	bool skipRandMod[0xFF]; //forms from these mods will not be randomized
};

#define OBLIVIONCFGFIELDOFFSET(x) (SInt32)&((OblivionCfg*)0)->x
#define OBLIVIONCFGFIELD(x, type) {#x, OBLIVIONCFGFIELDOFFSET(x), type}