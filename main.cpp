#include "randomizer.h"
#include "utils.h"
#include "rng.h"
#include "obse_common/SafeWrite.h"

IDebugLog		gLog("obrandomizer.log");

OBSEScriptInterface* g_scriptInterface = NULL;	// make sure you assign to this
PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;
OBSEScriptInterface			* g_scriptIntfc = NULL;

bool files_read = false;
bool checked_mods = false;

OblivionRng rng;
OblivionCfg config;

#define OBRN_VERSION_MAJOR 1
#define OBRN_VERSION_MINOR 1
#define OBRN_VERSION_REVISION 0

#define CompileFiles_Addr 0x0044F3D0
typedef int(__thiscall* CompileFiles_t)(DWORD*, char, char);
CompileFiles_t CompileFiles = NULL;

int __fastcall CompileFiles_Hook(DWORD* _this, void* _edx, char a2, char a3) {
	if (!checked_mods) {
		config.ReadExcludesFromFile("Data/RandomizerSkip.cfg");
		checked_mods = true;
	}
	int result = CompileFiles(_this, a2, a3);
	if (result) {
		fillUpWpRanges();
		fillUpClothingRanges();
		for (auto it : allAdded) {
			TESForm* form = LookupFormByID(it);
			if (!form || form->GetFormType() == kFormType_Creature || form->GetFormType() == kFormType_Spell) {
				continue;
			}
			allItems.push_back(it);
		}
		files_read = true;
#ifdef _DEBUG
		_MESSAGE("Files have been read.");
#endif
		for (auto it : toRandomize) {
			randomize(it, __FUNCTION__);
		}
		toRandomize.clear();
		if (!obrnFlag) {
			_ERROR("Couldn't find OBRN Flag in the loaded files. Some features will not work properly.");
		}
		allAdded.clear();
#ifdef _DEBUG
		logDetailedListInfo();
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
		if (!form) {
			return result;
		}
		if (form->IsReference()) {
			TESObjectREFR* ref = OBLIVION_CAST(form, TESForm, TESObjectREFR);
			if (ref && ref->baseForm && 
				((config.oRandCreatures && ref->GetFormType() == kFormType_ACRE) || 
					(config.oWorldItems && refIsItem(ref)))) {
				if (files_read) {
					randomize(form, __FUNCTION__);
				}
				else {
					toRandomize.push_back(form);
				}
			}
		}
		else {
			if (!files_read && retAddress == (void*)0x0044F221) { //called by a function that loads forms from an esp/esm
				tryToAddForm(form);
			}
			if (form->GetFormType() == kFormType_Spell) {
				if (files_read) {
					randomize(form, __FUNCTION__);
				}
				else {
					toRandomize.push_back(form);
				}
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
	if (config.oAddItems && !IsConsoleOpen() && retAddress == (void*)0x00507419 /*called within a script*/) {
		if (TESForm * replacement = getRandomBySetting(a2, config.oAddItems, true)) {
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
	if (config.oRandCreatures > 1 && result) {
		if (result->GetFormType() == kFormType_ACRE && (a1 >> 24) == 0xFF) {
			TESObjectREFR* ref = OBLIVION_CAST(result, TESForm, TESObjectREFR);
			restoreFlags.insert(std::make_pair(ref, ref->flags));
			randomize(ref, __FUNCTION__);
			ref->Update3D();
		}
	}
	return result;
}

/*
here edi should be equal to form id?
.text:00465C9A                 add     esp, 18h
.text:00465C9D                 cmp     edi, 0FEFFFFFFh
.text:00465CA3                 jnz     short loc_465D04
.text:00465CA5                 fldz
.text:00465CA7                 mov     edx, [esi+4]
.text:00465CAA                 push    ebx
*/

static const UInt32 LoadGameHookStart = 0x00465C9A;
static const UInt32 LoadGameHookReturn = 0x00465CAA;
static const UInt32 LoadGameJump = 0x00465D04;
//static const UInt32 LoadGameSkip = 0x0046686C;

static TESForm* LoadGameForm;
static UInt32 _edi;

static __declspec(naked) void LoadGamePatch(void) {
	__asm {
		pushad
		mov _edi, edi
	}

	LoadGameForm = LookupFormByID(_edi);
	
	__asm {
		popad
		add esp, 18h
		cmp edi, 0FEFFFFFFh
		jnz loc_465D04
		fldz
		mov edx, [esi + 4]
		jmp[LoadGameHookReturn]

		loc_465D04:
		jmp[LoadGameJump]
	}

}

/*
.text:00602709                 cmp     word ptr [esp+24h+arg_0], bx <- hook here
.text:0060270E                 jbe     short loc_602788
.text:00602710 <- return to this
.text:00602710 loc_602710:
*/
static const UInt32 LoadObjectStart = 0x00602709;
static const UInt32 LoadObjectReturn = 0x00602710;
static const UInt32 LoadObjectJump = 0x00602788;
static const UInt32 LoadObjectCancel = 0x00602788;
static WORD LoadObjectDataLength = 0;
static UInt32 _ebx;

static __declspec(naked) void LoadObjectPatch(void) {
	_asm { //fixme: is there a prettier way to do this?
		mov _ebx, ebx
		mov bx, word ptr[esp + 28h]
		mov LoadObjectDataLength, bx //msvc doesnt allow "mov LoadObjectDataLength, word ptr[esp + 28h]"
		mov ebx, _ebx
		pushad
	}

	if (LoadObjectDataLength > 0 &&
		LoadGameForm && LoadGameForm->GetFormType() == kFormType_ACRE) {
		//i dont know what data is loaded from the save file here,
		//but according to my research (i.e. logging LoadObjectDataLength
		//for several vanilla-ish saves) this should be non-zero only for actors.
		
		//in a seemingly corrupted save, this would take incredibly high values
		//for horses (up to several thousands), low to medium values for sheep
		//and almost 2k for a starving mountain lion, with the aforementioned lion
		//crashing the game. i suspect it has something to do with non-horse creatures
		//being randomized into horse creatures and vice versa, so we will exclude it
		//from the mod for the time being
		LoadGameForm = NULL; //clean up, just to be safe
		_asm {
			popad
			jmp [LoadObjectCancel]
		}
	}

	LoadGameForm = NULL; //clean up, just to be safe

	_asm {
		popad
		cmp     word ptr[esp + 28h], bx
		jbe     short loc_602788
		jmp [LoadObjectReturn]

		loc_602788:
			jmp [LoadObjectJump]
	}
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
	bool deathItem = config.oDeathItems && retAddress == (void*)0x005EA464,
		creatureSpawn = allCreatures.size() && config.oRandCreatures && lev->GetFormType() == kFormType_LeveledCreature,
		scriptAddItem = config.oAddItems && retAddress == (void*)0x005073FA /*called within a script*/ && 
		lev->GetFormType() == kFormType_LeveledItem;
	if (a4 && (deathItem || creatureSpawn || scriptAddItem)) {
		LevListResult_t* result = (LevListResult_t*)(a4 + 8);
		while (result) {
			if (result->data) {
				if (deathItem || scriptAddItem) {
					if (TESForm* ref = getRandomBySetting(result->data->item, 
						deathItem ? config.oDeathItems : config.oAddItems, scriptAddItem ? true : false)) {
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
	void* retAddress = _ReturnAddress();
	if ((void*)_this == (void*)(*g_thePlayer) && !IsConsoleOpen() && retAddress == (void*)0x005149FB /*called within a script*/ &&
		!spellBlacklisted(spell)) {
		TESForm* rando = getRandomBySetting(OBLIVION_CAST(spell, SpellItem, TESForm), config.oRandSpells, false);
		if (rando) {
#ifdef _DEBUG
			_MESSAGE("%s: Spell %s will now become %s, ret: %08X", __FUNCTION__, GetFullName(spell), GetFullName(rando), retAddress);
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

char __fastcall CastSpellOuter_Hook(DWORD* _this, void* _edx, MagicItem* a2, int a3, int a4) {
	//_this - 23 = caster
	TESForm* caster = (TESForm*)(DWORD*)(_this - 23);
	if ((void*)(*g_thePlayer) != (void*)caster && a2) {
		auto it = spellMapping.find(a2);
		if (it != spellMapping.end()) {
			//Console_Print("%s => %s, %i %i", a2->name.m_data, it->second->name.m_data, a3, a4);
			a2 = it->second;
		}
	}
	char result = CastSpellOuter(_this, a2, a3, a4);
	return result;
}

//im not sure what this function does, but it appears to have something to
//do with loading textures. it sometimes causes a crash on loading a save.
//i shouldnt be doing this, but installing this hook at least allows you
//to load the saved game
/*bool __thiscall sub_4AC730(_BYTE* this, unsigned __int8 a2)
{
	return (a2 & this[24]) != 0;
}*/

#define CrashFix_Addr 0x004AC730
typedef bool(__thiscall* CrashFix_t)(BYTE*, UInt8 a2);
CrashFix_t CrashFix = NULL;

bool __fastcall CrashFix_Hook(BYTE* _this, void* _edx, UInt8 a2) {
	return 0;
}

void InitHooks() {
	_MESSAGE("Initializing ConstructObject and CompileFiles hooks...");
	InitTrampHook(ConstructObject, 8);
	InitTrampHook(CompileFiles, 8);
	if (config.oRandCreatures > 1) {
		_MESSAGE("oRandCreatures: %i, initializing LoadForm and LoadObject hooks...\nWARNING: This setting is highly unstable", 
			config.oRandCreatures);
		//this hook's existence causes crashes on reloading as it appears to prevent the existing player or other reference forms
		//from being removed from the memory. a possible threading issue, as the pseudocode prints out the warning below
		// based on certain values from GetCurrentThreadId()
		//"2024/02/24 04:05:08  [0045C7CC] [WARNING]   DeleteForm() was called, but the game is not being loaded."
		InitTrampHook(LoadForm, 7);
		InitTrampHook(LoadObject, 11);
		//i dont think delving into it is worth it though, oRandCreatures being set to 2 is not a recommended setting
	}
	if (config.oAddItems || config.oDeathItems) {
		_MESSAGE("oAddItems: %i, oDeathItems: %i, initializing the AddItem hook...", config.oAddItems, config.oDeathItems);
		InitTrampHook(AddItem, 6);
	}
	if (config.oAddItems || config.oDeathItems || config.oRandCreatures) {
		_MESSAGE("oAddItems: %i, oDeathItems: %i, oRandCreatures: %i, initializing the CalcLevListOuter hook...", 
			config.oAddItems, config.oDeathItems, config.oRandCreatures);
		InitTrampHook(CalcLevListOuter, 7);
	}
	if (config.oRandSpells) {
		_MESSAGE("oRandSpells: %i, initializing the AddSpellOuter and CastSpellOuter hooks...", config.oRandSpells);
		InitTrampHook(AddSpellOuter, 7);
		InitTrampHook(CastSpellOuter, 7);
	}
	if (config.oInstallCrashFix & 1) {
		InitTrampHook(CrashFix, 5);
	}
	
	if (config.oInstallCrashFix & 2) {
		WriteRelJump(LoadGameHookStart, (UInt32)&LoadGamePatch);
		WriteRelJump(LoadObjectStart, (UInt32)&LoadObjectPatch);
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

bool Cmd_OBRNGetSetting_Execute(COMMAND_ARGS) {
	char s[512];
	OblivionCfgValueType type;
	SInt32 offset;
	*result = -1.0f;
	if (ExtractArgsEx(paramInfo, arg1, opcodeOffsetPtr, scriptObj, eventList, &s)) {
		type = config.GetSettingType(s, &offset);
		switch (type) {
			case FV_NONE:
				Console_Print("Couldn't find setting \'%s\'", s);
				return true;
			case FV_UINT:
				*result = config.GetSettingValueByOffset<UInt32>(offset);
#ifdef _DEBUG
				Console_Print("Retrieved value %.f for setting \'%s\' (uint)", *result, s);
#endif
				return true;
			case FV_SINT8:
				*result = config.GetSettingValueByOffset<SInt8>(offset);
#ifdef _DEBUG
				Console_Print("Retrieved value %.f for setting \'%s\' (byte)", *result, s);
#endif
				return true;
			case FV_FLOAT:
				*result = config.GetSettingValueByOffset<double>(offset);
#ifdef _DEBUG
				Console_Print("Retrieved value %.2f for setting \'%s\' (double)", *result, s);
#endif
				return true;
			case FV_BOOL:
				*result = config.GetSettingValueByOffset<bool>(offset);
#ifdef _DEBUG
				Console_Print("Retrieved value %.f for setting \'%s\' (bool)", *result, s);
#endif
				return true;
		}
	}
	return true;
}

#endif
/**************************
* Command definitions
**************************/
DEFINE_COMMAND_PLUGIN(OBRNRandomize, "Randomizes the passed object", 0, 1, kParams_OneObjectRef);
DEFINE_COMMAND_PLUGIN(OBRNListsReady, "Returns 1 if the lists are prepared", 0, 0, {});
DEFINE_COMMAND_PLUGIN(OBRNGetSetting, "Returns the value for a specified config setting", 0, 1, kParams_OneString);

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
	obse->RegisterCommand(&kCommandInfo_OBRNGetSetting);

	// set up serialization callbacks when running in the runtime
	if(!obse->isEditor)
	{

		// register to use string var interface
		// this allows plugin commands to support '%z' format specifier in format string arguments
		OBSEStringVarInterface* g_Str = (OBSEStringVarInterface*)obse->QueryInterface(kInterface_StringVar);
		g_Str->Register(g_Str);

		// get an OBSEScriptInterface to use for argument extraction
		g_scriptInterface = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);
		config.ReadCfgFromFile("Data/Randomizer.cfg");
		if (config.HasSeed()) {
			rng.seed(config.oSeed);
		}
		else {
			rng.seed();
		}
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
