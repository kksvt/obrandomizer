#pragma once
#include <string>
#include <unordered_set>
#include <random>

#include "obse_headers.h"
#include "hook.h"

extern std::map<UInt32, std::vector<UInt32>> allWeapons;
extern std::map<UInt32, std::vector<UInt32>> allClothingAndArmor;
extern std::map<UInt32, std::vector<UInt32>> allGenericItems;
extern std::map<UInt32, std::vector<UInt32>> allSpellsBySchool;
extern std::vector<UInt32> allCreatures;
extern std::vector<UInt32> allItems;
extern std::vector<UInt32> allSpells;
extern std::unordered_map<MagicItem*, MagicItem*> spellMapping;;
extern std::unordered_set<UInt32> allAdded;

extern std::unordered_set<UInt32> allRandomized;

extern std::list<TESForm*> toRandomize;
extern std::map<TESObjectREFR*, UInt32> restoreFlags;

extern bool files_read;
extern TESForm* obrnFlag;

void fillUpClothingRanges();
void fillUpWpRanges();
void addOrAppend(ItemMapPtr map, const UInt32 key, UInt32 value);
void randomize(TESForm* form, const char* function);
void alterActorStats(Actor* actor, bool onlyActorAttributes, bool restore);
bool tryToAddForm(TESForm* f);
TESForm* getRandomByType(TESForm* f, bool keysAreQuestItems);
TESForm* getRandomBySetting(TESForm* f, int option, bool keysAreQuestItems);
void logDetailedListInfo();