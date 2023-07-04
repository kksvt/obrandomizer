#include "randomizer.h"

IDebugLog		gLog("obrandomizer.log");

OBSEScriptInterface* g_scriptInterface = NULL;	// make sure you assign to this
PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSESerializationInterface	* g_serialization = NULL;
OBSEArrayVarInterface		* g_arrayIntfc = NULL;
OBSEScriptInterface			* g_scriptIntfc = NULL;

std::list<TESObjectREFR*> toRandomize;
std::map<TESObjectREFR*, UInt32> restoreFlags;

bool files_read = false;
bool checked_mods = false;

#define CompileFiles_Addr 0x0044F3D0
typedef int(__thiscall* CompileFiles_t)(DWORD*, char, char);
CompileFiles_t CompileFiles = NULL;

int __fastcall CompileFiles_Hook(DWORD* _this, void* _edx, char a2, char a3) {
	if (!checked_mods) {
		srand(time(NULL));
		InitModExcludes();
		InitConfig();
		checked_mods = true;
	}
	int result = CompileFiles(_this, a2, a3);
	if (result) {
		fillUpWpRanges();
		fillUpClothingRanges();
		files_read = true;
		for (auto it = toRandomize.begin(); it != toRandomize.end(); ++it) {
			randomize(*it, __FUNCTION__);
		}
		toRandomize.clear();
		allAdded.clear();
#ifdef _DEBUG
		UInt32 numArmorClothing = 0, numWeapons = 0, numGenericItems = 0, numCreatures = allCreatures.size();
		const char* name = NULL;
		_MESSAGE("At the end of the list generation, we have:");
		for (auto it = allClothingAndArmor.begin(); it != allClothingAndArmor.end(); ++it) {
			switch (it->first) {
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
			_MESSAGE("(CLOTHING) %s (%i): %i", name, it->first, it->second.size());
			for (auto cloth = it->second.begin(); cloth != it->second.end(); ++cloth) {
				_MESSAGE("\t%s", GetFullName(LookupFormByID(*cloth)));
			}
			numArmorClothing += it->second.size();
		}
		for (auto it = allWeapons.begin(); it != allWeapons.end(); ++it) {
			switch (it->first) {
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
			_MESSAGE("(WEAPON) %s: %i", name, it->second.size());
			for (auto wp = it->second.begin(); wp != it->second.end(); ++wp) {
				_MESSAGE("\t%s", GetFullName(LookupFormByID(*wp)));
			}
			numWeapons += it->second.size();
		}
		for (auto it = allGenericItems.begin(); it != allGenericItems.end(); ++it) {
			name = FormToString(it->first);
			_MESSAGE("(GENERIC): %s: %i", name, it->second.size());
			for (auto g = it->second.begin(); g != it->second.end(); ++g) {
				_MESSAGE("\t%s", GetFullName(LookupFormByID(*g)));
			}
			numGenericItems += it->second.size();
		}
		_MESSAGE("(CREATURE): %i", numCreatures);
		for (auto it = allCreatures.begin(); it != allCreatures.end(); ++it) {
			_MESSAGE("\t%s", GetFullName(LookupFormByID(*it)));
		}
		_MESSAGE("There are %u weapons, %u generic items, %u pieces of clothing / armor and %u creatures in the lists", numWeapons, numGenericItems, numArmorClothing, numCreatures);
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
			if (oRandCreatures) {
				TESObjectREFR* ref = OBLIVION_CAST(form, TESForm, TESObjectREFR);
				if (ref != NULL && ref->baseForm != NULL && ref->GetFormType() == kFormType_ACRE) {
					if (files_read) {
						randomize(ref, __FUNCTION__);
					}
					else {
						toRandomize.push_back(ref);
					}
				}
			}
		}
		else if (!files_read) {
			if (retAddress == (void*)0x0044F221) { //called by a function that loads forms from an esp/esm
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
	UInt32 refID;
	void* retAddress = _ReturnAddress();
	if (oAddItems && !IsConsoleOpen() && retAddress == (void*)0x00507419 /*called within a script*/ 
		&& getRandomByType(a2, refID)) {
		if (TESForm * replacement = LookupFormByID(refID)) {
			a2 = replacement;
		}
	}
	return AddItem(_this, a2, a3, a4);
}

#define LoadForm_Addr 0x004603E0
typedef TESForm* (__stdcall* LoadForm_t)(int, /*int*/UInt32*);
LoadForm_t LoadForm = NULL;

TESForm* __stdcall LoadForm_Hook(int a1, /*int*/UInt32* a2) {
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
		scriptAddItem = oAddItems && retAddress == (void*)0x005073FA /*called within a script*/ && lev->GetFormType() == kFormType_LeveledItem;;
	if (a4 && (deathItem || creatureSpawn || scriptAddItem)) {
		LevListResult_t* result = (LevListResult_t*)(a4 + 8);
		while (result != NULL) {
			if (result->data != NULL) {
				if (deathItem || scriptAddItem) {
					UInt32 refID;
					if (getRandomByType(result->data->item, refID)) {
						result->data->item = LookupFormByID(refID);
					}
				}
				else if (result->data->item->GetFormType() == kFormType_Creature) {
					TESForm* rando = LookupFormByID(allCreatures[rand() % allCreatures.size()]), * old = result->data->item;
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

void InitHooks() {
	InitTrampHook(ConstructObject, 8);
	InitTrampHook(CompileFiles, 8);
	//InitTrampHook(CalcLevList, 5);
	InitTrampHook(LoadForm, 7);
	InitTrampHook(AddItem, 6);
	InitTrampHook(LoadObject, 11);
	InitTrampHook(CalcLevListOuter, 7);
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
	*result = allCreatures.size() && allClothingAndArmor.size() && allGenericItems.size() && allWeapons.size() ? 1.0 : 0.0;
	Console_Print("Randomizer's Lists Info:");
	Console_Print("Number of creatures: %u", allCreatures.size());
	Console_Print("Number of clothing/armor: %u", getNumItems(&allClothingAndArmor));
	Console_Print("Number of generic items: %u", getNumItems(&allGenericItems));
	Console_Print("Number of weapons: %u", getNumItems(&allWeapons));
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
	info->version = 1;

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
