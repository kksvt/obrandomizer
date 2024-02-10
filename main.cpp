#include "randomizer.h"

IDebugLog		gLog("obrandomizer.log");

OBSEScriptInterface* g_scriptInterface = NULL;	// make sure you assign to this
PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;
OBSEScriptInterface			* g_scriptIntfc = NULL;

bool files_read = false;
bool checked_mods = false;
bool loading_game = false;

#define OBRN_VERSION_MAJOR 1
#define OBRN_VERSION_MINOR 0
#define OBRN_VERSION_REVISION 2

#define CompileFiles_Addr 0x0044F3D0
typedef int(__thiscall* CompileFiles_t)(DWORD*, char, char);
CompileFiles_t CompileFiles = NULL;

int __fastcall CompileFiles_Hook(DWORD* _this, void* _edx, char a2, char a3) {
	if (!checked_mods) {
		InitModExcludes();
		InitConfig();
		checked_mods = true;
	}
	int result = CompileFiles(_this, a2, a3);
	if (result) {
		fillUpWpRanges();
		fillUpClothingRanges();
		files_read = true;
		for (auto it : toRandomize) {
			randomize(it, __FUNCTION__);
		}
		toRandomize.clear();
		for (auto form : allAdded) {
			if (form == NULL || form->GetFormType() == kFormType_Creature || form->GetFormType() == kFormType_Spell) {
				continue;
			}
			allItems.push_back(form);
		}
		if (obrnFlag == NULL) {
			_ERROR("Couldn't find OBRN Flag in the loaded files. Some features will not work properly.");
		}
		allAdded.clear();
#ifdef _DEBUG
		UInt32 numArmorClothing = 0, numWeapons = 0, numGenericItems = 0, numCreatures = allCreatures.size(), numSpells = 0;
		const char* name = NULL;
		_MESSAGE("At the end of the list generation, we have:");
		for (const auto &it : allClothingAndArmor) {
			switch (it.first) {
			case kSlot_Head:
				name = "Helmets";
				break;
			case kSlot_LeftRing:
				name = "Rings";
				break;
			case kSlot_Shield:
				name = "Shields";
				break;
			case kSlot_Amulet:
				name = "Amulets";
				break;
			case kSlot_UpperBody:
				name = "Upper body";
				break;
			case kSlot_LowerBody:
				name = "Lower body";
				break;
			case kSlot_UpperHand:
				name = "Upper hand";
				break;
			case kSlot_UpperLower:
				name = "Upper lower";
				break;
			case kSlot_UpperLowerFoot:
				name = "Upper lower foot";
				break;
			case kSlot_UpperLowerHand:
				name = "Upper lower hand";
				break;
			case kSlot_UpperLowerHandFoot:
				name = "Upper lower hand foot";
				break;
			case kSlot_Foot:
				name = "Foot";
				break;
			case kSlot_Hand:
				name = "Hand";
				break;
			case kSlot_Weapon:
				name = "Weapon?";
				break;
			case kSlot_Torch:
				name = "Torch?";
				break;
			case kSlot_Quiver:
				name = "Quiver?";
				break;
			case kSlot_Tail:
				name = "Tail?";
				break;
			default:
				name = "Unknown";
				break;
			}
			_MESSAGE("(CLOTHING) %s (%i): %i", name, it.first, it.second.size());
			for (auto cloth : it.second) {
				_MESSAGE("\t%s", GetFullName(cloth));
			}
			numArmorClothing += it.second.size();
		}
		for (const auto &it : allWeapons) {
			switch (it.first) {
			case TESObjectWEAP::kType_BladeOneHand:
				name = "Blade";
				break;
			case TESObjectWEAP::kType_BluntOneHand:
				name = "Blunt";
				break;
			case TESObjectWEAP::kType_Bow:
				name = "Bow";
				break;
			case TESObjectWEAP::kType_Staff:
				name = "Staff";
				break;
			default:
				name = "Unknown Weapon";
				break;
			}
			_MESSAGE("(WEAPON) %s: %i", name, it.second.size());
			for (auto wp : it.second) {
				_MESSAGE("\t%s", GetFullName(wp));
			}
			numWeapons += it.second.size();
		}
		for (const auto &it : allGenericItems) {
			name = FormToString(it.first);
			_MESSAGE("(GENERIC): %s: %i", name, it.second.size());
			for (auto g : it.second) {
				_MESSAGE("\t%s", GetFullName(g));
			}
			numGenericItems += it.second.size();
		}
		for (const auto &it : allSpellsBySchool) {
			switch (it.first) {
			case EffectSetting::kEffect_Alteration:
				name = "Alteration";
				break;
			case EffectSetting::kEffect_Conjuration:
				name = "Conjuration";
				break;
			case EffectSetting::kEffect_Destruction:
				name = "Destruction";
				break;
			case EffectSetting::kEffect_Illusion:
				name = "Illusion";
				break;
			case EffectSetting::kEffect_Mysticism:
				name = "Mysticism";
				break;
			case EffectSetting::kEffect_Restoration:
				name = "Restoration";
				break;
			default:
				name = "Unknown";
				break;
			}
			_MESSAGE("(SPELL): %s: %i", name, it.second.size());
			for (auto spell : it.second) {
				_MESSAGE("\t%s", GetFullName(spell));
			}
			numSpells += it.second.size();
		}
		_MESSAGE("(CREATURE): %i", numCreatures);
		for (auto it : allCreatures) {
			_MESSAGE("\t%s", GetFullName(it));
		}
		if (oRandInventory) {
			_MESSAGE("There are %u total items", allItems.size());
		}
		_MESSAGE("There are %u weapons, %u generic items, %u pieces of clothing / armor, %u creatures and %u spells in the lists", 
			numWeapons, numGenericItems, numArmorClothing, numCreatures, numSpells);
#endif
	}
	return result;
}

#define ConstructObject_Addr 0x0044DCF0
typedef int(__thiscall* ConstructObject_t)(unsigned char*, int, char);
ConstructObject_t ConstructObject = NULL;

TESForm* order = NULL;

int __fastcall ConstructObject_Hook(unsigned char* _this, void* _edx, int a2, char a3) { //a3 == 1 -> reading from the first file? (master?)
	int result = ConstructObject(_this, a2, a3);
	if (result) {
		UInt32 refID = *((UInt32*)(a2 + 584));
		TESForm* form = LookupFormByID(refID);
		void* retAddress = _ReturnAddress();
		if (form == NULL) {
			return result;
		}
		if (form->IsReference()) {
			TESObjectREFR* ref = OBLIVION_CAST(form, TESForm, TESObjectREFR);
			if (ref != NULL && ref->baseForm != NULL && 
				((oRandCreatures && ref->GetFormType() == kFormType_ACRE) || (oWorldItems && refIsItem(ref)))) {
				if (files_read) {
					randomize(ref, __FUNCTION__);
				}
				else {
					toRandomize.push_back(ref);
				}
			}
		}
		else {
			if (!files_read && retAddress == (void*)0x0044F221) { //called by a function that loads forms from an esp/esm
				if (form->GetFormType() == kFormType_Creature || form->GetFormType() == kFormType_NPC) {
					//debugDumpSpells(form);
				}
				tryToAddForm(form);
			}
		}
	}
	return result;
}

/*
#define CalcLevList_Addr 0x0046CC70
typedef int(__thiscall* CalcLevList_t)(BYTE*, int, DWORD*, WORD*, DWORD*);
CalcLevList_t CalcLevList = NULL;

int __fastcall CalcLevList_Hook(BYTE* _this, void* _edx, int a2, DWORD* a3, WORD* a4, DWORD* a5) {
	int result = CalcLevList(_this, a2, a3, a4, a5);
	if (oRandCreatures && result) {
		TESForm* form = *(TESForm**)a3;
		if (form != NULL) {
			if (form->GetFormType() == kFormType_Creature && allCreatures.size()) {
				TESForm** a3_real = (TESForm**)a3;
				TESForm* rando = LookupFormByID(allCreatures[rand() % allCreatures.size()]);
#ifdef _DEBUG
				FILE* f = fopen("randomization.log", "a");
				fprintf(f, "%s: Going to randomize %s %08X into %s %08X\n", __FUNCTION__, GetFullName(*a3_real), (*a3_real)->refID, GetFullName(rando), rando->refID);
				fclose(f);
#endif
				*a3_real = rando;
			}
		}
	}
	return result;
}
*/

#define AddItem_Addr 0x00469D10
typedef int(__thiscall* AddItem_t)(int, TESForm*, int, char);
AddItem_t AddItem = NULL;

int __fastcall AddItem_Hook(int _this, void* _edx, TESForm* a2, int a3, char a4) {
	void* retAddress = _ReturnAddress();
	if (oAddItems && !IsConsoleOpen() && retAddress == (void*)0x00507419 /*called within a script*/ ) {
		if (TESForm * replacement = getRandomBySetting(a2, oAddItems)) {
			a2 = replacement;
		}
	}
	return AddItem(_this, a2, a3, a4);
}

#define LoadForm_Addr 0x004603E0
typedef TESForm* (__stdcall* LoadForm_t)(int, UInt32*);
LoadForm_t LoadForm = NULL;

TESForm* __stdcall LoadForm_Hook(int a1, UInt32* a2) {
	TESForm* result = LoadForm(a1, a2);
	if (oRandCreatures > 1 && result != NULL) {
		if (result->GetFormType() == kFormType_ACRE && (a1 >> 24) == 0xFF) {
			TESObjectREFR* ref = OBLIVION_CAST(result, TESForm, TESObjectREFR);
			restoreFlags.insert(std::make_pair(ref, ref->flags));
			randomize(ref, __FUNCTION__);
			ref->Update3D();
		}
	}
	return result;
}

#define LoadObject_Addr 0x006022F0
typedef unsigned int(__thiscall* LoadObject_t)(DWORD*, int, int);
LoadObject_t LoadObject = NULL;

unsigned int __fastcall LoadObject_Hook(DWORD* _this, void* _edx, int a2, int a3) {
	for (auto it = restoreFlags.begin(); it != restoreFlags.end(); ++it) {
		if ((it->second & TESObjectREFR::kFlags_Persistent) != (it->first->flags & TESObjectREFR::kFlags_Persistent)) {
			it->first->flags ^= TESObjectREFR::kFlags_Persistent;
		}
		//_MESSAGE("Old flags: %i, new flags - old flags = %i\n", it->second, it->first->flags & ~it->second);
		//it->first->flags |= (it->second & ~(0x00200000 | TESObjectREFR::kFlags_Persistent));
		//this is a problem, because i do not know if it should be left in or not
		//there are plenty of flags that are not documented and determining what
		//they do does not seem trivial
	}
	restoreFlags.clear();
	unsigned int result = LoadObject(_this, a2, a3);
	return result;
}

#define CalcLevListOuter_Addr 0x0046CDE0
typedef void(__thiscall* CalcLevListOuter_t)(TESLeveledList*, int, DWORD*, int);
CalcLevListOuter_t CalcLevListOuter = NULL;

void __fastcall CalcLevListOuter_Hook(TESLeveledList* _this, void* _edx, int a2, DWORD* a3, int a4) {
	//this = TESLeveledList
	//a3 = count
	//a4 + 8 = list of returned tesforms
	void* retAddress = _ReturnAddress();
	CalcLevListOuter(_this, a2, a3, a4);
	TESForm* lev = (TESForm*)((UInt32)_this - 36);
	bool deathItem = oDeathItems && retAddress == (void*)0x005EA464,
		creatureSpawn = allCreatures.size() && oRandCreatures && lev->GetFormType() == kFormType_LeveledCreature,
		scriptAddItem = oAddItems && retAddress == (void*)0x005073FA /*called within a script*/ && lev->GetFormType() == kFormType_LeveledItem;
	if (a4 && (deathItem || creatureSpawn || scriptAddItem)) {
		LevListResult_t* result = (LevListResult_t*)(a4 + 8);
		while (result != NULL) {
			if (result->data != NULL) {
				if (deathItem || scriptAddItem) {
					if (TESForm* ref = getRandomBySetting(result->data->item, deathItem ? oDeathItems : oAddItems)) {
						result->data->item = ref;
					}
				}
				else if (result->data->item->GetFormType() == kFormType_Creature) {
					TESForm* rando = allCreatures[rng(0, allCreatures.size() - 1)], * old = result->data->item;
#ifdef _DEBUG
					_MESSAGE("%s: Going to randomize %s %08X into %s %08X", __FUNCTION__, GetFullName(old), old->refID, GetFullName(rando), rando->refID);
#endif
					result->data->item = rando;
				}
			}
			result = result->next;
		}
	}
}

//465D57 is the last addr where its still good
#define LoadGame_Addr 0x00465860
typedef char(__thiscall* LoadGame_t)(int, int, int, char);
LoadGame_t LoadGame = NULL;

char __fastcall LoadGame_Hook(int _this, void* _edx, int a2, int a3, char a4) {
	//long story short, certain creatures (possibly only the ones with starting health being 0 that didn't have any items taken off them)
	//have their inventory reset somewhere between 0x00465D57 and the end of the LoadGame function if the game was restarted
	//it would be possible to track down the exact spot where it gets reset but
	//a) this hacky solution seems to work
	//b) i was meant to release this mod 3 months ago
	//c) it would take time
	loading_game = true;
	char result = LoadGame(_this, a2, a3, a4);
	loading_game = false;
	for (auto it : toRandomize) {
		randomize(it, __FUNCTION__);
	}
	toRandomize.clear();
	return result;
}

//char __thiscall sub_5E0990(_DWORD** this, int a2)
#define AddSpell_Addr 0x005E0990
typedef char(__thiscall* AddSpell_t)(Actor*, SpellItem*);
AddSpell_t AddSpell = NULL;

bool opened = false;

std::set<void*> calls;

char __fastcall AddSpell_Hook(Actor* _this, void* _edx, SpellItem* spell) {
	if (spell != NULL) {
		if (!opened) {
			remove("AddSpell_Hook.txt");
			opened = true;
		}
		FILE* f = fopen("AddSpell_Hook.txt", "a");
		fprintf(f, "Adding %s to %s\n", GetFullName(spell), GetFullName(_this));
		fclose(f);
		spell = OBLIVION_CAST(allSpells[rng(0, allSpells.size() - 1)], TESForm, SpellItem);
	}
	return AddSpell(_this, spell);
}

//char __thiscall sub_6646D0(_DWORD** this, int a2)
#define AddSpellOuter_Addr 0x006646D0
typedef char(__thiscall* AddSpellOuter_t)(Actor*, SpellItem*);
AddSpellOuter_t AddSpellOuter = NULL;

char __fastcall AddSpellOuter_Hook(Actor* _this, void* _edx, SpellItem* spell) {
	spell = OBLIVION_CAST(allSpells[rng(0, allSpells.size() - 1)], TESForm, SpellItem);
	char result = AddSpellOuter(_this, spell);
	void* retAddress = _ReturnAddress();
	/*if (!calls.contains(retAddress)) {
		calls.insert(retAddress);
		FILE* f = fopen("AddSpell_calls.txt", "a");
		fprintf(f, "Ret: %08X, Actor: %s, Spell: %s\n", retAddress, GetFullName(_this), GetFullName(spell));
		fclose(f);
	}*/
	return result;
}

//int __thiscall sub_5F3E00(_DWORD* this)
#define CastSpell_Addr 0x005F3E00
typedef int(__thiscall* CastSpell_t)(DWORD*);

CastSpell_t CastSpell = NULL;

int __fastcall CastSpell_Hook(DWORD* _this, void* _edx) {
	int result = CastSpell(_this);
	TESForm* caster = (TESForm*)((DWORD*)_this - 23);
	//FILE* f = fopen(__FUNCTION__, "a");
	//fprintf(f, "cast: _this: %08X, caster: %s result: %08X %u %i, ret: %08X\n", _this, GetFullName(caster), result, result, result, _ReturnAddress());
	//fclose(f);
	//TESLevSpell;
	return result;
}

//char __thiscall sub_699190(_DWORD *this, _DWORD *a2, int a3, int a4)
#define CastSpellOuter_Addr 0x00699190
typedef char(__thiscall* CastSpellOuter_t)(DWORD*, MagicItem*, int, int);
CastSpellOuter_t CastSpellOuter = NULL;

std::map<SpellItem*, MagicItem*> spellMapping;

char __fastcall CastSpellOuter_Hook(DWORD* _this, void* _edx, MagicItem* a2, int a3, int a4) {
	//_this - 23 = caster
	TESForm* caster = (TESForm*)(DWORD*)(_this - 23);
	SpellItem* spell = OBLIVION_CAST(a2, MagicItem, SpellItem);
	if (spell != NULL && allSpells.size()) {
		if (!spellMapping.contains(spell)) {
			spellMapping.insert(std::make_pair(spell, OBLIVION_CAST(allSpells[rand() % allSpells.size()], TESForm, MagicItem)));
			FILE* f = fopen(__FUNCTION__".txt", "a");
			fprintf(f, "Spell %s will now become %s. Caster type is %s\n", GetFullName(spell), GetFullName(OBLIVION_CAST(spellMapping.at(spell), MagicItem, TESForm)), FormToString(caster->GetFormType()));
			fclose(f);
		}
		a2 = spellMapping.at(spell);

	}
	char result = CastSpellOuter(_this, a2, a3, a4);
	/*FILE* f = fopen(__FUNCTION__, "a");
	fprintf(f, "_this: %08X, a2: %s (%08X), a3: %08X %u %i, a4: %08X %u %i: result: %u, ret: %08X\n", _this, a2->name.m_data, a2, a3, a3, a3, a4, a4, a4, result, _ReturnAddress());
	fclose(f);*/
	return result;
}

void InitHooks() {
	InitTrampHook(ConstructObject, 8);
	InitTrampHook(CompileFiles, 8);
	//InitTrampHook(CalcLevList, 5);
	InitTrampHook(LoadForm, 7);
	InitTrampHook(AddItem, 6);
	InitTrampHook(LoadObject, 11);
	InitTrampHook(CalcLevListOuter, 7);

	InitTrampHook(LoadGame, 7);
	//InitTrampHook(AddSpell, 7);
	//InitTrampHook(AddSpellOuter, 7);
	//InitTrampHook(CastSpell, 7);
	//InitTrampHook(CastSpellOuter, 7);
}

unsigned int getNumItems(ItemMapPtr map) {
	unsigned int num = 0;
	for (auto it = map->begin(); it != map->end(); ++it) {
		num += (*it).second.size();
	}
	return num;
}

/**********************
* Command handlers
**********************/

#if OBLIVION

bool Cmd_OBRNListsReady_Execute(COMMAND_ARGS) {
	static bool stats = false; //there's no point in spamming it every reload
	*result = allCreatures.size() && allClothingAndArmor.size() && allGenericItems.size() && allWeapons.size() ? 1.0 : 0.0;
	if (!stats) {
		Console_Print("Randomizer's Lists Info:");
		Console_Print("Number of creatures: %u", allCreatures.size());
		Console_Print("Number of clothing/armor: %u", getNumItems(&allClothingAndArmor));
		Console_Print("Number of generic items: %u", getNumItems(&allGenericItems));
		Console_Print("Number of weapons: %u", getNumItems(&allWeapons));
		Console_Print("Number of spells: %u", allSpells.size());
		stats = true;
	}
	return true;
}

bool Cmd_OBRNRandomize_Execute(COMMAND_ARGS) {
	TESObjectREFR* ref = NULL;
	*result = 0.0;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &ref)) {
		randomize(ref, "ESP");
	}
	return true;
}

#endif
/**************************
* Command definitions
**************************/
DEFINE_COMMAND_PLUGIN(OBRNRandomize, "Randomizes the passed object", 0, 1, kParams_OneObjectRef);
DEFINE_COMMAND_PLUGIN(OBRNListsReady, "Returns 1 if the lists are prepared", 0, 0, {});

extern "C" {

bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "obrandomizer";
	info->version = (OBRN_VERSION_MAJOR << 24) | (OBRN_VERSION_MINOR << 16) | (OBRN_VERSION_REVISION << 8);

	// version checks
	if(!obse->isEditor)
	{
		if(obse->obseVersion < OBSE_VERSION_INTEGER)
		{
			_ERROR("OBSE version too old (got %u expected at least %u)", obse->obseVersion, OBSE_VERSION_INTEGER);
			return false;
		}

#if OBLIVION
		if(obse->oblivionVersion != OBLIVION_VERSION)
		{
			_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
			return false;
		}
#endif

		g_serialization = (OBSESerializationInterface *)obse->QueryInterface(kInterface_Serialization);
		if(!g_serialization)
		{
			_ERROR("serialization interface not found");
			return false;
		}

		if(g_serialization->version < OBSESerializationInterface::kVersion)
		{
			_ERROR("incorrect serialization version found (got %08X need %08X)", g_serialization->version, OBSESerializationInterface::kVersion);
			return false;
		}

		g_arrayIntfc = (OBSEArrayVarInterface*)obse->QueryInterface(kInterface_ArrayVar);
		if (!g_arrayIntfc)
		{
			_ERROR("Array interface not found");
			return false;
		}

		g_scriptIntfc = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);		
	}
	else
	{
		// no version checks needed for editor
	}

	// version checks pass

	return true;
}

bool OBSEPlugin_Load(const OBSEInterface * obse)
{
	_MESSAGE("load");

	g_pluginHandle = obse->GetPluginHandle();

	/***************************************************************************
	 *	
	 *	READ THIS!
	 *	
	 *	Before releasing your plugin, you need to request an opcode range from
	 *	the OBSE team and set it in your first SetOpcodeBase call. If you do not
	 *	do this, your plugin will create major compatibility issues with other
	 *	plugins, and may not load in future versions of OBSE. See
	 *	obse_readme.txt for more information.
	 *	
	 **************************************************************************/

	// register commands
	obse->SetOpcodeBase(0x2500); //0x2500 to 0x2507
	obse->RegisterCommand(&kCommandInfo_OBRNRandomize);
	obse->RegisterCommand(&kCommandInfo_OBRNListsReady);

	// set up serialization callbacks when running in the runtime
	if(!obse->isEditor)
	{

		// register to use string var interface
		// this allows plugin commands to support '%z' format specifier in format string arguments
		OBSEStringVarInterface* g_Str = (OBSEStringVarInterface*)obse->QueryInterface(kInterface_StringVar);
		g_Str->Register(g_Str);

		// get an OBSEScriptInterface to use for argument extraction
		g_scriptInterface = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);
		InitHooks();
	}

	// get command table, if needed
	OBSECommandTableInterface* cmdIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);
	if (cmdIntfc) {
#if 0	// enable the following for loads of log output
		for (const CommandInfo* cur = cmdIntfc->Start(); cur != cmdIntfc->End(); ++cur) {
			_MESSAGE("%s",cur->longName);
		}
#endif
	}
	else {
		_MESSAGE("Couldn't read command table");
	}
	return true;
}

};
