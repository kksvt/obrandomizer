#include "randomizer.h"
#include "utils.h"

/*#define FORMMAPADDR 0x00B0613C

NiTMapBase<unsigned int, TESForm*>* allObjects = (NiTMapBase<unsigned int, TESForm*>*)FORMMAPADDR;
*/

std::map<UInt32, std::vector<UInt32>> allWeapons;
std::map<UInt32, std::vector<UInt32>> allClothingAndArmor;
std::map<UInt32, std::vector<UInt32>> allGenericItems;
std::map<UInt32, std::vector<UInt32>> allSpellsBySchool;
std::vector<UInt32> allCreatures;
std::vector<UInt32> allItems;
std::vector<UInt32> allSpells;
std::unordered_map<MagicItem*, MagicItem*> spellMapping;
std::unordered_set<UInt32> allAdded;

std::unordered_set<UInt32> allRandomized;

std::list<TESForm*> toRandomize;
std::map<TESObjectREFR*, UInt32> restoreFlags;

TESForm* obrnFlag = NULL;

int clothingRanges[CRANGE_MAX];
int weaponRanges[WRANGE_MAX];

int clothingKeys[] = { kSlot_UpperBody, kSlot_UpperHand, kSlot_UpperLower, kSlot_UpperLowerFoot, kSlot_UpperLowerHandFoot, kSlot_UpperLowerHand, -1 };
void fillUpClothingRanges() {
	for (int i = 0; i < CRANGE_MAX; ++i) {
		clothingRanges[i] = 0;
	}
	for (int* k = &clothingKeys[0], i = -1; *k != -1; ++k) {
		auto it = allClothingAndArmor.find(*k);//allClothingAndArmor.find(TESBipedModelForm::SlotForMask(*k));
		if (it == allClothingAndArmor.end()) {
			continue;
		}
		clothingRanges[++i] = it->second.size();
	}
	for (int i = 1; i < CRANGE_MAX; ++i) {
		clothingRanges[i] += clothingRanges[i - 1];
	}
}

void fillUpWpRanges() {
	int wpKeys[] = { TESObjectWEAP::kType_BladeOneHand, TESObjectWEAP::kType_BluntOneHand, TESObjectWEAP::kType_Staff, TESObjectWEAP::kType_Bow, -1 };
	int maxUnarmed = 0;
	for (int i = 0; i < WRANGE_MAX; ++i) {
		weaponRanges[i] = 0;
	}
	for (int* k = &wpKeys[0], i = -1; *k != -1; ++k) {
		auto it = allWeapons.find(*k);
		if (it == allWeapons.end()) {
			continue;
		}
		weaponRanges[++i] = it->second.size();
	}
	weaponRanges[UNARMED] = weaponRanges[BOWS] * (10.0 / 9.0);
	for (int i = 1; i < WRANGE_MAX; ++i) {
		weaponRanges[i] += weaponRanges[i - 1];
	}
}

TESForm* getRandomForKey(ItemMapPtr map, const UInt32 key) {
	auto it = map->find(key);
	if (it == map->end() || !it->second.size()) {
		const char* mapName = "generic";
		if (map == &allWeapons) {
			mapName = "weapons";
		}
		else if (map == &allClothingAndArmor) {
			mapName = "clothing & armor";
		}
		else if (map == &allSpellsBySchool) {
			mapName = "spells by school";
		}
		_ERROR("Couldn't find key %u for map %s", key, mapName);
		return NULL;
	}
	return LookupFormByID(it->second[rng(0, it->second.size() - 1)]);
}

void addOrAppend(ItemMapPtr map, const UInt32 key, UInt32 value) {
	allAdded.insert(value);
	auto it = map->find(key);
	if (it == map->end()) {
		std::vector<UInt32> itemList;
		itemList.push_back(value);
		map->insert(std::make_pair(key, itemList));
	}
	else {
		it->second.push_back(value);
	}
}

bool tryToAddForm(TESForm* f) {
	ItemMapPtr ptr = NULL;
	UInt32 key = 0xFFFFFFFF;
	const char* name = GetFullName(f);
	if (name[0] == '<') {
		return false;
	}
	if (!obrnFlag && f->GetModIndex() == config.randId && f->GetFormType() == kFormType_Misc && strcmp(name, "You should not see this") == 0) {
		obrnFlag = f;
		_MESSAGE("OBRN Flag found as %08X", f->refID);
		return false;
	}
	if (f->GetModIndex() == 0xFF || config.skipMod[f->GetModIndex()]) {
		return false;
	}
	if (f->GetFormType() != kFormType_Creature && isQuestOrScriptedItem(f, false) && config.oExcludeQuestItems) {
		return false;
	}
	if (allAdded.find(f->refID) != allAdded.end()) {
		return false;
	}
	if (strncmp(name, "aaa", 3) == 0) {
		//exception for some test objects that typically don't even have a working model
		return false;
	}
	switch (f->GetFormType()) {
	case kFormType_Creature:
	{
		TESCreature* critter = OBLIVION_CAST(f, TESForm, TESCreature);
		TESScriptableForm* scriptForm = OBLIVION_CAST(f, TESForm, TESScriptableForm);
		if (scriptForm && critter) {
			if ((config.oUseEssentialCreatures || (!critter->actorBaseData.IsEssential()) && 
				(!config.oExcludeQuestItems || !scriptForm->script))) {
				const char* model = critter->modelList.modelList.Info();
				if (!model) {
					model = "";
				}
				//hardcoded exception for SI grummites without a working model + excluding some test creatures
				if ((!config.oSkipHorses || critter->type != TESCreature::eCreatureType_Horse)
					&& creatureValid(name, model)) {
					allCreatures.push_back(f->refID);
					allAdded.insert(f->refID);
					return true;
				}
			}
		}
#ifdef DEBUG
		_MESSAGE("Skipping %08X (%s)", f->refID, name);
#endif
		break;
	}
	case kFormType_Armor:
	case kFormType_Clothing:
	{
		ptr = &allClothingAndArmor;
		if (f->GetFormType() == kFormType_Armor) {
			TESObjectARMO* armor = OBLIVION_CAST(f, TESForm, TESObjectARMO);
			key = armor->bipedModel.partMask & ~kSlot_Tail; //yeah... we kinda dont care about this slot
		}
		else {
			TESObjectCLOT* clothing = OBLIVION_CAST(f, TESForm, TESObjectCLOT);
			key = clothing->bipedModel.partMask;
		}
		if (key & kSlot_RightRing) {
			key = kSlot_LeftRing;
		}
		if (key & kSlot_Hair || key & kSlot_Head) {
			key = kSlot_Head;
		}
		break;
	}
	case kFormType_Weapon:
	{
		ptr = &allWeapons;
		TESObjectWEAP* weapon = OBLIVION_CAST(f, TESForm, TESObjectWEAP);
		key = weapon->type;
		if (key == TESObjectWEAP::kType_BladeTwoHand) {
			key = TESObjectWEAP::kType_BladeOneHand;
		}
		else if (key == TESObjectWEAP::kType_BluntTwoHand) {
			key = TESObjectWEAP::kType_BluntOneHand;
		}
		break;
	}
	case kFormType_Apparatus:
	case kFormType_Book:
	case kFormType_Ingredient:
	case kFormType_Misc:
	case kFormType_Ammo:
	case kFormType_SoulGem:
	case kFormType_Key:
	case kFormType_AlchemyItem:
	case kFormType_SigilStone:
	{
		ptr = &allGenericItems;
		key = f->GetFormType();
		break;
	}
	case kFormType_Spell:
	{
		SpellItem* spell = OBLIVION_CAST(f, TESForm, SpellItem);
		if (!spellBlacklisted(spell)) {		
			ptr = &allSpellsBySchool;
			key = spell->GetSchool();
			allSpells.push_back(f->refID);
		}
		break;
	}
	default:
		break;
	}
	if (ptr && key != 0xFFFFFFFF) {
		if (modelExists(f)) {
			addOrAppend(ptr, key, f->refID);
			return true;
		}
	}
	return false;
}

TESForm* getRandom(TESForm* f, bool keysAreQuestItems) {
	std::vector<UInt32>* ptr = NULL;
	if (!f) {
		return NULL;
	}
	if (config.oExcludeQuestItems && isQuestOrScriptedItem(f, keysAreQuestItems)) {
		return NULL;
	}
	if (f->GetModIndex() != 0xFF && config.skipRandMod[f->GetModIndex()]) { //very important!
#ifdef _DEBUG
		_MESSAGE("Item %s %08X will not be randomized because it belongs to a forbidden mod", GetFullName(f), f->refID);
#endif
		return NULL;
	}
	if (f->refID == ITEM_GOLD && !config.oRandGold) {
		return NULL;
	}
	switch (f->GetFormType()) {
		case kFormType_LeveledSpell:
		case kFormType_Spell:
			if (!allSpells.size()) {
				return NULL;
			}
			ptr = &allSpells;
			break;
		default:
			if (!allItems.size()) {
				return NULL;
			}
			ptr = &allItems;
			break;
	}
	if (ptr) {
		return LookupFormByID((*ptr)[rng(0, ptr->size() - 1)]);
	}
	return NULL;
}

TESForm* getRandomByType(TESForm *f, bool keysAreQuestItems) {
	ItemMapPtr ptr = NULL;
	UInt32 key = 0xFFFFFFFF;
	if (!f) {
		return NULL;
	}
	if (config.oExcludeQuestItems && isQuestOrScriptedItem(f, keysAreQuestItems)) {
		return NULL;
	}
	if (f->GetModIndex() != 0xFF && config.skipRandMod[f->GetModIndex()]) { //very important!
#ifdef _DEBUG
		_MESSAGE("Item %s %08X will not be randomized because it belongs to a forbidden mod", GetFullName(f), f->refID);
#endif
		return NULL;
	}
	if (f->refID == ITEM_GOLD && !config.oRandGold) {
#ifdef _DEBUG
		_MESSAGE("ITEM_GOLD: returning null because oRandGold == 0");
#endif
		return NULL;
	}
	switch (f->GetFormType()) {
		case kFormType_LeveledItem:
		{
			TESLevItem* lev = OBLIVION_CAST(f, TESForm, TESLevItem);
			return getRandomByType(getRandomFormFromLeveledList(&lev->leveledList, ItemRetrieval::rejectOnQuestItem).first, keysAreQuestItems);
		}
		case kFormType_LeveledSpell:
		{
			TESLevSpell* lev = OBLIVION_CAST(f, TESForm, TESLevSpell);
			return getRandomByType(getRandomFormFromLeveledList(&lev->leveledList, ItemRetrieval::rejectOnQuestItem).first, keysAreQuestItems);
		}
		case kFormType_Armor:
		case kFormType_Clothing:
		{
			ptr = &allClothingAndArmor;
			if (f->GetFormType() == kFormType_Armor) {
				TESObjectARMO* armor = OBLIVION_CAST(f, TESForm, TESObjectARMO);
				key = armor->bipedModel.partMask & ~kSlot_Tail; //yeah... we kinda dont care about this slot
			}
			else {
				TESObjectCLOT* clothing = OBLIVION_CAST(f, TESForm, TESObjectCLOT);
				key = clothing->bipedModel.partMask;
			}
			if (key & kSlot_RightRing) {
				key = kSlot_LeftRing;
			}
			if (key & kSlot_Hair || key & kSlot_Head) {
				key = kSlot_Head;
			}
			break;
		}
		case kFormType_Weapon:
		{
			ptr = &allWeapons;
			TESObjectWEAP* weapon = OBLIVION_CAST(f, TESForm, TESObjectWEAP);
			key = weapon->type;
			if (key == TESObjectWEAP::kType_BladeTwoHand) {
				key = TESObjectWEAP::kType_BladeOneHand;
			}
			else if (key == TESObjectWEAP::kType_BluntTwoHand) {
				key = TESObjectWEAP::kType_BluntOneHand;
			}
			break;
		}
		case kFormType_Apparatus:
		case kFormType_Book:
		case kFormType_Ingredient:
		case kFormType_Misc:
		case kFormType_Ammo:
		case kFormType_SoulGem:
		case kFormType_Key:
		case kFormType_AlchemyItem:
		case kFormType_SigilStone:
			ptr = &allGenericItems;
			key = f->GetFormType();
			break;
		case kFormType_Spell:
		{
			SpellItem* spell = OBLIVION_CAST(f, TESForm, SpellItem);
			if (!spellBlacklisted(spell)) {
				ptr = &allSpellsBySchool;
				key = spell->GetSchool();
			}
			break;
		}
		default:
			break;
	}
	if (ptr && key != 0xFFFFFFFF) {
		return getRandomForKey(ptr, key);
	}
	return NULL;
}

TESForm* getRandomBySetting(TESForm* f, int option, bool keysAreQuestItems) {
	switch (option) {
	case 0:
		return NULL;
	case 1:
		return getRandomByType(f, keysAreQuestItems);
	case 2:
		return getRandom(f, keysAreQuestItems);
	default:
		_MESSAGE("Invalid option %i for getRandomBySetting", option);
		return NULL;
	}
}

static void randomizeInventory(TESObjectREFR* ref) {
	if (allGenericItems.size() == 0 || allWeapons.size() == 0 || allClothingAndArmor.size() == 0) {
		return;
	}
	int randSetting = ref->GetFormType() == kFormType_ACHR ? config.oRandInventory : config.oRandContainers;
	std::unordered_map<TESForm*, int> removeItems;
	getContainerInventory(ref, removeItems, config.oExcludeQuestItems ? ItemRetrieval::noQuestItems : ItemRetrieval::all);
	for (const auto& it : removeItems) {
		TESForm* item = it.first;
		int cnt = it.second;
		if (randSetting == 1) {
			switch (item->refID) {
			case ITEM_LOCKPICK:
			case ITEM_REPAIRHAMMER:
				ref->AddItem(item, NULL, rng(1, 4) * cnt);
				break;
			case ITEM_GOLD:
				if (!config.oRandGold) {
					ref->AddItem(item, NULL, rng(5, 60) * getRefLevelAdjusted(ref) - cnt);
					break;
				}
			default:
			{
				if (TESForm* newItem = getRandomByType(item, false)) {
#ifdef _DEBUG
					_MESSAGE("Replacing item %s %08X with %s %08X x%i", GetFullName(item), item->refID, GetFullName(newItem), newItem->refID, cnt);
#endif
					ref->AddItem(newItem, NULL, cnt);
					if (ref->GetFormType() == kFormType_ACHR && itemIsEquippable(newItem)) {
						ref->Equip(newItem, 1, NULL, 0);
					}
				}
			}
			}
		}
		if ((item->refID != ITEM_GOLD || config.oRandGold) &&
			item->GetFormType() != kFormType_Book &&
			item->GetFormType() != kFormType_Key &&
			item->GetModIndex() != 0xFF && !config.skipRandMod[item->GetModIndex()] &&
			(!config.oExcludeQuestItems || !isQuestOrScriptedItem(item, false))) {
			ref->RemoveItem(item, NULL, cnt, 0, 0, NULL, NULL, NULL, 0, 0);
		}
	}
	removeItems.clear();
	if (randSetting == 1) {
		return;
	}
	//granting random items
	TESForm* selection = NULL;
	if (ref->GetFormType() == kFormType_ACHR) {
		int roll = rng(0, 99), level = getRefLevelAdjusted(ref);
		//Gold
		if (roll < 50) {
#ifdef _DEBUG
			_MESSAGE("GOLD: %s receiving gold", GetFullName(ref));
#endif
			ref->AddItem(LookupFormByID(ITEM_GOLD), NULL, rng(5, 60) * level);
		}
		//Weapon Randomization
		int clothingStatus = OBRNRC_LOWER | OBRNRC_FOOT | OBRNRC_HAND | OBRNRC_UPPER;
		bool gotTwoHanded = false;
		bool gotBow = false;
		for (int i = 0; i < 2; ++i) {
			roll = rng(0, weaponRanges[UNARMED]);
			int v;
			for (v = 0; v < WRANGE_MAX; ++v) {
				if (roll < weaponRanges[v]) {
					break;
				}
			}
			switch (v) {
			case BLADES:
				if (selection = getRandomForKey(&allWeapons, TESObjectWEAP::kType_BladeOneHand)) {
					TESObjectWEAP* wp = OBLIVION_CAST(selection, TESForm, TESObjectWEAP);
#ifdef _DEBUG
					_MESSAGE("BLADE: %s receiving %s", GetFullName(ref), GetFullName(wp));
#endif
					ref->AddItem(wp, NULL, 1);
					if (!i) {
						ref->Equip(wp, 1, NULL, 0);
					}
					if (wp->type == TESObjectWEAP::kType_BladeTwoHand) {
						gotTwoHanded = true;
					}
				}
				break;
			case BLUNT:
				if (selection = getRandomForKey(&allWeapons, TESObjectWEAP::kType_BluntOneHand)) {
					TESObjectWEAP* wp = OBLIVION_CAST(selection, TESForm, TESObjectWEAP);
#ifdef _DEBUG
					_MESSAGE("BLUNT: %s receiving %s", GetFullName(ref), GetFullName(wp));
#endif
					ref->AddItem(wp, NULL, 1);
					if (!i) {
						ref->Equip(wp, 1, NULL, 0);
					}
					if (wp->type == TESObjectWEAP::kType_BluntTwoHand) {
						gotTwoHanded = true;
					}
				}
				break;
			case STAVES:
				if (selection = getRandomForKey(&allWeapons, TESObjectWEAP::kType_Staff)) {
					TESObjectWEAP* wp = OBLIVION_CAST(selection, TESForm, TESObjectWEAP);
#ifdef _DEBUG
					_MESSAGE("STAFF: %s receiving %s", GetFullName(ref), GetFullName(wp));
#endif
					ref->AddItem(wp, NULL, 1);
					if (!i) {
						ref->Equip(wp, 1, NULL, 0);
					}
					gotTwoHanded = true;
				}
				break;
			case BOWS:
				if (selection = getRandomForKey(&allWeapons, TESObjectWEAP::kType_Bow)) {
					TESObjectWEAP* wp = OBLIVION_CAST(selection, TESForm, TESObjectWEAP);
#ifdef _DEBUG
					_MESSAGE("BOW: %s receiving %s", GetFullName(ref), GetFullName(wp));
#endif
					ref->AddItem(wp, NULL, 1);
					if (!i) {
						ref->Equip(wp, 1, NULL, 0);
					}
					gotTwoHanded = true;
					gotBow = true;
				}
				break;
			default:
#ifdef _DEBUG
				_MESSAGE("UNARMED: %s is unarmed", GetFullName(ref));
#endif
				break;
			}
		}
		if (!gotBow) {
			if (!rng(0, 5)) {
				if (selection = getRandomForKey(&allWeapons, TESObjectWEAP::kType_Bow)) {
					TESObjectWEAP* wp = OBLIVION_CAST(selection, TESForm, TESObjectWEAP);
#ifdef _DEBUG
					_MESSAGE("BOW: %s receiving %s", GetFullName(ref), GetFullName(wp));
#endif
					ref->AddItem(wp, NULL, 1);
					ref->Equip(wp, 1, NULL, 0);
					gotTwoHanded = true;
					gotBow = true;
				}
			}
		}
		if (gotBow) {
			//give arrows
			for (int i = 0; i < 10; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Ammo)) {
#ifdef _DEBUG
					_MESSAGE("ARROW: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
					ref->AddItem(selection, NULL, rng(5, 30));
					if (!i) {
						ref->Equip(selection, 1, NULL, 0);
					}
					if (rng(0, i + 2)) {
						break;
					}
				}
			}
		}
		//Shield
		if (!gotTwoHanded && !rng(0, 2) && (selection = getRandomForKey(&allClothingAndArmor, kSlot_Shield))) {
#ifdef _DEBUG
			_MESSAGE("SHIELD: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
			ref->AddItem(selection, NULL, 1);
			ref->Equip(selection, 1, NULL, 0);
		}
		//Clothing
		if (!rng(0, 19)) {
			clothingStatus &= ~OBRNRC_UPPER;
		}
		if (!rng(0, 15)) {
			clothingStatus &= ~OBRNRC_LOWER;
		}
		if (!rng(0, 13)) {
			clothingStatus &= ~OBRNRC_FOOT;
		}
		if (!rng(0, 9)) {
			clothingStatus &= ~OBRNRC_HAND;
		}
		if (clothingStatus & OBRNRC_UPPER) {
			roll = rng(0, clothingRanges[UPPERLOWERHAND]);
			int v;
			for (v = 0; v < CRANGE_MAX; ++v) {
				if (roll < clothingRanges[v]) {
					break;
				}
			}
			switch (v) {
			case UPPER:
				break;
			case UPPERHAND:
				clothingStatus &= ~OBRNRC_HAND;
				break;
			case UPPERLOWER:
				clothingStatus &= ~OBRNRC_LOWER;
				break;
			case UPPERLOWERFOOT:
				clothingStatus &= ~(OBRNRC_LOWER | OBRNRC_FOOT);
				break;
			case UPPERLOWERFOOTHAND:
				clothingStatus &= ~(OBRNRC_LOWER | OBRNRC_FOOT | OBRNRC_HAND);
				break;
			case UPPERLOWERHAND:
				clothingStatus &= ~(OBRNRC_LOWER | OBRNRC_HAND);
				break;
			default:
				break;
			}
			if (v != CRANGE_MAX) {
				if (selection = getRandomForKey(&allClothingAndArmor, clothingKeys[v])) {
#ifdef _DEBUG
					_MESSAGE("UPPER: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
					ref->AddItem(selection, NULL, 1);
					ref->Equip(selection, 1, NULL, 0);
				}
			}
			else {
#ifdef _DEBUG
				_ERROR("v == CRANGE_MAX. this should not happen");
#endif
			}
		}
		if (clothingStatus & OBRNRC_LOWER) {
			if (selection = getRandomForKey(&allClothingAndArmor, kSlot_LowerBody)) {
#ifdef _DEBUG
				_MESSAGE("LOWER: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, 1);
				ref->Equip(selection, 1, NULL, 0);
			}
		}
		if (clothingStatus & OBRNRC_FOOT) {
			if (selection = getRandomForKey(&allClothingAndArmor, kSlot_Foot)) {
#ifdef _DEBUG
				_MESSAGE("FOOT: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, 1);
				ref->Equip(selection, 1, NULL, 0);
			}
		}
		if (clothingStatus & OBRNRC_HAND) {
			if (selection = getRandomForKey(&allClothingAndArmor, kSlot_Hand)) {
#ifdef _DEBUG
				_MESSAGE("HAND: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, 1);
				ref->Equip(selection, 1, NULL, 0);
			}
		}
		//Head
		if (rng(0, 2)) {
			if (selection = getRandomForKey(&allClothingAndArmor, kSlot_Head)) {
#ifdef _DEBUG
				_MESSAGE("HEAD: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, 1);
				ref->Equip(selection, 1, NULL, 0);
			}
		}
		//Rings
		for (int i = 0; i < 2; ++i) {
			if (!rng(0, i + 1)) {
				if (selection = getRandomForKey(&allClothingAndArmor, kSlot_LeftRing)) {
#ifdef _DEBUG
					_MESSAGE("RING: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
					ref->AddItem(selection, NULL, 1);
					ref->Equip(selection, 1, NULL, 0);
				}
			}
		}
		//Amulets
		if (!rng(0, 2)) {
			if (selection = getRandomForKey(&allClothingAndArmor, kSlot_Amulet)) {
#ifdef _DEBUG
				_MESSAGE("AMULET: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, 1);
				ref->Equip(selection, 1, NULL, 0);
			}
		}
		//Potions
		for (int i = 0; i < 5; ++i) {
			if (rng(0, 2)) {
				break;
			}
			if (selection = getRandomForKey(&allGenericItems, kFormType_AlchemyItem)) {
#ifdef _DEBUG
				_MESSAGE("POTION: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, rng(1, 4));
			}
		}
		//Soul Gems
		for (int i = 0; i < 6; ++i) {
			if (rng(0, 4)) {
				break;
			}
			if (selection = getRandomForKey(&allGenericItems, kFormType_SoulGem)) {
#ifdef _DEBUG
				_MESSAGE("SOULGEM: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, rng(1, 2));
			}
		}
		//Sigil Stones
		if (!rng(0, 9)) {
			if (selection = getRandomForKey(&allGenericItems, kFormType_SigilStone)) {
#ifdef _DEBUG
				_MESSAGE("SIGILSTONE: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
				ref->AddItem(selection, NULL, 1);
			}
		}
		//Ingredients
		if (!rng(0, 5)) {
			for (int i = 0; i < 10; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Ingredient)) {
#ifdef _DEBUG
					_MESSAGE("INGREDIENT: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
					ref->AddItem(selection, NULL, rng(1, 6));
				}
				if (rng(0, 3)) {
					break;
				}
			}
		}
		//Apparatus
		if (!rng(0, 9)) {
			for (int i = 0; i < 3; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Apparatus)) {
#ifdef _DEBUG
					_MESSAGE("APPARATUS: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
					ref->AddItem(selection, NULL, rng(1, 2));
				}
				if (rng(0, 2)) {
					break;
				}
			}
		}
		//Clutter
		if (!rng(0, 9)) {
			for (int i = 0; i < 9; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Misc)) {
#ifdef _DEBUG
					_MESSAGE("GENERIC: %s receiving %s", GetFullName(ref), GetFullName(selection));
#endif
					ref->AddItem(selection, NULL, rng(1, 3));
				}
				if (rng(0, i + 1)) {
					break;
				}
			}
		}
	}
	else {//containers
		//Gold
		if (!rng(0, 2)) {
			ref->AddItem(LookupFormByID(ITEM_GOLD), NULL, rng(1, 100) * (!rng(0, 4) ? 30 : 1));
		}
		//Lockpick
		if (!rng(0, 4)) {
			ref->AddItem(LookupFormByID(ITEM_LOCKPICK), NULL, rng(1, 10));
		}
		//Repair Hammer
		if (!rng(0, 4)) {
			ref->AddItem(LookupFormByID(ITEM_REPAIRHAMMER), NULL, rng(1, 3));
		}
		//Clutter
		if (!rng(0, 5)) {
			for (int i = 0; i < 3; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Misc)) {
					ref->AddItem(selection, NULL, rng(1, 5));
					if (rng(0, i + 3)) {
						break;
					}
				}
			}
		}
		//Potions
		if (!rng(0, 3)) {
			for (int i = 0; i < 5; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_AlchemyItem)) {
					ref->AddItem(selection, NULL, rng(1, 5));
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Ingredients
		if (!rng(0, 4)) {
			for (int i = 0; i < 6; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Ingredient)) {
					ref->AddItem(selection, NULL, rng(1, 2));
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Books
		if (!rng(0, 4)) {
			for (int i = 0; i < 3; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Book)) {
					ref->AddItem(selection, NULL, 1);
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Soul Gems
		if (!rng(0, 9)) {
			for (int i = 0; i < 3; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_SoulGem)) {
					ref->AddItem(selection, NULL, rng(1, 3));
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Sigil Stones
		if (!rng(0, 11)) {
			for (int i = 0; i < 2; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_SigilStone)) {
					ref->AddItem(selection, NULL, 1);
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Keys
		if (!rng(0, 6)) {
			if (selection = getRandomForKey(&allGenericItems, kFormType_Key)) {
				ref->AddItem(selection, NULL, 1);
			}
		}
		//Apparatus
		if (!rng(0, 8)) {
			for (int i = 0; i < 4; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Apparatus)) {
					ref->AddItem(selection, NULL, 1);
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Weapons
		if (!rng(0, 4)) {
			for (int i = 0; i < 3; ++i) {
				int wpType = -1;
				switch (rng(0, 3)) {
				case 0:
					wpType = TESObjectWEAP::kType_BladeOneHand;
					break;
				case 1:
					wpType = TESObjectWEAP::kType_BluntOneHand;
					break;
				case 2:
					wpType = TESObjectWEAP::kType_Bow;
					break;
				default:
					wpType = TESObjectWEAP::kType_Staff;
					break;
				}
				if (selection = getRandomForKey(&allWeapons, wpType)) {
					ref->AddItem(selection, NULL, 1);
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Ammo
		if (!rng(0, 4)) {
			for (int i = 0; i < 4; ++i) {
				if (selection = getRandomForKey(&allGenericItems, kFormType_Ammo)) {
					ref->AddItem(selection, NULL, rng(1, 30));
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Clothing / Armor
		if (!rng(0, 2)) {
			for (int i = 0; i < 7; ++i) {
				UInt32 key = 0xFFFFFFFF;
				switch (rng(0, 12)) { //was 0, 7 in the script
				case 0:
					key = kSlot_Amulet;
					break;
				case 1:
					key = kSlot_Foot;
					break;
				case 2:
					key = kSlot_Hand;
					break;
				case 3:
					key = kSlot_Head;
					break;
				case 4:
					key = kSlot_LowerBody;
					break;
				case 5:
					key = kSlot_LeftRing;
					break;
				case 6:
					key = kSlot_Shield;
					break;
				case 7:
					key = kSlot_UpperBody;
					break;
				case 8:
					key = kSlot_UpperHand;
					break;
				case 9:
					key = kSlot_UpperLower;
					break;
				case 10:
					key = kSlot_UpperLowerFoot;
					break;
				case 11:
					key = kSlot_UpperLowerHandFoot;
					break;
				default:
					key = kSlot_UpperLowerHand;
					break;
				}
				if (key != 0xFFFFFFFF && (selection = getRandomForKey(&allClothingAndArmor, key))) {
					ref->AddItem(selection, NULL, 1);
					if (rng(0, i + 1)) {
						break;
					}
				}
			}
		}
	}
}

static void randomizeCreature(TESObjectREFR* ref, const char* function) {
	if (allCreatures.size() == 0) {
		return;
	}
	if (strcmp(function, "ESP") == 0) {
		QueueUIMessage_2("Randomizing creatures through the spell may cause issues", 3.0f, NULL, NULL);
	}
	else if (!config.oRandCreatures || (ref->GetModIndex() != 0xFF && config.skipRandMod[ref->GetModIndex()])) {
		return;
	}
	TESCreature* critter = OBLIVION_CAST(ref->baseForm, TESForm, TESCreature);
	if (!critter) {//shouldn't happen, but you never know with oblivion
		return;
	}
	if (config.oSkipHorses && critter->type == TESCreature::eCreatureType_Horse) {
#ifdef _DEBUG
		_MESSAGE("...skipping because its a horse.");
#endif
		return;
	}
	Actor* actor = OBLIVION_CAST(ref, TESObjectREFR, Actor);
	if (!actor) {
		return;
	}
	UInt32 health = actor->GetBaseActorValue(kActorVal_Health), aggression = actor->GetActorValue(kActorVal_Aggression);
	if (health == 0) {
		return;
	}
	std::unordered_map<TESForm*, int> keepItems;
	getContainerInventory(ref, keepItems, (ItemRetrieval::all | ItemRetrieval::noAccumulation));
	TESForm* oldBaseForm = ref->GetTemplateForm() ? ref->GetTemplateForm() : ref->baseForm,
		* rando = LookupFormByID(allCreatures[rng(0, allCreatures.size() - 1)]);
#ifdef _DEBUG
	_MESSAGE("%s: Going to randomize %s %08X into %s %08X", function, GetFullName(ref), ref->refID, GetFullName(rando), rando->refID);
#endif
	//TESActorBase* actorBase = OBLIVION_CAST(rando, TESForm, TESActorBase);
	ref->baseForm = rando;
	ref->SetTemplateForm(oldBaseForm);
	ForceActorValue(actor, kActorVal_Aggression, aggression);
	for (const auto& it : keepItems) {
		TESForm* item = it.first;
		int cnt = it.second;
		ref->AddItem(item, NULL, cnt);
	}
	ref->Update3D();
}

static void randomizeWorldItem(TESObjectREFR* ref, const char* function) {
	if (!config.oWorldItems || (ref->GetModIndex() != 0xFF && config.skipRandMod[ref->GetModIndex()])) {
		return;
	}
#ifdef _DEBUG
	_MESSAGE("%s: World item randomization: will try to randomize %s %08X", function, GetFullName(ref), ref->refID);
#endif
	if (TESForm* rando = getRandomBySetting(ref->baseForm, config.oWorldItems, false)) {
#ifdef _DEBUG
		_MESSAGE("%s: Going to randomize %s %08X into %s %08X", function, GetFullName(ref), ref->refID, GetFullName(rando), rando->refID);
#endif
		ref->baseForm = rando;
		ref->Update3D();
	}
}

static void randomizeSpell(TESForm* spell, const char* function) {
	MagicItem* magicItem = OBLIVION_CAST(spell, TESForm, MagicItem);
	if (!magicItem) {
		_ERROR("%s: could not convert spell %s (%08X %s) to MagicItem",
			function, GetFullName(spell), spell->refID, FormTypeToString(spell->GetFormType()));
		return;
	}

	TESForm* rando = getRandomBySetting(spell, config.oRandSpells, false);
	if (!rando) {
		return;
	}

	MagicItem* randoMagicItem = OBLIVION_CAST(rando, TESForm, MagicItem);
	if (!randoMagicItem) {
		_ERROR("%s: could not convert spell %s (%08X %s) to MagicItem",
			function, GetFullName(rando), rando->refID, FormTypeToString(rando->GetFormType()));
		return;
	}

	spellMapping.insert(std::make_pair(magicItem, randoMagicItem));
#ifdef _DEBUG
	_MESSAGE("%s: spell %s (%08X) has been randomized into %s (%08X).",
		function, GetFullName(spell), spell->refID, GetFullName(rando), rando->refID);
#endif
}

void randomize(TESForm* form, const char* function) {
	if (allRandomized.contains(form->refID) && strcmp(function, "ESP")) {
#ifdef _DEBUG
		_MESSAGE("%s: ref %s %08X has already been randomized.",
			function, GetFullName(form), form->refID);
#endif
		return;
	}
	allRandomized.insert(form->refID);
#ifdef _DEBUG
	_MESSAGE(__FUNCTION__": Attempting to randomize %s (%08X %s)",
		GetFullName(form), form->refID, FormTypeToString(form->GetFormType()));
#endif
	if (form->GetFormType() == kFormType_Spell) {
		randomizeSpell(form, function);
		return;
	}
	TESObjectREFR* ref = OBLIVION_CAST(form, TESForm, TESObjectREFR);
	if (!ref) {
		_ERROR("%s: could not convert form %s (%08X %s) to a reference.",
			function, GetFullName(form), form->refID, FormTypeToString(form->GetFormType()));
		return;
	}
	if (ref->GetFormType() == kFormType_ACRE) {
		randomizeCreature(ref, function);
		return;
	}
	if (refIsItem(ref)) {
		randomizeWorldItem(ref, function);
		return;
	}
	randomizeInventory(ref);
}

void alterActorStats(Actor* actor, bool stats, bool restore) {
	UInt32 start, end;
	if (restore) {
		start = kActorVal_Strength;
		end = kActorVal_Responsibility;
	}
	else if (stats) {
		start = kActorVal_Strength;
		end = kActorVal_Speechcraft;
	}
	else {
		start = kActorVal_Aggression;
		end = kActorVal_Responsibility;
	}
	for (UInt32 av = start; av <= end; ++av) {
		//best not touch these
		if (av == kActorVal_Energy ||
			av == kActorVal_Health ||
			av == kActorVal_Magicka ||
			av == kActorVal_Fatigue ||
			av == kActorVal_Encumbrance)
			continue;
		ForceActorValue(actor, av, restore ? actor->GetBaseActorValue(av) : rng(1, 100));
	}
}