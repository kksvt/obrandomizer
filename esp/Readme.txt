=============
Information
=============
Name: Oblivion Randomizer Mod
Author: lost cause / brndd
Date: November 5th 2024
Version: 1.0.4

=============
Description
=============
Oblivion Randomizer seeks to randomize as many parts of the game as possible, which currently include:
* items stored in containers
* items carried and worn by actors[1]
* items freely spawned in the world (optional)
* actors' aggression, confidence, responsibility (optional)
* vampirism
* (preview) actors' race
* the creatures you encounter[2]
* actor scaling (optional)
* random effects applied on each attack (optional)
* (preview) spells[3]

[1] - does not apply to dead actors
[2] - provided that they are alive and non essential
[3] - the two default starting spells (Flare and Heal Minor Wounds) will not be randomized

=============
Changelog
=============
v1.0.4:
* spells sold by vendors will no longer be randomized upon purchase
* spells will no longer be randomized from or into Porphyric Hemophilia

v1.0.3:
* fixed a bug that caused the skeleton key spell to be randomized every frame
* fixed a bug that caused scripted items not to be treated as quest items
* all keys given through scripts will now be treated as quest items
* re-dated the race randomization plugins so that they don't require a mod manager to work

v1.0.2:
* potential stability increase
* fixed an error that caused world items from non-master files not to be randomized with oWorldItems 2
* fixed an error that prevented some containers from being randomized if oRandInventory was set to 1
* added oRandGold - if it's set to 1, then gold will be treated as a regular object and randomized
* added oRandSpells - if it's set to 1, then spells will be randomized into spells of the same school; if set to 2, then any spell can be randomized into any spell; note that this feature is not 100% complete
* oRandContainers has been moved into the main version of the mod
* dead creatures will no longer be randomized as if they were containers - this feature caused way too many issues to be viable
* added a distinction between mod files that will be excluded from randomizer's lists and mod files that will be excluded from randomization
* randomizer's lists info is now only displayed in the console on the initialization as opposed to every reload
* added an experimental crash patch - if your save game crashes on a load, set the oInstallCrashFix option in the config file to 1; bear in mind that it's an experimental feature and even if your save game loads afterwards, it may be susceptible to corruption

v1.0.1:
* fixed errors in the randomization code that caused certain items to appear too rarely
* fixed an error in the randomization code that caused actors to get huge numbers of arrows if oRandInventory was set to 2
* fixed oWorldItems requiring oRandCreatures being set to 1 to work

v1.0.0:
* BACKWARDS SAVE INCOMPATIBILITY: saves made with older versions of the mod will not work properly when used with this version. You can still use them, but you might be locked out of completing creature-related quests.
* updated the FAQ section of the readme
* DLCs / Official Plug-Ins are no longer needed to run the mod
* renamed RandomizerRacePreview.ESP to RandomizerRaceSI.ESP
* added RandomizerRace.ESP (works the same as RandomizerRaceSI but does not randomize actors into Golden Saints and Dark Seducers)
* creatures and items are now loaded dynamically from all detected mod files (you can exclude certain mods from this process by editing RandomizerSkip.cfg)
* creature randomization no longer relies on PlaceAtMe
* creatures that are not spawned from leveled lists will now be re-randomized every time you restart the game
* deceased creatures (for example the dead Goblin Shaman in the tutorial) will now have their inventory randomized as if they were containers
* added an option to control creature randomization (oRandCreatures)
* added an option to control inventory randomization (oRandInventory; 1 = inventory items will be randomized into items of the same type [for example a sword will always be randomized into another sword]; 2 = randomize into any item)
* added an option to controls the chance for each actor to become a vampire (oVampire - 10% by default)
* added an option to control whether essential creatures are used in the randomizer's lists (oUseEssentialCreatures)
* added an option to control whether items given through scripts are randomized (oAddItems; 1 = randomize into the same type; 2 = randomize into any item)
* added an option to control whether items given to creatures/NPCs on their death will be randomized (oDeathItems; 1 = randomize into the same type; 2 = randomize into any item)
* added an option to control whether items spawned freely in the world (for example a book lying on a table) will be randomized (oWorldItems; 1 = randomize into the same type, 2 = randomize into any item)
* oExcludeQuestItems now also controls whether scripted creatures will be included in the randomizer's lists (oUseEssentialCreatures being 1 overrides this)
* if oRandomizeAttribEssential is set to 0, then Lucien Lachance will also not get his aggression/responsibility/confidence randomized (even though he doesn't become essential until the player kills Rufio)
* fixed an error that prevented certain types of clothing from being randomized into containers
* fixed numerous errors and poor techniques that negatively impacted the game's stability
* fixed Imperial Guards randomized into Dark Seducers not being recognized as Guards
* fixed Redguards randomized into Golden Saints appearing as Dark Seducers with Argonian lines

v0.9.2-unstable:
* discarded RandomizerAttrib.ESP, RandomizerDelay.ESP and RandomizerQuest.ESP
* created a configuration file (Randomizer.cfg) that allows the player to customize certain plugin options
* added a random chance for any attack to apply a random effect to the target actor (disabled by default)
* added actor scaling (disabled by default)
* added a way to restore the base actor stats (disabled by default)
* if you choose not to randomize essential actors' stats, then their confidence and responsibility will also no longer be randomized

v0.9.1-unstable:
* the newly randomized race will now be restored after quitting the game and reloading a save. If you're playing on a save from an old version of this mod, the races will be re-randomized upon loading
* fixed an issue that prevented an Elven race from transforming into another Elven race
* fixed an error in the readme

v0.9-unstable:
First release

=============
Requirements
=============
As of v1.0.0, Oblivion DLCs are no longer necessary to run the mod. The only requirement is to download and install xOBSE, preferably the latest version (https://github.com/llde/xOBSE/releases/).

WARNING: The 1.0.0 version has been tested on the Steam version of Oblivion. The GOG version also appears to work fine with it. If you find yourself unable to play with the mod enabled, please report it to me on the mod page, along with all the details you can provide.

=============
ESP Files
=============
This mod contains 3 ESP files (Randomizer.ESP needs to be loaded first)
* Randomizer.ESP		[REQUIRED] 		- the basic randomizer file
* RandomizerRace.ESP		[NOT RECOMMENDED] 	- randomizes the actors' races; it's fun albeit also extremely glitchy and greatly lowers the stability (use this if you don't want actors randomized into Golden Saints and Dark Seducers or if you do not own the Shivering Isles DLC)
* RandomizerRaceSI.ESP		[NOT RECOMMENDED]	- same as above, except it also uses SI races

NOTES: 
1) If you want race randomization, use either RandomizerRace.ESP or RandomizerRaceSI.ESP, not both.
2) Make sure that you do not rename "Randomizer.ESP" to anything else.

=============
Installation
=============
After installing the version of xOBSE of your choice, copy the ESP files, Randomizer.cfg, RandomizerSkip.cfg and the OBSE folder from this archive into your Oblivion/Data folder (the full path depends on your installation; a sample Steam installation path could be C:\Program Files (x86)\Steam\steamapps\common\Oblivion\Data) and then activate the ESP files through Oblivion Launcher => Data Files (or a mod manager of your choice).
NOTE: If you were previously using 0.9.1-unstable or 0.9-unstable, you should remove RandomizerAttrib.ESP, RandomizerDelay.ESP and RandomizerQuest.ESP from your Oblivion/Data folder. All functionality provided by these plugins can now be achieved through the configuration file.

===============
Uninstallation
===============
1) Delete the following files from your Oblivion/Data folder:
* Randomizer.ESP
* RandomizerRace.ESP
* RandomizerRaceSI.ESP
* Randomizer.CFG
* RandomizerSkip.CFG
2) Delete obrandomizer.DLL from Oblivion/Data/OBSE/Plugins.

=============
Configuration
=============
1) Randomizer.cfg allows you to configure some of the mods' features to more of your liking, creating either a somewhat normal game world or a completely broken mess (or anything in between). If you opt not to modify the configuration file, then you will get the same experience as you would with the older (0.9-unstable or 0.9.1-unstable) versions of the mod with all recommended plugin files enabled. The configuration file can be edited with any text editor (Windows's Notepad is perfectly fine for it) and every option is explained inside. If you don't feel like editing the file yourself, I've included two optional configuration files:

a) Chaos - to activate, copy the Randomizer.cfg from the Sample Configs/CHAOS folder to Oblivion/Data. As the name suggests, this configuration will make the game incredibly chaotic.
b) Quest - to activate, copy the Randomizer.cfg from the Sample Configs/QUEST folder to Oblivion/Data. This configuration file makes minimal use of the randomizer's features, randomizing only items and creatures (and races, should you decide to use RandomizerRace files). Won't mess with NPCs' aggression, so it's perfect if you want to do try to do quests without interruptions from actors hellbent on murdering you or each other.
The idea for the implementation of the configuration file was inspired by Ersh's and Gribbleshnibit8's LootMenu's config (which sadly is still incompatible with the Randomizer, as mentioned in Known Issues), thus I wanna credit them for it.

2) RandomizerSkip.cfg determines which plugin files will be skipped when generating randomizer's lists and which plugin files will be excluded from being randomized. There are two sections - [DON'T ADD TO LISTS] and [DON'T RANDOMIZE]. 

a) [DON'T ADD TO LISTS]
For example, if you are playing a total conversion mod, you may want the randomizer not to randomize objects into vanilla Oblivion ones. Thus you ought to put "Oblivion.esm" (without quotation marks) into RandomizerSkip.cfg under [DON'T ADD TO LISTS]. Should you desire to exclude more plugin files, simply add their names into RandomizerSkip.cfg (under [DON'T ADD TO LISTS]), each file name in a separate line.

b) [DON'T RANDOMIZE]
Let's say you are using the Umpa Animation mod to change the NPCs' animations. You wouldn't want the spells provided by it to be randomized into other spells and you still want normal Oblivion spells to be randomized, thus you should put "77_Umpa_Animation.esp" (without quotation marks) into RandomizerSkip.cfg under [DON'T RANDOMIZE].

A sample RandomizerSkip.cfg config may look like this:

"
[DON'T ADD TO LISTS]
DLCBattlehornCastle.esp
Knights.esp

[DON'T RANDOMIZE]
77_Umpa_Animation.esp
DLCVileLair.esp
"

That way nothing will get randomized into items (or spells or creatures) that are exclusive to the Battlehorn Castle DLC and the Knights of the Nine DLC. In addition, the Umpa Animation spells and objects added by the Vile Lair (Deepscorn Hollow) DLC will not be subject to randomization.

=============
FAQ
=============
Q: How do I know that the mod has been installed properly?
A: After entering the game with the mod loaded, you should be greeted with "Oblivion Randomizer is ready" in the top left corner of your screen (no matter if you use oDelayStart or not).

Q: Will I need to start a new game?
A: No, however the spells that the player already has will not be randomized.

Q: I loaded an existing save and certain creatures are not randomized, even though oRandCreatures is set to 1 in the config file. Why is that?
A: Creatures spawned from leveled lists are by default randomized only on spawn (and in case of an old save they were spawned without the randomizer running). If you want them to be randomized, change oRandCreatures to 2, but be advised that it is not a recommended setting and you should revert it to 1 once you make a new save with them randomized.

Q: Is this mod compatible with X?
A: It should be!

Q: I want my modded items to be included in the randomization process. How do I do that?
A: As of v1.0.0, you do not have to do anything.

Q: Can I make changes to your mod and release it?
A: Sure! You're welcome to alter it in any way you'd like, however I'd ask you to credit me, should you decide to publish your work.


=============
Known Issues
=============
1. Save corruption / Frequent crashes
As of v1.0.0, I believe the save corruption to be largely gone - I could not reproduce it in any way, but that does not mean it has been fixed. You have to keep in mind that Oblivion is an old game, running on a dated engine that is riddled with issues even in the vanilla game, and randomizing creatures, inventories, NPC stats and so on is bound to exacerbate the problems with it. You should most likely backup the saves you intend to play on with the mod just to be safe. Having said that, I am convinced that this is the most stable the mod has ever been and it will ever be.

2. showracemenu [RandomizerRace specific]
Due to the way race randomization is implemented, you should NOT call showracemenu after finishing the tutorial (or choose "Edit Race" before exiting the sewers). Otherwise you will have to scroll through duplicate races, that are the same as their original counterparts, with the exception of their voice.

3. Overusing "Randomize" spell might cause a crash.

4. LootMenu may cause the game to crash if the container you're looking at has certain items in it.

5. Randomized Creatures not breaking the stone wall in the Tutorial (Charactergen)
In the Tutorial section, right after the first ambush, two rats are supposed to break a stone wall to attack you and let you progress further. Sometimes the newly randomized creature might get confused or distracted by another randomized creature (especially if they are hostile towards one other) and not break the wall. If that happens, open the console (~), click on the stone wall and type "disable".

6. oRandCreatures set to 2 might sometimes cause the game to crash when reloading a save
It's not that big of a deal, since you will be able to properly load your desired save after launching the game again. Having said that, oRandCreatures set to 2 is not a recommended setting.

=======================================
Technical aspects / potential issues
=======================================
Keep in mind that my methods most likely leave a lot to be desired, as it's my first "real" mod. I had to learn both the Construction Set and Oblivion's scripting language practically from the scratch, and there are several things that may or may not cause problems:

1. Race randomization adds duplicate races, with the only distinction being their voice. CopyRace does not preserve the original race's voice, which causes NPCs with unique lines to be unvoiced. Therefore I had to manually copy each race and adjust their voice (for example ArgonianBreton has all the characteristics of an Argonian, save the tail, but uses the Breton voice). The duplicate races have to be marked as playable after exiting the player creation, otherwise their rumors and generic dialogue will be unvoiced. I do not know a way around that.

==========================
OBSE Plugin Source Code
==========================
If you're a developer and you wish to change some features, you can download the plugin source code from https://github.com/kksvt/obrandomizer. The Microsoft Visual Studio project included expects the directory with the source code to be in the same directory as the OBSE's source code, which you have to download separately from https://github.com/llde/xOBSE.

==============
Parting words
==============
Thank you for downloading my mod! I do hope you will enjoy it!