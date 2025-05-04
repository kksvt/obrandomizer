#include "utils.h"
#include "randomizer.h"

void logDetailedListInfo() {
	UInt32 numArmorClothing = 0, numWeapons = 0, numGenericItems = 0, numCreatures = allCreatures.size(), numSpells = 0;
	const char* name = NULL;
	_MESSAGE("At the end of the list generation, we have:");
	for (const auto& it : allClothingAndArmor) {
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
	for (const auto& it : allWeapons) {
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
	for (const auto& it : allGenericItems) {
		name = FormTypeToString(it.first);
		_MESSAGE("(GENERIC): %s: %i", name, it.second.size());
		for (auto g : it.second) {
			_MESSAGE("\t%s", GetFullName(LookupFormByID(g)));
		}
		numGenericItems += it.second.size();
	}
	for (const auto& it : allSpellsBySchool) {
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
	if (config.oRandInventory) {
		_MESSAGE("There are %u total items", allItems.size());
	}
	_MESSAGE("There are %u weapons, %u generic items, %u pieces of clothing / armor, %u creatures and %u spells in the lists",
		numWeapons, numGenericItems, numArmorClothing, numCreatures, numSpells);
}

void debugDumpSpells(TESForm* form) {
	TESActorBase* actor_base = OBLIVION_CAST(form, TESForm, TESActorBase);
	if (actor_base != NULL) {
		FILE* f = fopen("npc_spells.txt", "a");
		fprintf(f, "Actor/Creature: %s (%08X)\n", GetFullName(form), form->refID);
		{
			auto data = actor_base->spellList.spellList.Info();
			auto next = actor_base->spellList.spellList.Next();
			while (data != NULL) {
				UInt32 refID = files_read ? data->refID : (UInt32)data;
				TESForm* item = LookupFormByID(refID);
				if (item) {
					fprintf(f, "\tNon-leveled Spell data %08X %s (%08X)\n", data, GetFullName(item), item->refID);
				}
				if (next == NULL) {
					break;
				}
				data = next->Info();
				next = next->Next();
			}
		}
		if (1)
		{
			auto data = actor_base->spellList.leveledSpellList.Info();
			auto next = actor_base->spellList.leveledSpellList.Next();
			while (data != NULL) {
				UInt32 refID = (UInt32)data;
				fprintf(f, "\tLeveled Spell: %s (%08X) %s\n", GetFullName(data), data->refID, FormTypeToString(data->GetFormType()));
				if (next == NULL) {
					break;
				}
				data = next->Info();
				next = next->Next();
			}
		}
		fclose(f);
	}
}