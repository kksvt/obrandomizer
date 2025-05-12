=============
Information
=============
Name: Oblivion Randomizer Mod
Author: lost cause / brndd
Date: May 12 2025
Version: 1.1.0

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
v1.1.0:
* BACKWARDS CONFIG INCOMPATIBILITY: configs made for the previous versions of the mod will no longer be compatible with this version
* added a GUI application to facilitate config creation
* added oExcludeUnplayableItems - if it's set to 1, then armors marked as unplayable won't take part in the randomization
* added a fixed seed option (oSeed). Leave it empty to have a random seed every launch
* added oRandomizeStats - if it's set to 1, it will randomize non-essential actors' basic attributes (strength, dexterity...) and skills (blade, blunt...); 2 will aslo randomize essential actors
* oRandContainers now has three options (0 - disabled, 1 - randomize every item into an item of the same time, 2 - randomize every item without restrictions) and is separate from actors' oRandInventory
* fixed a minor bug in the oRandInventory 2 when randomizing bows and staves
* oRandomizeAttrib and oRandomizeAttribEssential are now merged into one setting (oRandomizeAttrib; 0 - disabled, 1 - only non-essential, 2 - all)
* removed a debug print previously displayed when re-enabling manually randomized creatures 
* spells used by actors are now randomized on launch
* fixed randomized creatures' aggression adjustment

v1.0.5:
* special thanks to razorblade457 for his bug reports and testing
* added oSkipHorses (default 1) - if it's set to 1, then rideable creatures (mounts) will no longer take part in randomization, as I believe they are the root cause of many crashes/save corruption issues. Set to 0 at your own risk.
* added 2 as an option to oInstallCrashFix - it will attempt to fix saves that crash because of corrupt creature data; if you wish for both oInstallCrashFix patches to be enabled at once, set it to 3.
* objects with empty models will no longer be randomized
* fixed a strange crash that was sometimes caused by randomizing jailor guards

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
After installing the version of xOBSE of your choice, copy the contents of the zip file to your Oblivion folder (the full path depends on your installation; a sample Steam installation path could be C:\Program Files (x86)\Steam\steamapps\common\Oblivion) and then activate the ESP files through Oblivion Launcher => Data Files (or a mod manager of your choice).
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
3) Delete obrandomizer-gui.EXE and obrn-configs from your Oblivion folder.

===========================
Configuration through GUI
===========================
As of version 1.1.0, the mod comes equipped with a GUI application to facilitate the configuration process. It requires .NET 8.0 Desktop Runtime to run, and should your computer not have it installed, it will provide you the following link to it: https://dotnet.microsoft.com/en-us/download/dotnet/8.0

======================
Manual Configuration
======================
If you elect not to use the GUI, you can still configure the mod manually.

1) Randomizer.cfg allows you to configure some of the mods' features to more of your liking, creating either a somewhat normal game world or a completely broken mess (or anything in between). If you opt not to modify the configuration file, then you will get the same experience as you would with the older (0.9-unstable or 0.9.1-unstable) versions of the mod with all recommended plugin files enabled. The configuration file can be edited with any text editor (Windows's Notepad is perfectly fine for it).

Options:
* oSeed - seed for the RNG system. Information about the randomization of world items, creatures not spawned from leveled lists and actor spells is not stored anywhere in the save file, meaning they will be re-randomized every time you launch the game. Using a fixed seed will allow them to be randomized into the same objects every time (provided your mod load order doesn't change), giving you a consistent experience across a single playthrough. Leave it empty if you want it to be randomized every launch.
* oExcludeQuestItems - if set to 1, quest items won't be included in the randomization process.
* oDelayStart - if set to 1, then the actor and inventory randomization won't kick in until the first ambush in the tutorial (Charactergen reaches stage 20).
* oInstallCrashFix - if set to 1, it will install a patch that prevents the game from crashing when loading certain textures; if set to 2, it will install a patch that fixes reading invalid data for creatures from a save file; setting it to 3 enables both patches. If you find your save game crashing on load, try to enable either of these options (or both) and see if it helps.
* oHitEffect - chance for each hit to apply a random effect to the victim. Possible effects: fire, frost, shock, demoralize, burden, frenzy, paralysis.
* oRandSpells - if set to 1, each spell will be randomized into a spell of the same school; if set to 2, there will be no restriction on school when it comes to randomization.

* oRandInventory - if set to 1, each item in an actor's inventory will be replaced with an item of the same type; if set to 2, actor's inventories will be randomized without restrictions.
* oRandContainers - same as above, except for chests and other containers.
* oWorldItems - if set to 1, every item spawned freely in the world (such as a book lying on a table) will be randomized into an item of the same type; if set to 2, they will be randomized without restrictions.
* oAddItems - same as above, except it applies to items given through scripts.
* oDeathItems - same as above, except it applies to items given on carrier's death (such as Dremoras receiving a Daedra Heart).
* oRandGold - if set to 0, then gold's quantity will be randomized; if set to 1, then gold will be treated just like any other item during randomization.
* oExcludeUnplayableItems - if set to 1, it will exclude armors marked as unplayable from the randomization process.

* oRandomizeAttrib - if set to 1, it will randomize non-essential actors' confidence/aggression/responsibility; if set to 2, it will randomize all actors' confidence/aggression/responsibility.
* oRandomizeStats - if set to 1, it will randomize non-essential actors' basic attributes (strength, dexterity, etc) and skills (blade, blunt, etc); if set to 2, it will randomize these stats for all actors.
* oRestoreBaseAttributes - if set to 1, it will restore actors' confidence/aggression/responsibility to their base values. Make sure oRandomizeAttrib is set to 0, if you plan on using it.
* oVampire - the chance for each actor to be turned into a vampire.
* oScaleActors - if set to 1, then it will scale each actor by a value in the range of [oScaleMin, oScaleMax].
* oScaleMin - refer to oScaleActors.
* oScaleMax - refer to oScaleActors.

* oRandCreatures - if set to 1, it will randomize non-leveled list creatures every launch and leveled list creatures on spawn; if set to 2, it will re-randomize all creatures on launch and savegame load. Note that 2 is a highly unstable setting and it is recommended that you don't use it.
* oUseEssentialCreatures - if set to 1, then essential creatures will be used in the randomization process.
* oSkipHorses - if set to 1, it will exclude horses from the randomization. It would appear that randomizing creatures from and into horses causes a great deal of problems, so it is recommended you leave it at 1.

Note: setting a given option to 0 disables it.

If you don't feel like editing the file yourself, I've included two optional configuration files:

a) Chaos - to activate, copy the "Chaos.cfg" file from the "obrn-configs" folder to Oblivion/Data and rename it to "Randomizer.cfg". As the name suggests, this configuration will make the game incredibly chaotic.
b) Quest - to activate, copy the "Quest.cfg" file from the "obrn-configs" folder to Oblivion/Data and rename it to "Randomizer.cfg". This configuration file makes minimal use of the randomizer's features, randomizing only items and creatures (and races, should you decide to use RandomizerRace files). Won't mess with NPCs' aggression, so it's perfect if you want to do try to do quests without interruptions from actors hellbent on murdering you or each other.

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

Q: The plugin file and/or the GUI application gets flagged by my antivirus. What do I do?
A: During my testing, some antiviruses flagged this mod and the GUI as malicious, but I can guarantee you that these are false positives. If you don't believe me, you're welcome to compile the mod and the GUI yourself, as they are both open-source on my github.

Q: Can I make changes to your mod and release it?
A: Sure! You're welcome to alter it in any way you'd like, however I'd ask you to credit me, should you decide to publish your work.


=============
Known Issues
=============
1. Save corruption / Frequent crashes
As of v1.0.0, I believe the save corruption to be largely gone - I could not reproduce it in any way, but that does not mean it has been fixed. You have to keep in mind that Oblivion is an old game, running on a dated engine that is riddled with issues even in the vanilla game, and randomizing creatures, inventories, NPC stats and so on is bound to exacerbate the problems with it. You should most likely backup the saves you intend to play on with the mod just to be safe. 
If you find yourself unable to load your save due to crashes to desktop, try setting the oInstallCrashFix option to 2 or 3.

2. showracemenu [RandomizerRace specific]
Due to the way race randomization is implemented, you should NOT call showracemenu after finishing the tutorial (or choose "Edit Race" before exiting the sewers). Otherwise you will have to scroll through duplicate races, that are the same as their original counterparts, with the exception of their voice.

3. Overusing "Randomize" spell might cause a crash.

4. LootMenu may cause the game to crash if the container you're looking at has certain items in it.

5. Randomized Creatures not breaking the stone wall in the Tutorial (Charactergen)
In the Tutorial section, right after the first ambush, two rats are supposed to break a stone wall to attack you and let you progress further. Sometimes the newly randomized creature might get confused or distracted by another randomized creature (especially if they are hostile towards one other) and not break the wall. If that happens, open the console (~), click on the stone wall and type "disable".

6. oRandCreatures set to 2 might sometimes cause the game to crash when reloading a save
It's not that big of a deal, since you will be able to properly load your desired save after launching the game again. Having said that, oRandCreatures set to 2 is not a recommended setting.

==========================
OBSE Plugin Source Code
==========================
If you're a developer and you wish to change some features, you can download the plugin source code from https://github.com/kksvt/obrandomizer. The Microsoft Visual Studio project included expects the directory with the source code to be in the same directory as the OBSE's source code, which you have to download separately from https://github.com/llde/xOBSE.

==============
Parting words
==============
Thank you for downloading my mod! I do hope you will enjoy it!