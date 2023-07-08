#include "randomizer.h"

/*#define FORMMAPADDR 0x00B0613C

NiTMapBase<unsigned int, TESForm*>* allObjects = (NiTMapBase<unsigned int, TESForm*>*)FORMMAPADDR;
*/

std::map<UInt32, std::vector<UInt32>> allWeapons;
std::map<UInt32, std::vector<UInt32>> allClothingAndArmor;
std::map<UInt32, std::vector<UInt32>> allGenericItems;
std::vector<UInt32> allCreatures;
std::set<UInt32> allAdded;

int clothingRanges[CRANGE_MAX];
int weaponRanges[WRANGE_MAX];

int oUseEssentialCreatures = 0;
int oExcludeQuestItems = 1;
int oRandCreatures = 1;
int oAddItems = 1;
int oDeathItems = 1;
int oWorldItems = 1;

bool skipMod[0xFF] = { 0 };

void InitModExcludes() {
	FILE* f = fopen("Data/RandomizerSkip.cfg", "r");
	if (f == NULL) {
		return;
	}
	char buf[256] = { 0 };
	unsigned char id = 0,
		randid = ModTable::Get().GetModIndex("Randomizer.esp");
	for (int i = 0; i < 0xFF; ++i) {
		skipMod[i] = false;
	}
	if (randid != 0xFF) {
		_MESSAGE("Randomizer.esp's ID is %02X", randid);
		skipMod[randid] = true;
	}
	else {
		_MESSAGE("Couldn't find Randomizer.esp's mod ID. Make sure that you did not rename the plugin file.");
	}
	if (f != NULL) {
		for (int i = 0; fscanf(f, "%255[^\n]\n", buf) > 0 /* != EOF*/ && i < 0xFF; ++i) {
			//same reasoning as in InitConfig()
			id = ModTable::Get().GetModIndex(buf);
			if (id == 0xFF) {
				_MESSAGE("Could not get mod ID for mod %s", buf);
				continue;
			}
			skipMod[id] = true;
			_MESSAGE("Skipping mod %s\n", buf);
		}
		fclose(f);
	}
}

char* cfgeol(char* s) {
	for (char *p = s; *p != 0; ++p) {
		if (*p == ';' || *p == '\t' || *p == '\n' || *p == '\r') {
			return p;
		}
	}
	return NULL;
}

#define OBRN_VAR2STR(x) #x
#define OBRN_PARSECONFIGLINE(line, var, str, len) \
if (strncmp(line, str, len) == 0 && isspace(line[len])) {\
	var = atoi(line + len + 1); \
	continue; \
}

#define OBRN_CONFIGLINE(line, var) \
{\
	int len = strlen(OBRN_VAR2STR(set ZZZOBRNRandomQuest.##var to));\
	OBRN_PARSECONFIGLINE(line, var, OBRN_VAR2STR(set ZZZOBRNRandomQuest.##var to), len);\
}

//this is not an elegant solution but i dont want to split the config into two files
void InitConfig() {
	FILE* f = fopen("Data/Randomizer.cfg", "r");
	if (f == NULL) {
		return;
	}
	char buf[256] = { 0 };
	for (int i = 0; fscanf(f, "%255[^\n]\n", buf) > 0/* != EOF*/ && i < 512; ++i) { 
		//the number of iterations is not necessary given the fscanf > 0 check but I'll sleep better knowing that this will never cause an infinite loop
		if (char* p = cfgeol(buf)) {
			*p = 0;
		}
		OBRN_CONFIGLINE(buf, oUseEssentialCreatures);
		OBRN_CONFIGLINE(buf, oExcludeQuestItems);
		OBRN_CONFIGLINE(buf, oRandCreatures);
		OBRN_CONFIGLINE(buf, oAddItems);
		OBRN_CONFIGLINE(buf, oDeathItems);
		OBRN_CONFIGLINE(buf, oWorldItems);
		//set ZZZOBRNRandomQuest.oUseEssentialCreatures to 0
		//set ZZZOBRNRandomQuest.oExcludeQuestItems to 1
	}
	fclose(f);
}

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

bool getRandomForKey(ItemMapPtr map, const UInt32 key, UInt32& out) {
	auto it = map->find(key);
	if (it == map->end() || !it->second.size()) {
		_ERROR("Couldn't find key %u for map %s\n", key, (map == &allWeapons ? "weapons" : (map == &allClothingAndArmor ? "clothing & armor" : "generic")));
		return false;
	}
	out = it->second[rand() % it->second.size()];
	return true;
}

void addOrAppend(ItemMapPtr map, const UInt32 key, const UInt32 value) {
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

bool isQuestItem(TESForm* item) {
	if (item->IsQuestItem()) {
		return true;
	}
	TESScriptableForm* scriptForm = OBLIVION_CAST(item, TESForm, TESScriptableForm);
	if (scriptForm != NULL && scriptForm->script != NULL) {
		return true;
	}
	return false;
}

bool tryToAddForm(TESForm* f) {
	ItemMapPtr ptr = NULL;
	UInt32 key = 0xFFFFFFFF;
	if (f != NULL && f->GetModIndex() != 0xFF && !skipMod[f->GetModIndex()] &&
		(f->GetFormType() == kFormType_Creature || (!isQuestItem(f) || !oExcludeQuestItems))) {
		if (allAdded.find(f->refID) != allAdded.end()) {
			return false;
		}
		const char* name = GetFullName(f);
		if (strncmp(name, "aaa", 3) == 0) {
			//exception for some test objects that typically don't even have a working model
			return false;
		}
		switch (f->GetFormType()) {
		case kFormType_Creature:
		{
			TESCreature* critter = OBLIVION_CAST(f, TESForm, TESCreature);
			TESScriptableForm* scriptForm = OBLIVION_CAST(f, TESForm, TESScriptableForm);
			if (name[0] != '<' && scriptForm != NULL && critter != NULL) {
				if (oUseEssentialCreatures || (!critter->actorBaseData.IsEssential() && scriptForm->script == NULL)) {
					const char* model = critter->modelList.modelList.Info();
					if (model == NULL) {
						model = "";
					}
					//hardcoded exception for SI grummites without a working model + excluding some test creatures
					if (strstr(name, "Test") == NULL && (strncmp(name, "Grummite Whelp", 14) ||
						strncmp(model, "GobLegs01.NIF", 13))) {
						allCreatures.push_back(f->refID);
						allAdded.insert(f->refID);
						return true;
					}
				}
			}
#ifdef _DEBUG
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
			ptr = &allGenericItems;
			key = f->GetFormType();
		default:
			break;
		}
		if (ptr != NULL && key != 0xFFFFFFFF) {
			addOrAppend(ptr, key, f->refID);
			return true;
		}
	}
	return false;
}

bool refIsItem(TESObjectREFR* ref) {
	switch (ref->baseForm->GetFormType()) {
	case kFormType_Armor:
	case kFormType_Clothing:
	case kFormType_Weapon:
	case kFormType_Apparatus:
	case kFormType_Book:
	case kFormType_Ingredient:
	case kFormType_Misc:
	case kFormType_Ammo:
	case kFormType_SoulGem:
	case kFormType_Key:
	case kFormType_AlchemyItem:
	case kFormType_SigilStone:
		return true;
	default:
		return false;
	}
}

const char* formToString[] = {
	TESFORM2STRING(kFormType_None),
	TESFORM2STRING(kFormType_TES4),
	TESFORM2STRING(kFormType_Group),
	TESFORM2STRING(kFormType_GMST),
	TESFORM2STRING(kFormType_Global),
	TESFORM2STRING(kFormType_Class),
	TESFORM2STRING(kFormType_Faction),
	TESFORM2STRING(kFormType_Hair),
	TESFORM2STRING(kFormType_Eyes),
	TESFORM2STRING(kFormType_Race),
	TESFORM2STRING(kFormType_Sound),
	TESFORM2STRING(kFormType_Skill),
	TESFORM2STRING(kFormType_Effect),
	TESFORM2STRING(kFormType_Script),
	TESFORM2STRING(kFormType_LandTexture),
	TESFORM2STRING(kFormType_Enchantment),
	TESFORM2STRING(kFormType_Spell),
	TESFORM2STRING(kFormType_BirthSign),
	TESFORM2STRING(kFormType_Activator),
	TESFORM2STRING(kFormType_Apparatus),
	TESFORM2STRING(kFormType_Armor),
	TESFORM2STRING(kFormType_Book),
	TESFORM2STRING(kFormType_Clothing),
	TESFORM2STRING(kFormType_Container),
	TESFORM2STRING(kFormType_Door),
	TESFORM2STRING(kFormType_Ingredient),
	TESFORM2STRING(kFormType_Light),
	TESFORM2STRING(kFormType_Misc),
	TESFORM2STRING(kFormType_Stat),
	TESFORM2STRING(kFormType_Grass),
	TESFORM2STRING(kFormType_Tree),
	TESFORM2STRING(kFormType_Flora),
	TESFORM2STRING(kFormType_Furniture),
	TESFORM2STRING(kFormType_Weapon),
	TESFORM2STRING(kFormType_Ammo),
	TESFORM2STRING(kFormType_NPC),
	TESFORM2STRING(kFormType_Creature),
	TESFORM2STRING(kFormType_LeveledCreature),
	TESFORM2STRING(kFormType_SoulGem),
	TESFORM2STRING(kFormType_Key),
	TESFORM2STRING(kFormType_AlchemyItem),
	TESFORM2STRING(kFormType_SubSpace),
	TESFORM2STRING(kFormType_SigilStone),
	TESFORM2STRING(kFormType_LeveledItem),
	TESFORM2STRING(kFormType_SNDG),
	TESFORM2STRING(kFormType_Weather),
	TESFORM2STRING(kFormType_Climate),
	TESFORM2STRING(kFormType_Region),
	TESFORM2STRING(kFormType_Cell),
	TESFORM2STRING(kFormType_REFR),
	TESFORM2STRING(kFormType_ACHR),
	TESFORM2STRING(kFormType_ACRE),
	TESFORM2STRING(kFormType_PathGrid),
	TESFORM2STRING(kFormType_WorldSpace),
	TESFORM2STRING(kFormType_Land),
	TESFORM2STRING(kFormType_TLOD),
	TESFORM2STRING(kFormType_Road),
	TESFORM2STRING(kFormType_Dialog),
	TESFORM2STRING(kFormType_DialogInfo),
	TESFORM2STRING(kFormType_Quest),
	TESFORM2STRING(kFormType_Idle),
	TESFORM2STRING(kFormType_Package),
	TESFORM2STRING(kFormType_CombatStyle),
	TESFORM2STRING(kFormType_LoadScreen),
	TESFORM2STRING(kFormType_LeveledSpell),
	TESFORM2STRING(kFormType_ANIO),
	TESFORM2STRING(kFormType_WaterForm),
	TESFORM2STRING(kFormType_EffectShader),
	TESFORM2STRING(kFormType_TOFT)
};

const char* FormToString(int form) {
	if (form >= 0 && form < sizeof(formToString) / sizeof(const char*)) {
		return formToString[form];
	}
	return "Unknown";
}

int myrand(int min, int max) {
	return ((double)rand() / (double)RAND_MAX) * (max - min) + min;
}

bool getFormsFromLeveledList(TESLevItem* lev, std::map<UInt32, TESForm*>& forms) {
	auto data = lev->leveledList.list.Info();
	auto next = lev->leveledList.list.Next();
	while (data != NULL) {
		if (data->form->GetFormType() == kFormType_LeveledItem) {
			if (!getFormsFromLeveledList(OBLIVION_CAST(data->form, TESForm, TESLevItem), forms)) {
				return false;
			}
		}
		else {
			if (isQuestItem(data->form) && oExcludeQuestItems) {
				return false;
			}
			forms.insert(std::make_pair(data->form->GetFormType(), data->form));
		}
		if (next == NULL) {
			break;
		}
		data = next->Info();
		next = next->Next();
	}
	return true;
}

void getInventoryFromTESLevItem(TESLevItem* lev, std::map<TESForm*, int>& itemList, bool addQuestItems) {
	auto data = lev->leveledList.list.Info();
	auto next = lev->leveledList.list.Next();
	while (data != NULL) {
		if (data->form->GetFormType() == kFormType_LeveledItem) {
			getInventoryFromTESLevItem(OBLIVION_CAST(data->form, TESForm, TESLevItem), itemList, addQuestItems);
		}
		else {
			if (!isQuestItem(data->form) || addQuestItems) {
				auto it = itemList.find(data->form);
				int count = max(1, data->count);
				if (it == itemList.end()) {
					itemList.insert(std::make_pair(data->form, count));
				}
				else {
					it->second += count;
				}
			}
		}
		if (next == NULL) {
			break;
		}
		data = next->Info();
		next = next->Next();
	}
}

void getInventoryFromTESContainer(TESContainer* container, std::map<TESForm*, int>& itemList, bool addQuestItems) {
	auto data = container->list.Info();
	auto next = container->list.Next();
	while (data != NULL) {
		if (data->type->GetFormType() == kFormType_LeveledItem) {
			getInventoryFromTESLevItem(OBLIVION_CAST(data->type, TESForm, TESLevItem), itemList, addQuestItems);
		}
		else {
			if (!isQuestItem(data->type) || addQuestItems) {
				auto it = itemList.find(data->type);
				int count = max(1, data->count);
				if (it == itemList.end()) {
					itemList.insert(std::make_pair(data->type, count));
				}
				else {
					it->second += count;
				}
			}
		}
		if (next == NULL) {
			break;
		}
		data = next->Info();
		next = next->Next();
	}
}

void getContainerInventory(TESObjectREFR* ref, std::map<TESForm*, int> & itemList, bool addQuestItems) {
	ExtraContainerChanges* cont = (ExtraContainerChanges*)ref->baseExtraList.GetByType(kExtraData_ContainerChanges);
	while (cont != NULL && cont->data != NULL && cont->data->objList != NULL) {
		for (auto it = cont->data->objList->Begin(); !it.End(); ++it) {
			TESForm* item = it->type;
			UInt32 count = max(1, it->countDelta); //certain items, for whatever reasons, have count 0
			//_MESSAGE("Ref: %s (%08X) (base: %s %08X) : found a %s (%08X %s), quantity: %i",
			//	GetFullName(ref), ref->refID, GetFullName(ref->baseForm), ref->baseForm->refID, GetFullName(item), item->refID, FormToString(item->GetFormType()), count);
			TESScriptableForm* scriptForm = OBLIVION_CAST(item, TESForm, TESScriptableForm);
			if (GetFullName(item)[0] != '<' && (addQuestItems || (!item->IsQuestItem() && (!oExcludeQuestItems || scriptForm == NULL || scriptForm->script == NULL)))) {
				auto it = itemList.find(item);
				if (it == itemList.end()) {
					itemList.insert(std::make_pair(item, count));
				}
				else {
					it->second += count;
				}
			}
		}
		BSExtraData* ptr = cont->next;
		while (ptr != NULL) {// && cont->next->type == kExtraData_ContainerChanges) {
			if (ptr->type == kExtraData_ContainerChanges) {
				break;
			}
			ptr = ptr->next;
		}
		cont = (ExtraContainerChanges*)ptr;
	}
	if (ref->GetFormType() == kFormType_ACRE) {
		TESActorBase* actorBase = OBLIVION_CAST(ref->baseForm, TESForm, TESActorBase);
		if (actorBase == NULL) {
			return;
		}
		getInventoryFromTESContainer(&actorBase->container, itemList, addQuestItems);
	}
}

int getPlayerLevel() {
	TESActorBase* actorBase = OBLIVION_CAST(*g_thePlayer, PlayerCharacter, TESActorBase);
	if (actorBase == NULL) {
		return 0;
	}
	return actorBase->actorBaseData.level;
}

void randomizeInventory(TESObjectREFR* ref) {
	//Console_Print("Randomizing the inventory of %s", GetFullName(ref));
	if (allGenericItems.size() == 0 || allWeapons.size() == 0 || allClothingAndArmor.size() == 0) {
		return;
	}
	std::map<TESForm*, int> removeItems;
	getContainerInventory(ref, removeItems, !oExcludeQuestItems);
	for (auto it = removeItems.begin(); it != removeItems.end(); ++it) {
		TESForm* item = it->first;
		int cnt = it->second;
		if (item->GetFormType() != kFormType_Book && item->GetFormType() != kFormType_Key &&
			(item->GetModIndex() != 0xFF && !skipMod[item->GetModIndex()])) {
			/*if (ref->IsActor()) {
				ref->Unequip(it->first, it->second, NULL);
			}*/
			ref->RemoveItem(item, NULL, cnt, 0, 0, NULL, NULL, NULL, 0, 0);
		}
	}
	removeItems.clear();
	//granting random items
	UInt32 selection;
	if (ref->GetFormType() == kFormType_ACHR) {
		Actor* actor = OBLIVION_CAST(ref, TESObjectREFR, Actor);
		TESActorBase* actorBase = OBLIVION_CAST(actor->baseForm, TESForm, TESActorBase);
		int roll = rand() % 100, level = min(1, actorBase->actorBaseData.IsPCLevelOffset() ? getPlayerLevel() + actorBase->actorBaseData.level : actorBase->actorBaseData.level);
		//Gold
		if (roll < 50) {
#ifdef _DEBUG
			_MESSAGE("GOLD: %s receiving gold", GetFullName(ref));
#endif
			ref->AddItem(LookupFormByID(ITEM_GOLD), NULL, myrand(5, 60) * level);
		}
		//Weapon Randomization
		int clothingStatus = OBRNRC_LOWER | OBRNRC_FOOT | OBRNRC_HAND | OBRNRC_UPPER;
		bool gotTwoHanded = false;
		bool gotBow = false;
		for (int i = 0; i < 2; ++i) {
			roll = myrand(0, weaponRanges[UNARMED]);
			int v;
			for (v = 0; v < WRANGE_MAX; ++v) {
				if (roll < weaponRanges[v]) {
					break;
				}
			}
			switch (v) {
			case BLADES:
				if (getRandomForKey(&allWeapons, TESObjectWEAP::kType_BladeOneHand, selection)) {
					TESObjectWEAP* wp = OBLIVION_CAST(LookupFormByID(selection), TESForm, TESObjectWEAP);
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
				if (getRandomForKey(&allWeapons, TESObjectWEAP::kType_BluntOneHand, selection)) {
					TESObjectWEAP* wp = OBLIVION_CAST(LookupFormByID(selection), TESForm, TESObjectWEAP);
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
				if (getRandomForKey(&allWeapons, TESObjectWEAP::kType_Staff, selection)) {
					TESObjectWEAP* wp = OBLIVION_CAST(LookupFormByID(selection), TESForm, TESObjectWEAP);
#ifdef _DEBUG
					_MESSAGE("STAFF: %s receiving %s", GetFullName(ref), GetFullName(wp));
#endif
					ref->AddItem(wp, NULL, 1);
					if (!i) {
						ref->Equip(wp, 1, NULL, 0);
					}
					gotTwoHanded = true;
				}
			case BOWS:
				if (getRandomForKey(&allWeapons, TESObjectWEAP::kType_Bow, selection)) {
					TESObjectWEAP* wp = OBLIVION_CAST(LookupFormByID(selection), TESForm, TESObjectWEAP);
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
			default:
#ifdef _DEBUG
				_MESSAGE("UNARMED: %s is unarmed", GetFullName(ref));
#endif
				break;
			}
		}
		if (!gotBow) {
			if (!myrand(0, 5)) {
				if (getRandomForKey(&allWeapons, TESObjectWEAP::kType_Bow, selection)) {
					TESObjectWEAP* wp = OBLIVION_CAST(LookupFormByID(selection), TESForm, TESObjectWEAP);
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
				if (getRandomForKey(&allGenericItems, kFormType_Ammo, selection)) {
					TESForm* ammo = LookupFormByID(selection);
#ifdef _DEBUG
					_MESSAGE("ARROW: %s receiving %s", GetFullName(ref), GetFullName(ammo));
#endif
					ref->AddItem(ammo, NULL, myrand(20, 100));
					if (!i) {
						ref->Equip(ammo, 1, NULL, 0);
						if (myrand(0, i + 1)) {
							break;
						}
					}
				}
			}
		}
		//Shield
		if (!gotTwoHanded && !myrand(0, 2) && getRandomForKey(&allClothingAndArmor, kSlot_Shield, selection)) {
			TESForm* shield = LookupFormByID(selection);
#ifdef _DEBUG
			_MESSAGE("SHIELD: %s receiving %s", GetFullName(ref), GetFullName(shield));
#endif
			ref->AddItem(shield, NULL, 1);
			ref->Equip(shield, 1, NULL, 0);
		}
		//Clothing
		if (!myrand(0, 19)) {
			clothingStatus &= ~OBRNRC_UPPER;
		}
		if (!myrand(0, 15)) {
			clothingStatus &= ~OBRNRC_LOWER;
		}
		if (!myrand(0, 13)) {
			clothingStatus &= ~OBRNRC_FOOT;
		}
		if (!myrand(0, 9)) {
			clothingStatus &= ~OBRNRC_HAND;
		}
		if (clothingStatus & OBRNRC_UPPER) {
			roll = myrand(0, clothingRanges[UPPERLOWERHAND]);
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
				if (getRandomForKey(&allClothingAndArmor, clothingKeys[v], selection)) {
					TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
					_MESSAGE("UPPER: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
					ref->AddItem(cloth, NULL, 1);
					ref->Equip(cloth, 1, NULL, 0);
				}
			}
			else {
#ifdef _DEBUG
				_ERROR("v == CRANGE_MAX. this should not happen");
#endif
			}
		}
		if (clothingStatus & OBRNRC_LOWER) {
			if (getRandomForKey(&allClothingAndArmor, kSlot_LowerBody, selection)) {
				TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("LOWER: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
				ref->AddItem(cloth, NULL, 1);
				ref->Equip(cloth, 1, NULL, 0);
			}
		}
		if (clothingStatus & OBRNRC_FOOT) {
			if (getRandomForKey(&allClothingAndArmor, kSlot_Foot, selection)) {
				TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("FOOT: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
				ref->AddItem(cloth, NULL, 1);
				ref->Equip(cloth, 1, NULL, 0);
			}
		}
		if (clothingStatus & OBRNRC_HAND) {
			if (getRandomForKey(&allClothingAndArmor, kSlot_Hand, selection)) {
				TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("HAND: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
				ref->AddItem(cloth, NULL, 1);
				ref->Equip(cloth, 1, NULL, 0);
			}
		}
		//Head
		if (myrand(0, 2)) {
			if (getRandomForKey(&allClothingAndArmor, kSlot_Head, selection)) {
				TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("HEAD: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
				ref->AddItem(cloth, NULL, 1);
				ref->Equip(cloth, 1, NULL, 0);
			}
		}
		//Rings
		for (int i = 0; i < 2; ++i) {
			if (!myrand(0, i + 1)) {
				if (getRandomForKey(&allClothingAndArmor, kSlot_LeftRing, selection)) {
					TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
					_MESSAGE("RING: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
					ref->AddItem(cloth, NULL, 1);
					ref->Equip(cloth, 1, NULL, 0);
				}
			}
		}
		//Amulets
		if (!myrand(0, 2)) {
			if (getRandomForKey(&allClothingAndArmor, kSlot_Amulet, selection)) {
				TESForm* cloth = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("AMULET: %s receiving %s", GetFullName(ref), GetFullName(cloth));
#endif
				ref->AddItem(cloth, NULL, 1);
				ref->Equip(cloth, 1, NULL, 0);
			}
		}
		//Potions
		for (int i = 0; i < 5; ++i) {
			if (myrand(0, 2)) {
				break;
			}
			if (getRandomForKey(&allGenericItems, kFormType_AlchemyItem, selection)) {
				TESForm* item = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("POTION: %s receiving %s", GetFullName(ref), GetFullName(item));
#endif
				ref->AddItem(item, NULL, myrand(1, 4));
			}
		}
		//Soul Gems
		for (int i = 0; i < 6; ++i) {
			if (myrand(0, 4)) {
				break;
			}
			if (getRandomForKey(&allGenericItems, kFormType_SoulGem, selection)) {
				TESForm* item = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("SOULGEM: %s receiving %s", GetFullName(ref), GetFullName(item));
#endif
				ref->AddItem(item, NULL, myrand(1, 2));
			}
		}
		//Sigil Stones
		if (!myrand(0, 9)) {
			if (getRandomForKey(&allGenericItems, kFormType_SigilStone, selection)) {
				TESForm* item = LookupFormByID(selection);
#ifdef _DEBUG
				_MESSAGE("SIGILSTONE: %s receiving %s", GetFullName(ref), GetFullName(item));
#endif
				ref->AddItem(item, NULL, 1);
			}
		}
		//Ingredients
		if (!myrand(0, 5)) {
			for (int i = 0; i < 10; ++i) {
				if (myrand(0, 3)) {
					break;
				}
				if (getRandomForKey(&allGenericItems, kFormType_Ingredient, selection)) {
					TESForm* item = LookupFormByID(selection);
#ifdef _DEBUG
					_MESSAGE("INGREDIENT: %s receiving %s", GetFullName(ref), GetFullName(item));
#endif
					ref->AddItem(item, NULL, myrand(1, 6));
				}
			}
		}
		//Apparatus
		if (!myrand(0, 9)) {
			for (int i = 0; i < 3; ++i) {
				if (myrand(0, 2)) {
					break;
				}
				if (getRandomForKey(&allGenericItems, kFormType_Apparatus, selection)) {
					TESForm* item = LookupFormByID(selection);
#ifdef _DEBUG
					_MESSAGE("APPARATUS: %s receiving %s", GetFullName(ref), GetFullName(item));
#endif
					ref->AddItem(item, NULL, myrand(1, 2));
				}
			}
		}
		//Clutter
		if (!myrand(0, 9)) {
			for (int i = 0; i < 9; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_Misc, selection)) {
					TESForm* item = LookupFormByID(selection);
#ifdef _DEBUG
					_MESSAGE("GENERIC: %s receiving %s", GetFullName(ref), GetFullName(item));
#endif
					ref->AddItem(item, NULL, myrand(1, 3));
				}
				if (myrand(0, i + 1)) {
					break;
				}
			}
		}
	}
	else {//containers
		//Gold
		if (!myrand(0, 2)) {
			ref->AddItem(LookupFormByID(ITEM_GOLD), NULL, myrand(1, 100) * (!myrand(0, 4) ? 30 : 1));
		}
		//Lockpick
		if (!myrand(0, 4)) {
			ref->AddItem(LookupFormByID(ITEM_LOCKPICK), NULL, myrand(1, 10));
		}
		//Repair Hammer
		if (!myrand(0, 4)) {
			ref->AddItem(LookupFormByID(ITEM_REPAIRHAMMER), NULL, myrand(1, 3));
		}
		//Clutter
		if (!myrand(0, 5)) {
			for (int i = 0; i < 3; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_Misc, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, myrand(1, 5));
					if (myrand(0, i + 3)) {
						break;
					}
				}
			}
		}
		//Potions
		if (!myrand(0, 3)) {
			for (int i = 0; i < 5; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_AlchemyItem, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, myrand(1, 5));
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Ingredients
		if (!myrand(0, 4)) {
			for (int i = 0; i < 6; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_Ingredient, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, myrand(1, 2));
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Books
		if (!myrand(0, 4)) {
			for (int i = 0; i < 3; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_Book, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, 1);
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Soul Gems
		if (!myrand(0, 9)) {
			for (int i = 0; i < 3; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_SoulGem, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, myrand(1, 3));
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Sigil Stones
		if (!myrand(0, 11)) {
			for (int i = 0; i < 2; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_SigilStone, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, 1);
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Keys
		if (!myrand(0, 6)) {
			if (getRandomForKey(&allGenericItems, kFormType_Key, selection)) {
				ref->AddItem(LookupFormByID(selection), NULL, 1);
			}
		}
		//Apparatus
		if (!myrand(0, 8)) {
			for (int i = 0; i < 4; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_Apparatus, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, 1);
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Weapons
		if (!myrand(0, 4)) {
			for (int i = 0; i < 3; ++i) {
				int wpType = -1;
				switch (myrand(0, 3)) {
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
				if (getRandomForKey(&allWeapons, wpType, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, 1);
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Ammo
		if (!myrand(0, 4)) {
			for (int i = 0; i < 4; ++i) {
				if (getRandomForKey(&allGenericItems, kFormType_Ammo, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, myrand(1, 100));
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
		//Clothing / Armor
		if (!myrand(0, 2)) {
			for (int i = 0; i < 7; ++i) {
				UInt32 key = 0xFFFFFFFF;
				switch (myrand(0, 12)) { //was 0, 7 in the script
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
				if (key != 0xFFFFFFFF && getRandomForKey(&allClothingAndArmor, key, selection)) {
					ref->AddItem(LookupFormByID(selection), NULL, 1);
					if (myrand(0, i + 1)) {
						break;
					}
				}
			}
		}
	}
}


TESForm* getFormFromLeveledList(TESLevItem* lev) {
	if (lev == NULL) {
		return NULL;
	}
	std::map<UInt32, TESForm*> forms;
	if (!getFormsFromLeveledList(lev, forms) || !forms.size()) {
		return NULL;
	}
	int i = -1, cnt = myrand(0, forms.size());
	auto it = forms.begin();
	while (it != forms.end()) {
		if (++i == cnt) {
			return it->second;
		}
		++it;
	}
	return NULL;
}

bool getRandomByType(TESForm *f, UInt32& out) {
	ItemMapPtr ptr = NULL;
	UInt32 key = 0xFFFFFFFF;
	if (f == NULL) {
		return false;
	}
	if (oExcludeQuestItems && isQuestItem(f)) {
		return false;
	}
	if (f->GetModIndex() != 0xFF && skipMod[f->GetModIndex()]) { //very important!
		return false;
	}
	switch (f->GetFormType()) {
	case kFormType_LeveledItem:
	{
		TESLevItem* lev = OBLIVION_CAST(f, TESForm, TESLevItem);
		return getRandomByType(getFormFromLeveledList(lev), out);
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
	default:
		break;
	}
	if (ptr != NULL && key != 0xFFFFFFFF) {
		return getRandomForKey(ptr, key, out);
	}
	return false;
}

void randomize(TESObjectREFR* ref, const char* function) {
	if (ref->GetFormType() == kFormType_ACRE) {
		if (allCreatures.size() == 0) {
			return;
		}
		if (strcmp(function, "ESP") == 0) {
			QueueUIMessage_2("Randomizing creatures through the spell may cause issues", 1000, NULL, NULL);
		}
		else if (!oRandCreatures) {
			return;
		}
		Actor* actor = OBLIVION_CAST(ref, TESObjectREFR, Actor);
		if (actor == NULL) {
			return;
		}
		UInt32 health = actor->GetBaseActorValue(kActorVal_Health), aggression = actor->GetActorValue(kActorVal_Aggression);//actor->GetBaseActorValue(kActorVal_Aggression);
		if (health == 0) {
#ifdef _DEBUG
			_MESSAGE("%s: Dead creature %s %08X will be treated as a container", function, GetFullName(ref), ref->refID);
#endif
			randomizeInventory(ref);
			return;
		}
		std::map<TESForm*, int> keepItems;
		getContainerInventory(ref, keepItems, true);
		TESForm* oldBaseForm = ref->GetTemplateForm() != NULL ? ref->GetTemplateForm() : ref->baseForm,
			* rando = LookupFormByID(allCreatures[rand() % allCreatures.size()]);
#ifdef _DEBUG
		_MESSAGE("%s: Going to randomize %s %08X into %s %08X", function, GetFullName(ref), ref->refID, GetFullName(rando), rando->refID);
#endif
		//TESActorBase* actorBase = OBLIVION_CAST(rando, TESForm, TESActorBase);
		ref->baseForm = rando;
		ref->SetTemplateForm(oldBaseForm);
		actor->SetActorValue(kActorVal_Aggression, aggression);
		for (auto it = keepItems.begin(); it != keepItems.end(); ++it) {
			TESForm* item = it->first;
			int cnt = it->second;
			ref->AddItem(item, NULL, cnt);
		}
	}
	else if (refIsItem(ref)) {
		if (!oWorldItems) {
			return;
		}
#ifdef _DEBUG
		_MESSAGE("%s: World item randomization: will try to randomize %s %08X", function, GetFullName(ref), ref->refID);
#endif
		if (isQuestItem(ref->baseForm) && oExcludeQuestItems) {
			return;
		}
		UInt32 selection;
		if (getRandomByType(ref->baseForm, selection)) {
			TESForm* rando = LookupFormByID(selection);
#ifdef _DEBUG
			_MESSAGE("%s: Going to randomize %s %08X into %s %08X", function, GetFullName(ref), ref->refID, GetFullName(rando), rando->refID);
#endif
			ref->baseForm = rando;
			ref->Update3D();
		}
	}
	else {
		randomizeInventory(ref);
	}
}