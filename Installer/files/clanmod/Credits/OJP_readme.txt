==============================
Open Jedi Project (OJP) Readme
==============================

"You can't seem more open source than Open Jedi" - Keshire

Website:  ojp.jediknight.net.

NOTE:  This readme might not be part of an official OJP release since this readme is required to be included in all OJP based projects.  If this isn't an official OJP release, this document will only partially apply.


========================
0000 - Table of Contents
========================

	0000..................Table of Contents
	0001..................Introduction
	0002..................Installation/Uninstallation
	0003..................What's New?
	0004..................Features
	     0004.1...........Basic
	     0004.2...........Enhanced
	     0004.3...........Vehicles
	0005..................Using Our Work
	0006..................Submitting Stuff to OJP
	     0006.1...........Submission Guidelines
	0007..................Known Issues
	     0007.1...........Issues with Basic Features
	0008..................Credits
	0009..................Contact Information
	0010..................Legal Stuff

	
	
===================
0001 - Introduction
===================

The Open Jedi Project is a coding/modding collaboration with the intent of maximizing the features and fun factor for all Jedi Knight Academy (JKA) mods.  We work together by contributing fun, interesting, and useful game features so that everyone can benefit.

We operate on what's basically an open source system. Open source basically means that the source code is freely available and accessible by all. See the "Using Our Work" section for details about rights and permissions.

Our design philosophy is to make everything as separated and customizable as possible to allow developers and players to choose what features they wish to use. 

We currently have two distributions (gameplay/code changes), Basic and Enhanced. We also have two packs (non-gameplay related), Vehicles and Skins.

Basic has two main features. One is bug fixes and balance fixes, neither which will severely alter game play. They are designed to be the "unofficial patch" for bugs and game play problems. Basic can still be considered vanilla Jedi Academy. The other main feature is map enhancements. Things such as new entities, expanded AI, vehicles, scripting and effects system can allow mappers to create far more immersive and fun maps. Since Basic also aims towards recreating all the single player entities and code, it is possible for mappers to create full featured cooperative and single player games and levels using the multiplayer engine. This allows modders to make other enhancements such as new weapons, AI, etc., not possible using the single player engine. 

Enhanced is a superset of Basic, meaning it has anything and everything included in the Basic distribution. The difference is that it adds many significant gameplay alterations. On its own, it is a fully playable mod, with major gameplay alterations that deviate from vanilla Jedi Academy's. Players get a full mod that offers almost totally revamps lightsaber gameplay (see below). Developers get a solid, flexible code base that includes the basic, fundamental features people would want to find in most mods, allowing them to spend their time on what makes their work truly unique.

The Vehicles package is a collection of additional vehicles for JKA.  We're including them in the project to allow OJP compatible maps to use additional vehicles without having to include said vehicles in their release packages.

The Skins package is a collection of quality modder created skins that have been turned 
into customizable player classes.  We've done this to allow more player options, to 
allow SP useage of these skins, to allow servers and players to quickly get a pack of 
quality skins, and to prevent a lot of redundent data from clogging up your game 
directory.  

We have a web forum set up for OJP.  You can find it at...
http://www.lucasforums.com/forumdisplay.php?s=&forumid=542



===================
0002 - Installation/Uninstallation
===================

Installation/Uninstallation instructions have been moved to OJP_install.txt



==================
0003 - What's New?
==================

What's New? has been moved to OJP_changelog.txt


		
===============
0004 - Features
===============


--------------
0004.1 - Basic
--------------

	Admin System (AdminSys)
	Multiplayer, Admins

	See OJP_adminsys.txt.


	Ambient Player Sounds (AMBIENTEV)
	Multiplayer, Sounds, Animevents

	What It Does:

		Players now have the ability to setup two fully customizable ambient sounds (Vader's resporator breathing, sensor ping, breathing, whatever you want) thru use of a new animevent type.  See ojp_readme.txt and ojp_animevents.txt for details.


	Animevents System Overhaul (ANIMEVENTS)
	Multiplayer, Sounds, Animations, Animevents

	What It Does: 

		The animation events system controls how most of the player sound effects are controlled in JKA.  OJP has heavily modified the system to add back in SP animevent features that were removed for MP and add additional functionally.  Changes include:

	- Ambient Sound Support (See Ambient Player Sounds)   

	- Dynamic/Custom Sounds:
		A part of the SP animevents.cfgs, this support was missing from MP until now.  These are sounds that are player model dependant.  IE, Vader's pain sounds are different than Jan's.

	- Per-Model animevents.cfg:
		You can use per-model animevents.cfgs.  This means that you can personalize each person to have individualized sound effects and even special event!  A good example what you can do with this would be to make a certain character scream before certain attacks or have their spine crackle from hard impacts.

See OJP_readme.txt and OJP_animevents.txt for details on how to customize/use animevents.cfgs.


	Asteroids (Asteroids)
	Multiplayer, Vehicles
	
	What It Does:

		All the Asteroids Mod code has been ported to OJP.  You can now use Asteroids' maps without problems.  The changes that Asteroids made were as follows:

	- some XBOX only code that wasn't in the original SDK release.

 	- Changed the some rocket lock effect prespective values for vehicles.

 	- Changed the screen location for non menu seige messages?  Same with trap_sendservercommand cps messages.

 	- lots of death message improvements.

 	- passenger view turret control tweaking.

 	- fire trails for dieing vehicles.

 	- some surface debris tweaks.

 	- increased the number of possible animation files to 64

 	- Two new MOD (methods of death): MOD_COLLISION and MOD_VEH_EXPLOSION

 	- Some additional otherkiller code so the death messenges do better vehicle messenges

 	- Astromech droids in vehicle set their teams to be on the same team as the vehicle.

 	- Improved the vehicle weapon automatic target leading and added a server toggle. (g_vehAutoAimLead).  This makes the game auto lead your shots on other vehicles if you point your crosshairs directly at them while firing.

 	- Disabled missile health for moving missiles to prevent nasty issues with vehicles hitting missiles before the missiles hit the vehicles.

 	- Automated vehicle turrets can't fire without a pilot in the vehicle.

 	- Vehicle turrets will no longer fire on teammates.

 	- Killing a enemy vehicle turret now increases your score by one.

 	- Some hyperspace behavior tweaks.

 	- Some code tweaks regarding NPCs in vehicles and team damage.

 	- New hyperspace map entity varible "exitscale" - "Can use this to make the vehicle appear farther from the exit position than they were from the entry position at the time of teleporting (scales the relative position - default is "1" - normal, no scale)"

	 - Fixed issue with vehicle missiles not having a parent entity set.

 	- If dead/dieing vehicles crash into another entity, the crashing vehicle's killer gets the kill.

 	- Fixed damage inflictor pointers for G_RadiusDamage.

 	- Players standing on top of a vehicle taking off die now.

 	- Tweaked the control pitching  for fighter vehicles for damage, gravity, and such.


	Bug Fix 001 (BUGFIX1) - Animation.cfg fix for BOTH_WALKBACK2
	Single Player, Animations

	What It Does:
		Fixes an animation bug seen when walking backwards with an ignited single saber in single player by correcting the number of frames
	
	Special Notes:
		This fix will be obsolete once a game patch comes out.  Remember to remove this file before you patch the game.
		Might cause problems when playing multiplayer.  If you have any problems, just move the file out of the /base directory. 


	Bug Fix 002 (BugFix2)
	Multiplayer, Animations

	What It Does:
		Fixes a major issue with the animation timer calculation in MP.  The animation timers are now calculated correctly and this should help with animations that play in Force Rage, Force Speed, or at odd saber animation speeds.


	Bug Fix 003 (BugFix3)
	Multiplayer, Fall Damage

	What It Does:
		Fixes bug in the fall damage code that caused client number 0 to always take Jedi style fall damage even when not meeting any of the Jedi style requirements.	


	Bug Fix 004 (BugFix4)
	Multiplayer, Linux
	
	What It Does:
		Fixes an issue with Linux servers and RandFloat.  It was causing all sorts of weird issues.


	Bug Fix 005 (BugFix5)
	Multiplayer, Effects

	What It Does:
		Fixed minor bug with saber clash effect logic.  Before, the clash effect was being duplicated for sabers with multiple blades when only one clash should have been occurring.


	Bug Fix 006 (BugFix6)
	Singleplayer, Multiplayer, Models

	What It Does:
		Twi'lek "skirts" 3 and 4 are now twosided (added z_newstuff.shader)


	Bug Fix 007 (BugFix7)
	Multiplayer, Siege

	What It Does:
		Fixed a problem with the capturable seige items getting caught on walls and floating up off the map and not respawning.


	Bug Fix 007 (BugFix7)
	Multiplayer

	What It Does:
		Fixed stack overflow in CalculateRanks().


	Bot Tweaks (BotTweaks)
	Multiplayer, Bots

	What It Does:

	- Bots in honorable duel acceptance mode (bot_honorableduelacceptance 1) now require you to fully holster your saber before accepting a saber challenge.
		
	- Add Bot menu commands added to vchat menu.  In addition, vchat/addbot menu is now availible in all team gamemodes.
		
	- Bot AI FPS Cvar:
		You can now control how often (in fps) the Bot AI is processed by using the bot_fps cvar.  NOTE:  The Bot AI can't be done faster than the current server fps (sv_fps).

	- Bots can now move thru all areas that players can.

	- Changed the default settings for both "bot_wp_distconnect" and "bot_wp_visconnect to off.

	- Cleared up the strafe code a bit to reduce CPU useage.

	- Removed some redundent code.

	
	CoOp Gametype [CoOp]
	Multiplayer
	See OJP_CoOp.txt for details.


	Dismemberment Enhancements (FullDismemberment)
	Multiplayer, Dismemberment

	What It Does:
		
		Players can now have more than one limb lopped off at a time and explosive attacks now blow players to pieces.  Note:  This doesn't work for bodies after the player has respawned.


	Duel Taunts in all Gametypes (ALLTAUNTS)
	Multiplayer, Emotes

	What It Does:
	Duel taunts now work in all gametypes.  These can be setup using theControls menu or by using the following commands (same as the original duel commands):

	/bow - "Show respect to your opponent."
	/meditate - "Sit and wait patiently for your opponent to make the first move."
	/flourish - "Show off a little."
	/gloat - "Celebrate your victory over your opponent."


	Extra Vehicle Implementation (VEHICLE_IMP)
	Multiplayer, Vehicles

	What It Does:

		- Allows vehicle creators more flexability to set up their vehicle behaviours. These features were added specifically to support the Droideka vehicle, but these features can also be used by any VH_WALKER class vehicle. Some of the features are also useable by any vehicle of any class.
		- New cloaking ability option for vehicles.  Vehicles with cloak can be cloaked by pressing the Use Forcepower button. [CloakedVehicles]
			- Engine exhaust disappears while cloaked.
			- Firing a weapon causes a ship to decloak.
			- Plays CloakingDeviceSound when the device is turned on and off.
			- If the a single attack does more than 20% of the ship's max armor, the ship decloaks
			- Having less than 40% armor/health disables the cloaking device.
		See OJP_vehicles.txt for full details of the new options avaliable.


	Hidden JKA Saber Hilts (HSABERS)
	Multiplayer

	What It Does:
		Adds to multiplayer the Retribution (Desann's), the Skywalker (Luke Skywalker's Saber), and the Stinger (default Reborn's) saber hilts.  These hilts come with the game but are hidden by baseJKA.


	Hidden Lannik Racto Skin
	Multiplayer
	
	What It Does:
		Makes the Lannik Racto skin selectable in the MP skins menu.  Also created icon for said skin.


	Holocron File System (HolocronFiles)
	Multiplayer, General

	What It Does:
			
		Added holocron points for all JKA FFA maps in the form of an external hpf file. 
		
	- New Commands: (while in bot_wp_edit mode ONLY)
		
		!addholocron - Add a holocron point at your feet.
		!saveholocrons - Save the list of holocron positions.
		!spawnholocron - Spawn a holocron where you are. (for testing)

	- Added holocron point hpf files from AOTCTC for compatability.


	Holstered Weapons (VisualWeapons)
	Multiplayer

	What It Does:
		You can now see the weapons you're carrying (but not using) holstered on your player's body.  Note:  This feature currently requires that you to specially prepare your model for this feature.


	Linux GCC Fixes (Linux)
	Multiplayer, Server, Linux

	What It Does:
		The OJP code will now compile on the gcc compiler for Linux.


	Match Warm Ups (FIXWARMUP)
	Multiplayer, Server

	What It Does:
		You can now set a warm up period before the beginning of all non-duel gametypes.  You can find options for this in the create server menu under the advanced tab.  Also see OJP_cvars.txt for details on the cvars that control the warm ups.


	More Force Disable Options
	Multiplayer, Server

	What It Does:
		You can now set "Jump Only" and "Neutrals Only" for the Force Disable (now called "Force Powers") option.

	All On 	  = All Force Powers Enabled
	All Off   = All Force Powers Disabled
	Jump Only = Only Force Jump Enabled 
	Neutrals Only = Only Saber Defense, Saber Offense, Jump, Seeing, Push, and Pull are enabled.

	NOTE:  Force Jump defaults to level 1 when disabled.  Saber Defense/Offense default to level 3 when disabled.


	More Weapon Disable Options (MOREWEAPOPTIONS)
	Multiplayer, Weapons

	What It Does:
		You can now disable any weapon in the game in any combination except for having all weapons (including melee) disabled. This is controled by the weapondisable cvars (g_weaponDisable and g_duelWeaponDisable).  Ammo now correctly disappears when the weapons for that ammo type are disabled.  You now have menu options for Melee Only, Sabers & Melee Only, and No Explosives.


	More Vehicles (MOREVEHICLES)
	Multiplayer, Vehicles

	What It Does:
		The game code has been changed to up the vehicle limit from 16 to 64 different vehicle files.  This means you can have 64 different types of vehicle models (You can spawn up to 128 vehicles total.)  Note:  Different vehicle skins count against the vehicle model total.

		In addition, the vehicle weapon limit has been raised from 16 to 64.


	Improved Botrouting
	Multiplayer, Bots
	
	What It Does:
		Botrouting controls how MP bots navigate a given maps.  We've gone thru and improved the botrouting for the following maps:
		mp/duel3
		mp/duel6
		mp/duel8
		mp/ffa5


	Old JK2 Gametypes (OLDGAMETYPES)
	Multiplayer, Basic, gametypes
	
	What it Does:
		Reintroduces the old JK2 gametypes Holocron FFA, Jedi Master, and Capture the Ysalimari.  Added "team" (team ffa) map type to the .arena file options.  I've added a fix to the map selector function to prevent the asteroid maps from being used in FFA mode.  Basically, this means that you have to make sure the "team" option is in a map's .arena file or else it won't be availible in the menu system for Team FFA. 	
		

	RGB Character Color Menu Options
	Multiplayer, Single Player, Menus

	What It Does:
		 Adds a submenu that allows you to pick any color for your character's color with simple slide bars or float values.  To enter a new float value, just click on the float value seen next to the slider bar and enter in a new number.


	RGB Sabers
	Multiplayer, Menus

	See OJP_rgbsabers for all the details on the very detailed RGB saber system.


	Realistic Saber Menu Options
	Single Player, Sabers, Menus

	What It Does: 
		Adds a menu option to SP that allows you to turn on different levels of saber damage realism. You can find the option under Setup->Options->Saber Realism.

	Normal = Default Saber Damage / Dismemberment
	Boosted Damage = Boosted Saber Damage
	Realistic Dismemberment = Attack swings now do lethal damage and can cut off multiple limbs at once.
	Realistic Idle = Idle Sabers now do lethal damage.  Also includes the features of Realistic Dismemberment.

	Please note that this option defaults to Normal each time you start up the game.  We currently don't know of a way around that.  Sorry.

	ROFF Scripting
	Multiplayer

	What It Does:
		ROFF scripting supported added to MP.

	SP Dual/Staff Menu
	Single Player, Sabers

	What It Does:
		The SP menu file has been altered to allow you to use the dual sabers/saber staff from the beginning of the game.  This also allows you to choice a red saber blade in SP.


	TAB Bots
	Multiplayer
	
	What It Does:
		Totally redesigned AI bots for multiplayer.  WIP. Currently works on all the default Siege maps and all FFA gametypes.  See OJP_bots.txt.


	True View (TrueView)
	Multiplayer
	Related cvars/Commands:
		cg_truespin
		cg_trueflip
		cg_trueroll

	What It Does:
		True View puts you in the game.  It's a special first person camera that puts you in the body of your character.  Roll and you'll see your body roll.  Look down and you'll see your feet.

	Realism Settings:
		Unfortunately, JKA wasn't created with the FPS view in mind.  Many of the animations move too quickly and can result in motion sickness.  With that in mind, we've added some controls to alway you to enjoy even if you get motion sickness or prefer to not to have the camera view spin all the time.  These setttings are controlled by the cvars cg_truespin, cg_trueflip, and cg_trueroll.

		There are three realism settings for the three axii of movement; No Movement, Simplifed, and Real Mode for Spins, Flips, and Rolls.  All settings default to No Movement.

	No Movement (0):  Prevents any camera movement for this type of move.
	Simplified  (1):  Performs a graceful 360 degree in the rotation of the move.
	Real	    (2):  Sets the camera movement to the player model.  This is the most realistic mode and is what you would actually see if you were the player.

	Notes:
		The animations don't match up with the No Movement and Simplified Modes. You are more likely to have your camera view clip thru body parts in these modes.
		Due to the way the game engine handles the camera, anything that moves too close to the camera clips, so there are many occasions where your camera will slightly clip thru walls, bodies, etc.

	Eye Positioning:
		Since the models weren't set up for True View, many non-human have noses, mouths, etc that clip thru the camera by default.  OJP has accounted for this with the motf_trueeyeposition cvar and the Automatic Eye Adjuster.  Whenever a model is loaded, OJP checks trueview.cfg for the name of the model.  If the model is listed, the given value is automatically inputed into cg_trueeyeposition for you.  A ingame message will be given when this happens.  

		We've already provided the needed values for Raven's standard model set.  However, we can't account for every model by ourselves.  If you find a model that have facial clipping manually use cg_trueeyeposition (positive numbers to shift the view forward / negative move back) to find a good position.  Please send in your ideal settings so we can add them to the next version of OJP.

	Using True View:
		Using True View is easy.  By default, just press your first person/third person toggle button while using your Saber or in Melee Mode to activate True View.  
	
	If you wish to use True View for your gun weapons, set cg_trueguns to 1.  
		
	If you wish to have True View be the default mode for the Saber or Melee Modes, set cg_trueinvertsaber to 0.  

	If you wish to have your True View field of view (how much you can see on the screen at once), set cg_truefov to your desired fov (in degrees).  Setting cg_truefov to 0 will set your True View fov to be the same as your normal fov (cg_fov).

	If you wish to only see your saber blade while in True View, set cg_truesaberonly to 1.


	Various Bot Tweaks
	Multiplayer, Bots
	
	What It Does:

	- The Luke bot now uses the skywalker saber (single_skywalker) that is included in the game.

	- Chewie now speaks in Wookie.

	- The Lannik Racto bot now uses the correct model.


	Various Code Tweaks (VariousCode)
	Multiplayer, Code

	What It Does:

	- Fixed a bug with debug compiles that caused the animation parsing function to display a minor error whenever it processed the ROOT animation.


	Various Menu Tweaks (UITweaks)
	Multiplayer, Single Player, Menus

	What It Does:

	- Shadow options are now in both SP and MP.  Projected shadows are now an option.
 
	- Light Flare option in the MP ingame More video Options menu.

	- Dynamic Glow option in the MP ingame More video Options menu.

	- Doubled the number of skins icons that will be displayed in the player selection menu.

	Added to Setup/Options menu

		-Crosshair size - small/med/large

		-crosshair - off, or 1-8 

	 Single Player

		Added Setup/Advanced menu with the following options

		-Wireframe

		-Game speed 

		-Saber Anim Speed 

		-Pick Up Sabers

		-Melee Style (punches, or more kung-fu-ish)

		-"Hovering" health bars above each character 

		-Saber Realism 

		Added to Controls/Other

		-zoom -- zoom goggles (primary fire zooms in, secondary zooms out, cannot fire)

		-Update to missionfailed.menu (now has Quit Game as an option)

		-Melee added to ingameweaponselect.menu (no icon -- just automatically loads for new missions)


	Multiplayer


		Added Setup/Advanced menu with the following options


		-Lagometer (shows how much lag you have in an online game)

		-Framerate 

	Players can have the ADVANCED SP cvars stay the way they want if they copy/paste this to the bottom of their jaconfig.cfg:

		seta r_showtris "0"
		seta g_saberanimspeed "1"
		seta g_saberPickuppableDroppedSabers "0"
		seta cg_debugHealthBars "0"
		seta g_sabermorerealistic "0"
		seta g_debugmelee "0"
		bind z "zoom"

	Players can have the Lagometer ADVANCED MP cvar stay the way they want if they copy/paste this to the bottom of their jampconfig.cfg:

		seta cg_lagometer "0"


	Various Map Tweaks
	Multiplayer

	What It Does:

	- All basejka Duel Maps are now availible as FFA maps.  Duel maps are great for a low number of players or against bots.



	Various Server Tweaks
	Multiplayer

	What It Does:

	- g_saberdamagescale is now a server info cvar.  This means you can now use third party server browsers, like the All Seeing Eye, to create filters useing this cvar.


	Various UI Tweaks
	Multiplayer

	What It Does:

	- The map loading screen is now less vague about the server's disabled Force Powers setting.  It also now has support for the More Force Disable Options feature.

	- Server filters for OJP Basic and OJP Enhanced. (SERVERFILTERS)

	- Siege is now a selectable gametype in the Solo (quickgame) game setup menus.

	- You can now use the bot selection menu in the server/game creation menus to add specific bots in Siege.


	Various Model Fixes
	Single/Multiplayer

	- BUGFIX:  Twi'lek "skirts" 3 and 4 are now twosided (added z_newstuff.shader)

		

-----------------
0004.2 - Enhanced
-----------------

	Animation System (AnimationSys)
	Multiplayer, Animations

	What It Does:

	- New Animation Flag:  SETANIM_FLAG_PACE - Acts like a SETANIM_FLAG_RESTART flag, but only restarts if the same animation is over (animation timer ran out).  This is useful for times when you want to have the Animation Timer "pace" with a repeating animation.


	Force Fall (ForceFall)
	Multiplayer

	What It Does:

	- If you have any level of force jump you can use force fall by pressing and holding the jump button as you descend. 	The higher level force jump you have the slower the use of force power.


	Ledge Grab (LedgeGrab)
	Multiplayer, New Animations

	What It Does:
		
		Players can now grab ledges and shimmy right/left, pull up, or drop using some brand new animations!  Ledge grabs occur when you're in the air near a ledge and either face towards it or press the movement keys in the direction of the ledge.
		While on Ledge:
		Forward = Climb up onto the ledge.
		Backward = Let go of ledge.
		Right/Left = Shimmy Right/Left


	Melee System Overhaul (Melee)
	Multiplayer, Melee

	What It Does:

	- All Players in all gametypes have melee combat (WP_MELEE) by default (you don't need to cheat anymore).  
		Melee Mode:
		-----------
		Primary Fire - Punches
		Secondary Fire - Kicks
		Primary + Secondary - Grapple Moves
			Default:  	Head Lock
			+Forward:  	Punching Grapple
			+Backward:	Kneeing Grapple

	- You can now use the select saber button to quickly toggle between an ignited saber and melee mode.  You can go back to the traditional system (where that same command toggles the saber on/off by setting ojp_sabermelee to 0).  Please note that you can also use /togglesaber to toggle the saber on/off reguardless of your ojp_sabermelee setting. 
		
	- Added in headlock (BOTH_KYLE_PA_3).  Just don't press forward or backward while attempting a grapple to try a headlock.
	- Defender now comes out of the kneeing grapple facing in the correct direction.
	- Defender now plays get up animation (BOTH_GETUP3) instead of snapping at the end of a kneeing grapple.
	- Fixed the grapple body positioning.


	Seeker NPC item (SeekerNpcItem)
	Multiplayer, Inventory Item.

	What It Does:
	
	- The Seeker item now spawns a true seeker npc.  This NPC will orbit around you and shoot at enemies when in its standard mone.
	- You can command the seeker by using the item again.  When you first use the item, the seeker will either attack the target player, defend the target player (if the target is on the same team as you), or patrol the area (if the target is not a player).  Using the item again will have it return to guarding you.


-----------------
0004.3 - Vehicles
-----------------

	Droideka
	Multiplayer, Vehicle
	Model Names:
		droideka
	NOTE: THis vehicle requires OJP Basic v0.0.5 or OJP Enhanced 0.0.2 to operate correctly.

	Description:
		"Unlike the spindly battle droids, whose humanoid builds allow them a degree of versatility, droidekas are designed with one sole function in mind: the complete annihilation of their targets. The bronzen-armored combat automata has a frightening build, insectoid in its mix of curves and sharp angles. Slung at the end of heavy arms are immense twin blasters, which unleash destructive energy at a pounding pace. The destroyer droid can completely envelope itself in a globe of protective energy via its compact deflector shield generators. 
Although slow and awkward on its three-legged gait, the droideka can transform into a much speedier shape. By curling into itself, the droideka can turn into a disk-shaped form, which can roll on smooth surfaces at impressive speeds." (starwars.com/databank)


	Eopie
	Multiplayer, Vehicle
	Model Names:
		eopie - eopie with saddle
		eopiewild - eopie without saddle

	Description:
		A slow, ugly, and gassy beast of burden used on Tatooine.  Seen in Episode 1.


	Sith Speeder
	Multiplayer, Vehicle
	Model Names:
		sithspeeder

	Description:
		"A pared-down crescent-shaped conveyance, Darth Maul's Sith speeder is a very utilitarian craft stripped of all non-essential features to deliver the swiftest speeds possible. Deployed from the Sith Infiltrator, Maul's speeder bike traverses distances across land when use of his starship is deemed unnecessary.  When searching for the escaped Queen Amidala of the Naboo, Maul used the Sith speeder to soar towards the Queen's starship on Tatooine.  He lept from the moving vehicle and immediately engaged in a duel with Jedi Master Qui-Gon Jinn."  (starwars.com/databank)

	Weapons:
		(Dropped) Hovering Mines - Can be shoot.  Explodes on after a period of time.  High Damage.

	
	Snowspeeder
	Multiplayer, Vehicle
	Model Names:
		snowspeeder

	Description:
		"When stationed on Hoth, the Rebel Alliance modified T-47 airspeeders to become snowspeeders, fast flying conveyances for patrol and defense of their hidden base. It took some doing to keep the crippling cold from permanently grounding their airforce, but Rebel ingenuity overcame the relentless Hoth elements. 
		The T-47 airspeeder is a small, wedge-shaped craft with two forward-facing laser cannons. In its rear arc is a harpoon gun fitted with a heavy-duty tow cable. The snowspeeder is a two-man vessel, with a pilot and rear-facing tailgunner."  (starwars.com/databank)

	Weapons:
		(2, Forward) Rebel Blaster Cannons
		(1, Backward) Blaster Cannon

	Stap
	Multiplayer, Vehicle
	Model Name:
		stap

	Description:
		"The STAP (single trooper aerial platform) is a slim, agile craft sporting a pair of blaster cannons.  STAPs are often deployed as support vehicles in conjunction with larger craft.  The design draws inspiration from similar civilian vehicles called airhooks.  Trade Federation engineers refitted the design with greater performance and reliability.  High-voltage energy cells fuel the tiny craft's drive turbines, which afford the STAP impressive maneuverability." (starwars.com/databank) 

	Weapons:
		(2, Forward) Blaster Cannon 
	

--------------
0004.4 - Skins
--------------

	Hoth Custom Player Clothing
	Multiplayer, Single Player, Player Classes

 	What It Does:
		Adds the ability to select the Hoth clothes for all the built-in player classes.  You can now also select the color of Hoth outfit.

	Technical Notes:
		For most of the player classes, I've shared the icons by placing dummy .png icon files (so the menu tries to load them) in the player model directories and then using shader referencing to use the general icons.  The generic icons are located in the jedi_hoth directory.  
		Please note that the jedi_kdm, jedi_rm, and jedi_tf have custom torso icons and jedi_tf has a custom lower icon.  Those custom icons are found in the individual model directories.


=====================
0005 - Using Our Work
=====================

We have few rules for using our work as part of your own projects:

 - You must include this readme in any public releases of your mod.  This doesn't apply if you're only using OJP features that you wrote yourself.
 - You must treat your fellow coders and the project with respect.
 - You may NOT use our work for ANY commercial purposes without the author's direct permission.

Please don't violate these rules; they are here for everyone's benefit.

We have a public CVS repository set up to let you directly access the OJP source materials to keep up-to-date with latest additions to the project.  However, the process to access the repository is a bit complicated, so please email one of the moderators for assistance.

In addition, we are still waiting for the MP SDK so there will probably not be anything to see for a while anyway.

We suggest that you:

 - Submit any cool features from your work that you think other developers may benefit from. 
 - Keep in contact with us about your project.  The more information we have, the better we can coordinate OJP to help you and the community.  We also like to know that people are using and enjoying our works.



==============================
0006 - Submitting Stuff to OJP
==============================


We are looking mostly for new code features, but are not limited to that. We are usually looking for generic, flexible, and adaptable features that anyone can work into their own code. Features that are drastically unique or special probably do not belong here, because we believe in keeping individuality and uniqueness among mods.

We also accept patches. If you see something in our code that has a problem, you can submit a patch for it. A patch would usually be replacement code or files.

Before you consider submitting, take note that we won't let you desubmit or remove your works from the project. Allowing people to do so would cause too many problems for the project. While your work will remain your work, submitting stuff to OJP means that you give us the rights to use your work and modify it freely as part of the project forever.

In addition, your work won't necessarily be turned on or even in every compiled version of OJP.  Some features will be disabled by default to allow people to just fire up and play OJP without confusion.

That being said, if you have something to submit just contact one of the OJP moderators.

DO NOT E-MAIL MOD MATERIALS TO STAFF MEMBERS WITHOUT ASKING FIRST! Just contact one of us, tell us a summary of your patch or feature, and if we think it fits the project, we'll accept it.

If you think you would like to actively participate in developing OJP, we can give you write access to the CVS repository, so that you may work on it yourself. This doesn't have to be a commitment, but if you would like to just submit features or patches separately, go ahead.


------------------------------
0006.1 - Submission Guidelines
------------------------------

 - Document your work as much as possible.  Be sure to add mentions of your work in the readme and other project documents.

 - Make your work as clean and tight as possible.

 - Follow the coding guidelines.  Try to keep your code as separated from other code as is reasonable.  Label EVERY coding change (from basejka) with appropriate coding tags.  If you're creating a new feature, you'll get to determine what the tag name will be.  Try to pick something that is simple and easy to search for.

 - NEVER DELETE FILES/DIRECTORIES/ETC FROM THE CVS REPOSITORY.  If it is necessary, the moderators will handle it.  



===================
0007 - Known Issues
===================


-----------------------------------
0007.1 - Issues with Basic Features
-----------------------------------

	Debug Mode:

		Issue:  I'm getting a assert error whenever someone presses alt fire while firing the Asteroids version of the Y-Wing!
		Solution:  It's a bug in the Y-Wing's NPC file.  It's weapon should be set to WP_BLASTER instead of WP_NONE.  The bug is harmless as it doesn't break anything.  It's best to either fix the file or just not fly the Y-Wing while debugging.


	Hidden Player Class Clothes (Hoth):

		Issue:  In the Hoth mission of the single player campaign, your Hoth outfit will lose its color selection abilities.  
		Solution:  This is because the Hoth cold suit that is selectable in the menu is a slightly modified version of the original Hoth skin. The Hoth maps use the originals, so you're basically stuck with this issue unless you activate cheat mode and manually switch back to the selectable Hoth.

		Issue:  Twi'lek Female doesn't have selectable colors for her Hoth suit.
		Solution:  This is because the color selection system can only handle one custom color at a time.  In this case, the color is already being used for your player's skin tone.  Sorry! 


	Capture the Flag Gametype while Map Loading for Capture the Ysalamiri

		Issue:  When the map is loading for a Capture the Ysalamiri, the load screen says that we're playing Capture the Flag.
		Solution:  This isn't really a bug as it's caused by running OJP Basic without the client component.  If you wish to fix the problem, download and install OJP Basic.



==============
0008 - Credits
==============

Coding:

	Original Jedi Knight Academy source code:  Raven Software


OJP Administration:

	Documentation:  Emon, Razor Ace, UOM

	Original OJP Concept:  Razor Ace

	OJP Organizational Planning:  Emon, RenegadeofPhunk, Razor Ace

	Screenshots:  Razor Ace, Buffy, Insane Sith, Kurgan

	OJP Beta Server (MeatGrinder) Admins:  Kurgan, Lathain Valtiel, OnlyOneCannoli, Razor Ace


OJP Basic:

	Admin System:
		Auto Team Balancer:  Razor Ace
		Team Voting:  Astrosmash
		Voting Enhancements:  Razor Ace
		Additional Astroids Admin Code: Astrosmash

	Beta Testers:  JustABot

	Bug Fix 4:  -ONE-Mushroom

	Bug Fix 6:  Teancum

	Bug Fix 10:  Razor Ace, Enisform

	Bug Fix 30:  dumbledore0003

	Bug Fix 31:  Enisform, Razor Ace

	Bug Fix 32:  Razor Ace, Enisform

	Cooperative (CoOp) Mode:
		Remote Model Port:  LightNinja
		Sand Creature SP AI Port:  Tinny
		Camera Scripting Port:  Vruki
		Seeker NPC AI Fixes:  robo85045
		Everything Else:  Razor Ace

	Dismemberment Enhancements:  Tinny, Wudan

	Extra Vehicle Implementation: RenegadeOfPhunk

	German Translation:  Red Rain

	Hidden JKA Sabers:  TiM
		Menu Names:  Razor Ace

	Holocron File System:  Unique1

	Holstered Weapons:  
		Coding:  Razor Ace
		Concept/Support:  Keshire
		Holster.cfgs:  Lathain Valtiel, Razor Ace

	Linux GCC Fixes:  Adam

	Old JK2 Gametypes:  Razor Ace, -ONE-Mushroom

	Overflow Protection:  Ensiform

	Private Duel Enhancements:  Chosen One

	RGB Sabers:
		Code:  Tchouky
		RGB Saber Icon:  Red Rain
		RGB Improved Saber Core and Glow:  Lathain Valtiel
		RGB Menus:  Tchouky, Lathain Valtiel
		gcc Memory Overflow Fix:  dumbledore0003

	Siege Cvar Fix:  dumbledore0003

	Spanish Translation:  Alesh

	Splash Screen:  Tanqexe

	SP Dual/Staff Menu:  Aryyn

	TrueView Configuration File:  Razor Ace, Kurgan

	CrashLog (g_crash.c):  tjw, forty of ETPub

	Various Menu Tweaks:  Razor Ace, Teancum


OJP Enhanced:

	Flamethrower:
		Coding:  Razor Ace
		Visual Effect:  Tobe

	Force Fall:  NickR	

	Saber System:
		Coding:  Razor Ace
		Design:  Razor Ace, Ytmh, JRHockney*, and the OJP forum visitors
		Saber Combat Manual:  JRHockney*, Razor Ace

	New Saber Effects:
		Effect Files and Original Concept:  Scarlett
		Coding/Tweaks:  Razor Ace

	Weapon System:
		Coding:  Lance, Razor Ace

	Seeker NPC Item
		Coding: Phred


Vehicles:

	Droideka: 
		Model / Textures / Anims:	Duncan_10158
		Sound:				RenegadeOfPhunk

	Eopie:  Chairwalker

	Sith Speeder: 
		Model/Textures:  Monsoontide
		Vehicle Files:  Duncan_10158
		Beta Testers:  Sunburn, Nym259, Tesla, Scouttrooper, Pnut_Master

	Snowspeeder:
		Model/Vehicle Files:  tFighterPilot
		Skin:  Mars Marshel
		Beta Testers:  Jean, gothiX, Mars Marshel
		

	Stab:  
		Model/Textures:	Monsoontide
		Vehicle Files:	Duncan_10158


Everything Else:

	Razor Ace



Special Thanks:

Raven Software
George Lucas
Lucas Entertainment Company (LEC)



==========================
0009 - Contact Information
==========================

OJP Project Moderators:
(Please do not send feature suggestions directly to OJP staff.  Use the Discussion Board for that stuff.)

	Razor Ace
	Email:  razorace@hotmail.com
	MSN:  Razor Ace (razorace@hotmail.com)
	ICQ:  618641
	AIM:  razorsaces
	Yahoo:  razorsaces

	RenegadeOfPhunk
	Email:  renegadeofphunk@3dactionplanet.com
	ICQ:  253305779 (Knobby)
	(To search for any of my code, search for 'ROP')


OJP General Discussion Board:

	http://www.lucasforums.com/forumdisplay.php?s=&forumid=542


==================
0010 - Legal Stuff
==================

We are making no claim on Raven Software's, ID Software's, or LEC's intellectual properties.  The above rules only apply to the additional works created by OJP contributors.  Ravensoft's, ID's, and LEC's code and additional materials is only included for ease of use.  All applicable licensing agreements still apply.  Please don't sue us.

End of Line.


