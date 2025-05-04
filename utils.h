#pragma once
#include "obse_headers.h"
#include "rng.h"
#include "config.h"

#include "ModTable.h"
#include "GameData.h"

#define SPELL_SKELETONKEY 0x000C45AA
#define SPELL_HEMOPHILIA 0x0003DB3D

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
	noTESContainer = 8,
};

#define TESFORM2STRING(x) #x

extern TESForm* obrnFlag;

bool modelExists(TESForm* f);
bool isQuestOrScriptedItem(TESForm* item, bool keysAreQuestItems);
bool creatureValid(const char* name, const char* model);
bool refIsItem(TESObjectREFR* ref);
bool itemIsEquippable(TESForm* item);
const char* FormTypeToString(int form);

bool getFormsFromLeveledList(TESLeveledList* list, std::vector<std::pair<TESForm*, int>>& itemList, UInt16 flag); //returns false if the rejectOnQuestItem flag is on and there's a quest item in the leveled list
std::pair<TESForm*, int> getRandomFormFromLeveledList(TESLeveledList* list, UInt16 flag);
bool getInventoryFromTESContainer(TESContainer* container, std::unordered_map<TESForm*, int>& itemList, UInt16 flag);
bool getContainerInventory(TESObjectREFR* ref, std::unordered_map<TESForm*, int>& itemList, UInt16 flag);
int getPlayerLevel();
int getRefLevelAdjusted(TESObjectREFR* ref);
bool spellBlacklisted(SpellItem* spell);

UInt8 GetModIndexShifted(const std::string& name);

extern OblivionRng rng;
extern OblivionCfg config;;