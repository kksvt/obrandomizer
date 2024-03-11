#pragma once
#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"

#if OBLIVION
#include "obse/GameAPI.h"

/*	As of 0020, ExtractArgsEx() and ExtractFormatStringArgs() are no longer directly included in plugin builds.
	They are available instead through the OBSEScriptInterface.
	To make it easier to update plugins to account for this, the following can be used.
	It requires that g_scriptInterface is assigned correctly when the plugin is first loaded.
*/
#define ENABLE_EXTRACT_ARGS_MACROS 1	// #define this as 0 if you prefer not to use this

#if ENABLE_EXTRACT_ARGS_MACROS

extern OBSEScriptInterface* g_scriptInterface;
#define ExtractArgsEx(...) g_scriptInterface->ExtractArgsEx(__VA_ARGS__)
#define ExtractFormatStringArgs(...) g_scriptInterface->ExtractFormatStringArgs(__VA_ARGS__)

#endif

#else
#include "obse_editor/EditorAPI.h"
#endif

#include "obse/ParamInfos.h"
#include "obse/Script.h"
#include "obse/GameObjects.h"
#include "obse/ModTable.h"
#include <string>
#include <unordered_set>
#include <random>

#include "hook.h"

#define ITEM_GOLD 0x0000000F
#define ITEM_REPAIRHAMMER 0x0000000C
#define ITEM_LOCKPICK 0x0000000A

struct LevListResult_t {
	struct LevListData_t {
		UInt32 count;
		TESForm* item;
	}*data;
	LevListResult_t* next;
};

enum ClothingRanges {
	UPPER,
	UPPERHAND,
	UPPERLOWER,
	UPPERLOWERFOOT,
	UPPERLOWERFOOTHAND,
	UPPERLOWERHAND,
	CRANGE_MAX
};

enum WeaponRanges {
	BLADES,
	BLUNT,
	STAVES,
	BOWS,
	UNARMED,
	WRANGE_MAX
};

enum {
	kSlot_Head = 0x1 << TESBipedModelForm::kPart_Head,
	kSlot_Hair = 0x1 << TESBipedModelForm::kPart_Hair,
	kSlot_UpperBody = 0x1 << TESBipedModelForm::kPart_UpperBody,
	kSlot_LowerBody = 0x1 << TESBipedModelForm::kPart_LowerBody,
	kSlot_Hand = 0x1 << TESBipedModelForm::kPart_Hand,
	kSlot_Foot = 0x1 << TESBipedModelForm::kPart_Foot,
	kSlot_RightRing = 0x1 << TESBipedModelForm::kPart_RightRing,
	kSlot_LeftRing = 0x1 << TESBipedModelForm::kPart_LeftRing,
	kSlot_Amulet = 0x1 << TESBipedModelForm::kPart_Amulet,
	kSlot_Weapon = 0x1 << TESBipedModelForm::kPart_Weapon,
	kSlot_BackWeapon = 0x1 << TESBipedModelForm::kPart_BackWeapon,
	kSlot_SideWeapon = 0x1 << TESBipedModelForm::kPart_SideWeapon,
	kSlot_Quiver = 0x1 << TESBipedModelForm::kPart_Quiver,
	kSlot_Shield = 0x1 << TESBipedModelForm::kPart_Shield,
	kSlot_Torch = 0x1 << TESBipedModelForm::kPart_Torch,
	kSlot_Tail = 0x1 << TESBipedModelForm::kPart_Tail,
	kSlot_UpperLower = kSlot_UpperBody | kSlot_LowerBody,
	kSlot_UpperLowerFoot = kSlot_UpperLower | kSlot_Foot,
	kSlot_UpperLowerHandFoot = kSlot_UpperLowerFoot | kSlot_Hand,
	kSlot_UpperLowerHand = kSlot_UpperLower | kSlot_Hand,
	kSlot_BothRings = kSlot_RightRing | kSlot_LeftRing,
	kSlot_UpperHand = kSlot_UpperBody | kSlot_Hand,

	kSlot_None = 0,
};

enum RandClothes {
	OBRNRC_NONE = 0,
	OBRNRC_LOWER = 1,
	OBRNRC_FOOT = 2,
	OBRNRC_HAND = 4,
	OBRNRC_UPPER = 8,
};

enum ItemRetrieval {
	none = 0,
	all = 1,
	noQuestItems = 2,
	rejectOnQuestItem = 3,
	noAccumulation = 4,
};

#define TESFORM2STRING(x) #x

typedef std::map<UInt32, std::vector<UInt32>>* ItemMapPtr;

extern int oRandCreatures;
extern int oAddItems;
extern int oDeathItems;
extern int oWorldItems;
extern int oRandInventory;
extern int oRandSpells;

extern std::map<UInt32, std::vector<UInt32>> allWeapons;
extern std::map<UInt32, std::vector<UInt32>> allClothingAndArmor;
extern std::map<UInt32, std::vector<UInt32>> allGenericItems;
extern std::map<UInt32, std::vector<UInt32>> allSpellsBySchool;
extern std::vector<UInt32> allCreatures;
extern std::vector<UInt32> allItems;
extern std::vector<UInt32> allSpells;
extern std::unordered_set<UInt32> allAdded;

extern std::unordered_set<UInt32> allRandomized;

extern std::list<TESObjectREFR*> toRandomize;
extern std::map<TESObjectREFR*, UInt32> restoreFlags;

extern bool files_read;
extern TESForm* obrnFlag;

void InitModExcludes();
void fillUpClothingRanges();
void fillUpWpRanges();
void addOrAppend(ItemMapPtr map, const UInt32 key, UInt32 value);
void randomize(TESObjectREFR* ref, const char* function);
bool tryToAddForm(TESForm* f);
TESForm* getRandomByType(TESForm* f);
TESForm* getRandomBySetting(TESForm* f, int option);
const char* formTypeToString(int form);
void InitConfig();
bool refIsItem(TESObjectREFR* ref);
void randomizeInventory(TESObjectREFR* ref);
bool getContainerInventory(TESObjectREFR* ref, std::unordered_map<TESForm*, int>& itemList, UInt16 flag);
UInt32 rng(UInt32 a, UInt32 b);
void debugDumpSpells(TESForm* form);