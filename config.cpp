#include "config.h"
#include "utils.h"

const OblivionCfgFieldsOffset miscFields[] = {
	OBLIVIONCFGFIELD(oSeed, FV_UINT),
	OBLIVIONCFGFIELD(oExcludeQuestItems, FV_BOOL),
	OBLIVIONCFGFIELD(oDelayStart, FV_BOOL),
	OBLIVIONCFGFIELD(oInstallCrashFix, FV_SINT8),
	OBLIVIONCFGFIELD(oHitEffect, FV_SINT8),
	OBLIVIONCFGFIELD(oRandSpells, FV_SINT8),
	{NULL, 0, FV_NONE},
};

const OblivionCfgFieldsOffset lootFields[] = {
	OBLIVIONCFGFIELD(oRandInventory, FV_SINT8),
	OBLIVIONCFGFIELD(oRandContainers, FV_SINT8),
	OBLIVIONCFGFIELD(oWorldItems, FV_SINT8),
	OBLIVIONCFGFIELD(oAddItems, FV_SINT8),
	OBLIVIONCFGFIELD(oDeathItems, FV_SINT8),
	OBLIVIONCFGFIELD(oRandGold, FV_BOOL),
	OBLIVIONCFGFIELD(oExcludeUnplayableItems, FV_BOOL),
	{NULL, 0, FV_NONE},
};

const OblivionCfgFieldsOffset actorFields[] = {
	OBLIVIONCFGFIELD(oRandomizeAttrib, FV_SINT8),
	OBLIVIONCFGFIELD(oRandomizeStats, FV_SINT8),
	OBLIVIONCFGFIELD(oRestoreBaseAttributes, FV_BOOL),
	OBLIVIONCFGFIELD(oVampire, FV_SINT8),
	OBLIVIONCFGFIELD(oScaleActors, FV_BOOL),
	OBLIVIONCFGFIELD(oScaleMin, FV_FLOAT),
	OBLIVIONCFGFIELD(oScaleMax, FV_FLOAT),
	{NULL, 0, FV_NONE},
};

const OblivionCfgFieldsOffset creatureFields[] = {
	OBLIVIONCFGFIELD(oRandCreatures, FV_SINT8),
	OBLIVIONCFGFIELD(oUseEssentialCreatures, FV_BOOL),
	OBLIVIONCFGFIELD(oSkipHorses, FV_BOOL),
	{NULL, 0, FV_NONE},
};

const OblivionCfgSection sections[] = {
	{"[Misc]", miscFields},
	{"[Loot]", lootFields},
	{"[Actor]", actorFields},
	{"[Creatures]", creatureFields},
	{NULL, NULL},
};

static char* cfgeol(char* s) {
	for (char* p = s; *p != 0; ++p) {
		if (*p == ';' || *p == '\t' || *p == '\n' || *p == '\r') {
			return p;
		}
	}
	return NULL;
}

static bool strtobool(const char* s) {
	if (_stricmp(s, "yes") == 0 ||
		_stricmp(s, "true") == 0) {
		return true;
	}

	if (_stricmp(s, "no") == 0 ||
		_stricmp(s, "false") == 0) {
		return false;
	}

	return (atoi(s) != 0);
}

static const char* settingToHumanString(int setting) {
	switch (setting) {
		case 0:
			return "no";
		case 1:
			return "yes, into the same type";
		case 2:
			return "yes, no restrictions";
		default:
			return "invalid value";
	}
}

OblivionCfg::OblivionCfg() {
	oSeed = 0;
	oSeedRead = false;

	oExcludeQuestItems = true;
	oDelayStart = false;
	oInstallCrashFix = 0;
	oHitEffect = 20;
	oRandSpells = 2;

	oRandInventory = 1;
	oRandContainers = 1;
	oWorldItems = 1;
	oAddItems = 1;
	oDeathItems = 1;
	oRandGold = true;
	oExcludeUnplayableItems = false;

	oRandomizeAttrib = 1;
	oRandomizeStats = 2;
	oRestoreBaseAttributes = false;
	oVampire = 10;
	oScaleActors = false;
	oScaleMin = 0.7;
	oScaleMax = 1.5;

	oRandCreatures = 1;
	oUseEssentialCreatures = false;
	oSkipHorses = true;

	randId = 0xFF;
	for (int i = 0; i < 0xFF; ++i) {
		skipRandMod[i] = skipMod[i] = false;
	}
}

bool OblivionCfg::ReadCfgFromFile(const char* name) {
	FILE* f = fopen(name, "r");
	if (!f) {
		_ERROR("Couldn't read config file %s", name);
		return false;
	}
	char buf[256];
	const OblivionCfgSection* curr = NULL;
	bool success = true;

	for (int line = 0; fscanf(f, "%255[^\n]\n", buf) > 0 && line < 0xFF; ++line) {
		if (buf[0] == '[') {
			curr = NULL;
			for (auto it = &sections[0]; it->name; ++it) {
				if (_strnicmp(buf, it->name, strlen(it->name)) == 0) {
					curr = it;
					break;
				}
			}
			if (!curr) {
				_ERROR(__FUNCTION__": couldn't match section \'%s\'", buf);
				success = false;
			}
			continue;
		}
		if (!curr) {
			_ERROR(__FUNCTION__": line \'%s\' is not contained within a section", buf);
			success = false;
			continue;
		}
		success = false;
		if (char* p = cfgeol(buf)) {
			*p = 0;
		}
		if (!buf[0]) {
			_ERROR(__FUNCTION__": trying to parse an empty line from %s", name);
			continue;
		}
		int i;
		for (i = 0; curr->arr[i].name; ++i) {
			int len = strlen(curr->arr[i].name), start;
			if (_strnicmp(buf, curr->arr[i].name, len))
				continue;
			for (start = len - 1; buf[start]; ++start) {
				if (isspace(buf[start])) {
					continue;
				}
				if (buf[start] == '=') {
					++start;
					break;
				}
			}
			if (!buf[start]) {
				//seed can be empty
				_MESSAGE(__FUNCTION__": \'%s\' will be empty", curr->arr[i].name);
				//_ERROR("...couldn't parse...");
				//success = false;
				continue;
			}
			if (_stricmp(curr->arr[i].name, "oSeed") == 0) {
				oSeedRead = true;
			}
			switch (curr->arr[i].type) {
				case FV_NONE:
					break;
				case FV_SINT8:
					*(SInt8*)((SInt32)this + curr->arr[i].offset) = strtol(buf + start, NULL, 0);
					break;
				case FV_UINT:
					*(UInt32*)((SInt32)this + curr->arr[i].offset) = strtoul(buf + start, NULL, 0);
					break;
				case FV_FLOAT:
					*(double*)((SInt32)this + curr->arr[i].offset) = strtod(buf + start, NULL);
					break;
				case FV_BOOL:
					*(bool*)((SInt32)this + curr->arr[i].offset) = strtobool(buf + start);
					break;
			}
			break;
		}
		if (!curr->arr[i].name) {
			_ERROR(__FUNCTION__": couldn't match line \'%s\' to any setting", buf);
			success = false;
		}

	}
	fclose(f);
	_MESSAGE("Randomizer settings:\n=====================================");
	if (!oSeedRead) {
		_MESSAGE("  Seed: random");
	}
	else {
		_MESSAGE("  Seed: %u (0x%08X)", oSeed, oSeed);
	}
	_MESSAGE("  Exclude quest items: %s", oExcludeQuestItems ? "yes" : "no");
	_MESSAGE("  Delay start: %s", oDelayStart ? "yes" : "no");
	_MESSAGE("  Install Texture Crash Patch: %s", (oInstallCrashFix & 1) ? "yes" : "no");
	_MESSAGE("  Install Creature Data Crash Patch: %s", (oInstallCrashFix & 2) ? "yes" : "no");
	_MESSAGE("  Hit Effect Chance: %i%%", oHitEffect);
	_MESSAGE("  Randomize spells: %s", settingToHumanString(oRandSpells));
	_MESSAGE("  Randomize actor inventory: %s", settingToHumanString(oRandInventory));
	_MESSAGE("  Randomize container inventory: %s", settingToHumanString(oRandContainers));
	_MESSAGE("  World items: %s", settingToHumanString(oWorldItems));
	_MESSAGE("  AddItem items: %s", settingToHumanString(oAddItems));
	_MESSAGE("  Death items: %s", settingToHumanString(oDeathItems));
	_MESSAGE("  Treat gold as a regular item: %s", oRandGold ? "yes" : "no");
	_MESSAGE("  Exclude unplayable items: %s", oExcludeUnplayableItems ? "yes" : "no");
	_MESSAGE("  Randomize actor attributes: %s", 
		!oRandomizeAttrib ? "no" :
		oRandomizeAttrib == 1 ? "yes, for non-essential" : "yes, for all");
	_MESSAGE("  Randomize actor stats: %s", 
		!oRandomizeStats ? "no" : 
		oRandomizeStats == 1 ? "yes, for non-essential" : "yes, for all");
	_MESSAGE("  Restore base attributes: %s", oRestoreBaseAttributes ? "yes" : "no");
	_MESSAGE("  Vampire Chance: %i%%", oVampire);
	_MESSAGE("  Scale actors: %s", oScaleActors ? "yes" : "no");
	_MESSAGE("   Min: %.2f", oScaleMin);
	_MESSAGE("   Max: %.2f", oScaleMax);
	_MESSAGE("  Randomize creatures: %s", 
		!oRandCreatures ? "no" : 
		oRandCreatures == 1 ? "yes" : 
		oRandCreatures == 2 ? "yes, rerandomize leveled lists spawns (unstable)" : "invalid option");
	_MESSAGE("  Use essential creatures: %s", oUseEssentialCreatures ? "yes" : "no");
	_MESSAGE("  Skip horses: %s", oSkipHorses? "yes" : "no (unstable)");
	_MESSAGE("=====================================");

	return success;
}

bool OblivionCfg::ReadExcludesFromFile(const char* name) {
	randId = GetModIndexShifted("Randomizer.esp");
	if (randId != 0xFF) {
		_MESSAGE(__FUNCTION__": Randomizer.esp's ID is %02X", randId);
		skipRandMod[randId] = skipMod[randId] = true;
	}
	else {
		_ERROR(__FUNCTION__": couldn't find Randomizer.esp's mod ID. Make sure that you did not rename the plugin file.");
	}
	FILE* f = fopen(name, "r");
	if (!f) {
		_ERROR(__FUNCTION__": couldn't read mod exclusion file %s", name);
		return false;
	}
	char buf[256] = { 0 };
	bool* section = skipMod;
	for (int i = 0; fscanf(f, "%255[^\n]\n", buf) > 0 /* != EOF*/ && i < 0x200; ++i) {
		//same reasoning as in InitConfig()
		if (strcmp(buf, "[DON'T ADD TO LISTS]") == 0) {
			section = skipMod;
			continue;
		}
		if (strcmp(buf, "[DON'T RANDOMIZE]") == 0) {
			section = skipRandMod;
			continue;
		}
		UInt8 id = GetModIndexShifted(buf);
		if (id == 0xFF) {
			_WARNING(__FUNCTION__": could not get mod ID for mod %s", buf);
			continue;
		}
		section[id] = true;
		_MESSAGE(__FUNCTION__": forms from mod %s will not be %s", buf, section == skipMod ? "added to the Randomizer's lists" : "randomized");
	}
	fclose(f);
	return true;
}

bool OblivionCfg::HasSeed() {
	return oSeedRead;
}

OblivionCfgValueType OblivionCfg::GetSettingType(const char* name, SInt32* offset) {
	*offset = 0;
	for (auto it = &sections[0]; it->name; ++it) {
		for (auto curr = &it->arr[0]; curr->name; ++curr) {
			if (_stricmp(curr->name, name))
				continue;
			//got a match
			*offset = curr->offset;
			return curr->type;
		}
	}
	return FV_NONE;
}