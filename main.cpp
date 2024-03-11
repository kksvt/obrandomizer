#include "randomizer.h"
#include "obse_common/SafeWrite.h"

IDebugLog		gLog("obrandomizer.log");

OBSEScriptInterface* g_scriptInterface = NULL;	// make sure you assign to this
PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;
OBSEScriptInterface			* g_scriptIntfc = NULL;

bool files_read = false;
bool checked_mods = false;

#define OBRN_VERSION_MAJOR 1
#define OBRN_VERSION_MINOR 0
#define OBRN_VERSION_REVISION 2

#define CompileFiles_Addr 0x0044F3D0
typedef int(__thiscall* CompileFiles_t)(DWORD*, char, char);
CompileFiles_t CompileFiles = NULL;

int __fastcall CompileFiles_Hook(DWORD* _this, void* _edx, char a2, char a3) {
	if (!checked_mods) {
		InitModExcludes();
		checked_mods = true;
	}
	int result = CompileFiles(_this, a2, a3);
	if (result) {
		fillUpWpRanges();
		fillUpClothingRanges();
		for (auto it : allAdded) {
			TESForm* form = LookupFormByID(it);
			if (form == NULL || form->GetFormType() == kFormType_Creature || form->GetFormType() == kFormType_Spell) {
				continue;
			}
			allItems.push_back(it);
		}
		files_read = true;
		for (auto it : toRandomize) {
			randomize(it, __FUNCTION__);
		}
		toRandomize.clear();
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
				_MESSAGE("\t%s", GetFullName(LookupFormByID(cloth)));
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
				_MESSAGE("\t%s", GetFullName(LookupFormByID(wp)));
			}
			numWeapons += it.second.size();
		}
		for (const auto &it : allGenericItems) {
			name = formTypeToString(it.first);
			_MESSAGE("(GENERIC): %s: %i", name, it.second.size());
			for (auto g : it.second) {
				_MESSAGE("\t%s", GetFullName(LookupFormByID(g)));
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
				_MESSAGE("\t%s", GetFullName(LookupFormByID(spell)));
			}
			numSpells += it.second.size();
		}
		_MESSAGE("(CREATURE): %i", numCreatures);
		for (auto it : allCreatures) {
			_MESSAGE("\t%s", GetFullName(LookupFormByID(it)));
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
				tryToAddForm(form);
			}
		}
	}
	return result;
}

#define AddItem_Addr 0x00469D10
typedef int(__thiscall* AddItem_t)(int, TESForm*, int, char);
AddItem_t AddItem = NULL;

int __fastcall AddItem_Hook(int _this, void* _edx, TESForm* a2, int a3, char a4) {
	void* retAddress = _ReturnAddress();
	if (oAddItems /* && !IsConsoleOpen()*/ && retAddress == (void*)0x00507419 /*called within a script*/) {
		if (TESForm * replacement = getRandomBySetting(a2, oAddItems)) {
			a2 = replacement;
		}
	}
	return AddItem(_this, a2, a3, a4);
}

#define LoadForm_Addr 0x004603E0
typedef TESForm* (__stdcall* LoadForm_t)(UInt32, UInt32*);
LoadForm_t LoadForm = NULL;


TESForm* __stdcall LoadForm_Hook(UInt32 a1, UInt32* a2) {
#ifdef _DEBUG_LOADFORM
	static int calls = 0;
	FILE* f = fopen(__FUNCTION__"_calls.txt", "a");
	TESForm* fa1 = LookupFormByID(a1);
	TESForm* fa2 = LookupFormByID(a2[1]);
	const char* a1_name = GetFullName(fa1);
	const char* a2_name = GetFullName(fa2);
	const char* a1_form = "invalid", * a2_form = "invalid";
	if (fa1 != NULL && fa2 != NULL) {
		a1_form = formIDToString(fa1->GetFormType());
		TESForm* v3 = fa1, * v4 = fa2;
		TESBoundObject* v2 = OBLIVION_CAST(v4, TESForm, TESBoundObject);
		TESObjectREFR* v6 = OBLIVION_CAST(v3, TESForm, TESObjectREFR);
		TESBoundObject* bound = (*(TESBoundObject*(__thiscall**)(TESObjectREFR*))(*(DWORD*)v6 + 368))(v6);
		fprintf(f, "v2 = %08X %s %08X %s, bound = %08X %s %08X %s, the player: %08X %08X, v4: %08X, v3: %08X\n", 
			v2, GetFullName(v2), v2->refID, formIDToString(v2->GetFormType()),
			bound, GetFullName(bound), bound->refID, formIDToString(bound->GetFormType()), 
			*g_thePlayer, (*g_thePlayer)->refID, v4, v3);
		//this crashes if we attempt to load the player's character
		//the bound object has a proper full name, but the refID is 0. why is it not initialized properly?
		//returning here prevents crash on the second game load, but then the loadform
		//appears to have duplicate calls for the same object and causes crash on subsequent reloads
		fclose(f);
		//return v3;
	}
	if (fa2 != NULL) {
		a2_form = formIDToString(fa2->GetFormType());
	}
	//fprintf(f, "%d: a1: %08X, a2: %08X, a2[0]: %08X, a2[1]: %08X, a1 form: %s (%s), a2[1] form: %s (%s)\n", ++calls, a1, a2, a2[0], a2[1], 
	//	a1_name, a1_form, a2_name, a2_form);
	fclose(f);
	/*if (fa1 != NULL) {
		FormHeap_Free(fa1);
	}*/
#endif
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
					TESForm* rando = LookupFormByID(allCreatures[rng(0, allCreatures.size() - 1)]), * old = result->data->item;
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

//char __thiscall sub_6646D0(_DWORD** this, int a2)
#define AddSpellOuter_Addr 0x006646D0
typedef char(__thiscall* AddSpellOuter_t)(Actor*, SpellItem*);
AddSpellOuter_t AddSpellOuter = NULL;

char __fastcall AddSpellOuter_Hook(Actor* _this, void* _edx, SpellItem* spell) {
	static char randMsg[256] = "\0";
	if ((void*)_this == (void*)(*g_thePlayer) && !IsConsoleOpen()) {
		TESForm* rando = getRandomBySetting(OBLIVION_CAST(spell, SpellItem, TESForm), oRandSpells);
		if (rando != NULL) {
#ifdef _DEBUG
			_MESSAGE("%s: Spell %s will now become %s, ret: %08X", __FUNCTION__, GetFullName(spell), GetFullName(rando), _ReturnAddress());
#endif
			sprintf_s(randMsg, sizeof(randMsg), "Spell %s has been randomized into %s", GetFullName(spell), GetFullName(rando));
			spell = OBLIVION_CAST(rando, TESForm, SpellItem);
			QueueUIMessage_2(randMsg, 2.0f, NULL, NULL); 
			//this is necessary (or well, welcome) since randomizing the spell within this function will still print out
			//the old spell name. to circumvent that, we'd either have to hook LookupFormByID and keep a spell mapping there 
			//or hook the function that parses the "AddSpell" script command itself (possibly 0x00514950?) 
		}
	}
	char result = AddSpellOuter(_this, spell);
	return result;
}

//char __thiscall sub_699190(_DWORD *this, _DWORD *a2, int a3, int a4)
#define CastSpellOuter_Addr 0x00699190
typedef char(__thiscall* CastSpellOuter_t)(DWORD*, MagicItem*, int, int);
CastSpellOuter_t CastSpellOuter = NULL;

std::unordered_map<SpellItem*, MagicItem*> spellMapping;

char __fastcall CastSpellOuter_Hook(DWORD* _this, void* _edx, MagicItem* a2, int a3, int a4) {
	//_this - 23 = caster
	TESForm* caster = (TESForm*)(DWORD*)(_this - 23);
	SpellItem* spell = OBLIVION_CAST(a2, MagicItem, SpellItem);
	if ((void*)(*g_thePlayer) != (void*)caster && spell != NULL && allSpells.size()) {
		if (!spellMapping.contains(spell)) {
			TESForm *rando = getRandomBySetting(OBLIVION_CAST(a2, MagicItem, TESForm), oRandSpells);
			if (rando != NULL) {
				spellMapping.insert(std::make_pair(spell, OBLIVION_CAST(rando, TESForm, MagicItem)));
#ifdef _DEBUG
			_MESSAGE("%s: Spell %s will now become %s. Caster is %s %08X (%s %08X)", __FUNCTION__, 
				GetFullName(spell), GetFullName(OBLIVION_CAST(spellMapping.at(spell), MagicItem, TESForm)), 
				GetFullName(caster), caster->refID, formTypeToString(caster->GetFormType()), caster);
#endif
			}
		}
		a2 = spellMapping.at(spell);
	}
	char result = CastSpellOuter(_this, a2, a3, a4);
	return result;
}

void InitHooks() {
	_MESSAGE("Initializing ConstructObject and CompileFiles hooks...");
	InitTrampHook(ConstructObject, 8);
	InitTrampHook(CompileFiles, 8);
	if (oRandCreatures > 1) {
		_MESSAGE("oRandCreatures: %i, initializing LoadForm and LoadObject hooks...\nWARNING: This setting is highly unstable", oRandCreatures);
		//this hook's existence causes crashes on reloading as it appears to prevent the existing player or other reference forms
		//from being removed from the memory. a possible threading issue, as the pseudocode prints out the warning below
		// based on certain values from GetCurrentThreadId()
		//"2024/02/24 04:05:08  [0045C7CC] [WARNING]   DeleteForm() was called, but the game is not being loaded."
		InitTrampHook(LoadForm, 7);
		InitTrampHook(LoadObject, 11);
		//i dont think delving into it is worth it though, oRandCreatures being set to 2 is not a recommended setting
	}
	if (oAddItems || oDeathItems) {
		_MESSAGE("oAddItems: %i, oDeathItems: %i, initializing the AddItem hook...", oAddItems, oDeathItems);
		InitTrampHook(AddItem, 6);
	}
	if (oAddItems || oDeathItems || oRandCreatures) {
		_MESSAGE("oAddItems: %i, oDeathItems: %i, oRandCreatures: %i, initializing the CalcLevListOuter hook...", oAddItems, oDeathItems, oRandCreatures);
		InitTrampHook(CalcLevListOuter, 7);
	}
	if (oRandSpells) {
		_MESSAGE("oRandSpells: %i, initializing the AddSpellOuter and CastSpellOuter hooks...", oRandSpells);
		InitTrampHook(AddSpellOuter, 7);
		InitTrampHook(CastSpellOuter, 7);
	}
}

unsigned int getNumItems(ItemMapPtr map) {
	unsigned int num = 0;
	for (const auto &it: *map) {
		num += it.second.size();
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
		InitConfig();
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
