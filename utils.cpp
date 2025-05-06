#include "utils.h"
#include "rng.h"

bool isQuestOrScriptedItem(TESForm* item, bool keysAreQuestItems) {
	if (keysAreQuestItems && item->GetFormType() == kFormType_Key) {
		return true;
	}
	if (item->IsQuestItem()) {
		return true;
	}
	TESScriptableForm* scriptForm = OBLIVION_CAST(item, TESForm, TESScriptableForm);
	if (scriptForm != NULL && scriptForm->script != NULL) {
		return true;
	}
	return false;
}

bool modelExists(TESForm* f) {
	const char* model1, * model2;
	model1 = model2 = NULL;
	switch (f->GetFormType()) {
	case kFormType_Spell:
		return true;
	case kFormType_Armor: {
		TESObjectARMO* armor = OBLIVION_CAST(f, TESForm, TESObjectARMO);
		if (config.oExcludeUnplayableItems && !armor->bipedModel.IsPlayable()) {
#ifdef _DEBUG
			_MESSAGE("Model validation: %s (%08X %s) is marked as unplayable.", GetFullName(f), f->refID, FormTypeToString(f->GetFormType()));
#endif
			return false;
		}
		model1 = armor->bipedModel.GetPath(TESBipedModelForm::kPath_Ground, false);
		model2 = armor->bipedModel.GetPath(TESBipedModelForm::kPath_Ground, true);
		break;
	}
	case kFormType_Clothing: {
		TESObjectCLOT* clothing = OBLIVION_CAST(f, TESForm, TESObjectCLOT);
		if (!clothing->bipedModel.IsPlayable()) {
#ifdef _DEBUG
			_MESSAGE("Model validation: %s (%08X %s) is marked as unplayable.", GetFullName(f), f->refID, FormTypeToString(f->GetFormType()));
#endif
			return false;
		}
		model1 = clothing->bipedModel.GetPath(TESBipedModelForm::kPath_Ground, false);
		model2 = clothing->bipedModel.GetPath(TESBipedModelForm::kPath_Ground, true);
		break;
	}
	case kFormType_Weapon: {
		TESObjectWEAP* weapon = OBLIVION_CAST(f, TESForm, TESObjectWEAP);
		model1 = weapon->model.GetModelPath();
		break;
	}
	case kFormType_Apparatus: {
		TESObjectAPPA* apparatus = OBLIVION_CAST(f, TESForm, TESObjectAPPA);
		model1 = apparatus->model.GetModelPath();
		break;
	}
	case kFormType_Book: {
		TESObjectBOOK* book = OBLIVION_CAST(f, TESForm, TESObjectBOOK);
		model1 = book->model.GetModelPath();
		break;
	}
	case kFormType_Ingredient: {
		IngredientItem* item = OBLIVION_CAST(f, TESForm, IngredientItem);
		model1 = item->model.GetModelPath();
		break;
	}
	case kFormType_Misc: {
		TESObjectMISC* misc = OBLIVION_CAST(f, TESForm, TESObjectMISC);
		model1 = misc->model.GetModelPath();
		break;
	}
	case kFormType_Ammo: {
		TESAmmo* ammo = OBLIVION_CAST(f, TESForm, TESAmmo);
		model1 = ammo->model.GetModelPath();
		break;
	}
	case kFormType_SoulGem: {
		TESSoulGem* soulgem = OBLIVION_CAST(f, TESForm, TESSoulGem);
		model1 = soulgem->model.GetModelPath();
		break;
	}
	case kFormType_Key: {
		TESKey* key = OBLIVION_CAST(f, TESForm, TESKey);
		model1 = key->model.GetModelPath();
		break;
	}
	case kFormType_AlchemyItem: {
		AlchemyItem* alchemy = OBLIVION_CAST(f, TESForm, AlchemyItem);
		model1 = alchemy->model.GetModelPath();
		break;
	}
	case kFormType_SigilStone: {
		TESSigilStone* sigilstone = OBLIVION_CAST(f, TESForm, TESSigilStone);
		model1 = sigilstone->model.GetModelPath();
		break;
	}
	default:
#ifdef _DEBUG
		_MESSAGE("Model validation: not supported for %s", FormTypeToString(f->GetFormType()));
#endif
		return true;
	}
	if (!model1 && !model2) {
#ifdef _DEBUG_MODEL_VALIDATION
		_MESSAGE("Model validation: %s (%08X %s) has NULL model1 and model2", GetFullName(f), f->refID, FormTypeToString(f->GetFormType()));
#endif
		return false;
	}

	if (model1) {
#ifdef _DEBUG_MODEL_VALIDATION
		_MESSAGE("Model validation: %s (%08X %s) has model1 %s", GetFullName(f), f->refID, FormTypeToString(f->GetFormType()), model1);
#endif
		if (!model1[0]) {
#ifdef _DEBUG_MODEL_VALIDATION
			_MESSAGE("...rejecting it because its empty.");
#endif
			return false;
		}
	}

	if (model2) {
#ifdef _DEBUG_MODEL_VALIDATION
		_MESSAGE("Model validation: %s (%08X %s) has model2 %s", GetFullName(f), f->refID, FormTypeToString(f->GetFormType()), model2);
#endif
		if (!model2[0]) {
#ifdef _DEBUG_MODEL_VALIDATION
			_MESSAGE("...rejecting it because its empty.");
#endif
			return false;
		}
	}
	return true;
}

bool creatureValid(const char* name, const char* model) {
	//hardcoded exception for SI grummites without a working model + excluding some test creatures
	if (!name[0]) {
		return false;
	}

	if (strstr(name, "Test")) {
		return false;
	}

	if (strncmp(name, "Grummite Whelp", 14) == 0 &&
		strncmp(model, "GobLegs01.NIF", 13) == 0) {
		return false;
	}

	//exclude creatures with a lowercase letter followed by an uppercase letter - these are typically
	//test creatures
	//would be lovely, but it ends up excluding GateKeeper
	/*for (int i = 0; name[i + 1]; ++i) {
		if (islower(name[i]) && isupper(name[i + 1])) {
			return false;
		}
	}*/

	return true;
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

bool itemIsEquippable(TESForm* item) {
	switch (item->GetFormType()) {
	case kFormType_Armor:
	case kFormType_Clothing:
	case kFormType_Weapon:
	case kFormType_Ammo:
		return true;
	default:
		return false;
	}
}

static const char* formToString[] = {
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

const char* FormTypeToString(int form) {
	if (form >= 0 && form < sizeof(formToString) / sizeof(const char*)) {
		return formToString[form];
	}
	return "Unknown";
}

//returns false if the rejectOnQuestItem flag is on and there's a quest item in the leveled list
bool getFormsFromLeveledList(TESLeveledList* list, std::vector<std::pair<TESForm*, int>>& itemList, UInt16 flag) {
	auto data = list->list.Info();
	auto next = list->list.Next();
	bool addQuestItems = (flag & ItemRetrieval::all),
		rejectOnQuestItem = (flag & ItemRetrieval::rejectOnQuestItem);
	while (data != NULL) {
		switch (data->form->GetFormType()) {
		case kFormType_LeveledItem:
		{
			TESLevItem* nested = OBLIVION_CAST(data->form, TESForm, TESLevItem);
			if (!getFormsFromLeveledList(&nested->leveledList, itemList, flag) && rejectOnQuestItem) {
				return false;
			}
			break;
		}
		case kFormType_LeveledSpell:
		{
			TESLevSpell* nested = OBLIVION_CAST(data->form, TESForm, TESLevSpell);
			if (!getFormsFromLeveledList(&nested->leveledList, itemList, flag) && rejectOnQuestItem) {
				return false;
			}
			break;
		}
		default:
		{
			bool questItem = isQuestOrScriptedItem(data->form, false);
			if (questItem && rejectOnQuestItem) {
				return false;
			}
			if (!questItem || addQuestItems) {
				int count = std::max((UInt16)1, data->count);
				if (flag & ItemRetrieval::noAccumulation) {
					itemList.push_back(std::make_pair(data->form, count));
				}
				else {
					bool found = false;
					for (auto& it : itemList) {
						if (it.first == data->form) {
							it.second += count;
							found = true;
							break;
						}
					}
					if (!found) {
						itemList.push_back(std::make_pair(data->form, count));
					}
				}
			}
		}

		}
		if (!next) {
			break;
		}
		data = next->Info();
		next = next->Next();
	}
	return true;
}

std::pair<TESForm*, int> getRandomFormFromLeveledList(TESLeveledList* list, UInt16 flag) {
	std::vector<std::pair<TESForm*, int>> itemList;
	if (!getFormsFromLeveledList(list, itemList, flag) || !itemList.size()) {
		return std::make_pair((TESForm*)NULL, (int)0);
	}
	int r = rng(0, itemList.size() - 1);
#ifdef _DEBUG_LEVLIST
	_MESSAGE("    %s: we are returning %s %08X x%i from the leveled list", __FUNCTION__, GetFullName(itemList[r].first), itemList[r].first->refID, itemList[r].second);
#endif
	return itemList[r];
}

bool getInventoryFromTESContainer(TESContainer* container, std::unordered_map<TESForm*, int>& itemList, UInt16 flag) {
	if (!container) {
		return false;
	}
	bool hasFlag = false, addQuestItems = (flag & ItemRetrieval::all);
	auto data = container->list.Info();
	auto next = container->list.Next();
	while (data != NULL) {
		if (data->type != NULL) {
			switch (data->type->GetFormType()) {
			case kFormType_LeveledItem:
			{
				TESLevItem* lev = OBLIVION_CAST(data->type, TESForm, TESLevItem);
				auto it = getRandomFormFromLeveledList(&lev->leveledList, flag);
				if (it.first != NULL) {
					itemList.insert(it);
				}
				break;
			}
			case kFormType_LeveledSpell:
			{
				TESLevSpell* lev = OBLIVION_CAST(data->type, TESForm, TESLevSpell);
				auto it = getRandomFormFromLeveledList(&lev->leveledList, flag);
				if (it.first != NULL) {
					itemList.insert(it);
				}
				break;
			}
			default:
#ifdef _DEBUG_LEVLIST
				_MESSAGE("    Found item %s %08X x%i", GetFullName(data->type), data->type->refID, data->count);
#endif
				if (data->type == obrnFlag) {
#ifdef _DEBUG
					_MESSAGE(__FUNCTION__": OBRN Flag has been found for");
#endif
					hasFlag = true;
				}
				else if (!isQuestOrScriptedItem(data->type, false) || addQuestItems) {
					auto it = itemList.find(data->type);
					int count = std::max((SInt32)1, data->count);
					if (it == itemList.end()) {
						itemList.insert(std::make_pair(data->type, count));
					}
					else if (!(flag & ItemRetrieval::noAccumulation)) {
						it->second += count;
					}
				}
				break;
			}
		}
		if (!next) {
			break;
		}
		data = next->Info();
		next = next->Next();
	}
	return hasFlag;
}

bool getContainerInventory(TESObjectREFR* ref, std::unordered_map<TESForm*, int>& itemList, UInt16 flag) {
	bool hasFlag = false, addQuestItems = (flag & ItemRetrieval::all);
	ExtraContainerChanges* cont = (ExtraContainerChanges*)ref->baseExtraList.GetByType(kExtraData_ContainerChanges);
	while (cont != NULL && cont->data != NULL && cont->data->objList != NULL) {
		for (auto it = cont->data->objList->Begin(); !it.End(); ++it) {
			//TESForm* item = it->type; //this line has crashed. 
			//it should not crash until any circumstances. but it has. and everything about the underlying classes is horrible.
			auto itemEntry = it.accessNode();
			if (!itemEntry || !itemEntry->item) {
				//anvil jailor (be56) consistently triggers this
				_WARNING("itemEntry or itemEntry->item is NULL for %s (%08X %s)", 
					GetFullName(ref), ref->refID, FormTypeToString(ref->GetFormType()));
				continue;
			}
			TESForm* item = itemEntry->item->type;
			UInt32 count = std::max((SInt32)1, itemEntry->item->countDelta);
			if (item == obrnFlag) {
				hasFlag = true;
#ifdef _DEBUG
				_MESSAGE(__FUNCTION__": OBRN Flag has been found for %s (%08X)", GetFullName(ref), ref->refID);
#endif
				continue;
			}
			//_MESSAGE("Ref: %s (%08X) (base: %s %08X) : found a %s (%08X %s), quantity: %i",
			//	GetFullName(ref), ref->refID, GetFullName(ref->baseForm), ref->baseForm->refID, GetFullName(item), item->refID, FormToString(item->GetFormType()), count);
#ifdef _DEBUG_LEVLIST
			_MESSAGE("    Found item %s %08X x%u", GetFullName(item), item->refID, count);
#endif
			if (GetFullName(item)[0] != '<' && (addQuestItems || !isQuestOrScriptedItem(item, false))) {
				auto it = itemList.find(item);
				if (it == itemList.end()) {
					itemList.insert(std::make_pair(item, count));
				}
				else if (!(flag & ItemRetrieval::noAccumulation)) {
					it->second += count;
				}
			}
		}
		BSExtraData* ptr = cont->next;
		while (ptr != NULL) {
			if (ptr->type == kExtraData_ContainerChanges) {
				break;
			}
			ptr = ptr->next;
		}
		cont = (ExtraContainerChanges*)ptr;
	}
	if (flag & ItemRetrieval::noTESContainer) {
		return hasFlag;
	}
	return std::max(hasFlag, getInventoryFromTESContainer(ref->GetContainer(), itemList, flag | ItemRetrieval::noAccumulation));
}

int getPlayerLevel() {
	TESActorBase* actorBase = OBLIVION_CAST(*g_thePlayer, PlayerCharacter, TESActorBase);
	if (!actorBase) {
		return 0;
	}
	return actorBase->actorBaseData.level;
}

int getRefLevelAdjusted(TESObjectREFR* ref) {
	if (!ref->IsActor()) {
		return 1;
	}
	Actor* actor = OBLIVION_CAST(ref, TESObjectREFR, Actor);
	TESActorBase* actorBase = OBLIVION_CAST(actor->baseForm, TESForm, TESActorBase);
	return std::min(1, actorBase->actorBaseData.IsPCLevelOffset() ? getPlayerLevel() + actorBase->actorBaseData.level : actorBase->actorBaseData.level);
}

UInt8 GetModIndexShifted(const std::string& name) {
	UInt8 id = ModTable::Get().GetModIndex(name);
	if (id == 0xFF) {
		return id;
	}
	const ModEntry** activeModList = (*g_dataHandler)->GetActiveModList();
	if (_stricmp(activeModList[0]->data->name, "oblivion.esm") && 
		activeModList[id] != NULL && 
		_stricmp(activeModList[id]->data->name, name.c_str()) == 0) {
		++id;
	}
	return id;
}

bool spellBlacklisted(SpellItem* spell) {
	switch (spell->refID) {
	case SPELL_SKELETONKEY:
	case SPELL_HEMOPHILIA:
		return true;
	default:
		return false;
	}
}

void ForceActorValue(Actor* actor, UInt32 av, SInt32 value) {
	//Mod(value) = GetActorValue + value
	//Mod(value - GetActorValue) = value
	actor->ApplyScriptAVMod(av, value - actor->GetActorValue(av), NULL);
}