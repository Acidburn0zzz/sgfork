/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2009 Smokin' Guns

This file is part of Smokin' Guns.

Smokin' Guns is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Smokin' Guns is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Smokin' Guns; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
Q_EXPORT intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
		cgDC.cursorx = cgs.cursorX;
		cgDC.cursory = cgs.cursorY;
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];
int					cg_burnTimes[MAX_CLIENTS];
// Smokin' Guns specific
cplane_t	cg_frustum[4];

// duel intermission points
vec3_t		cg_intermission_origin[MAX_MAPPARTS];
vec3_t		cg_intermission_angles[MAX_MAPPARTS];

vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_crosshairDrawFriend;
vmCvar_t	cg_crosshair1ActivateColor;
vmCvar_t	cg_crosshair1OpponentColor;
vmCvar_t	cg_crosshair1FriendColor;
vmCvar_t	cg_crosshair1Color;
vmCvar_t	cg_drawCrosshair1;
vmCvar_t	cg_crosshair1PickUpPulse;
vmCvar_t	cg_crosshair1WidthRatio;
vmCvar_t	cg_crosshair1Transparency;
vmCvar_t	cg_crosshair2Color;
vmCvar_t	cg_drawCrosshair2;
vmCvar_t	cg_crosshair2ActivateColor;
vmCvar_t	cg_crosshair2OpponentColor;
vmCvar_t	cg_crosshair2FriendColor;
vmCvar_t	cg_crosshair2PickUpPulse;
vmCvar_t	cg_crosshair2WidthRatio;
vmCvar_t	cg_crosshair2Transparency;
vmCvar_t	cg_crosshair1X;
vmCvar_t	cg_crosshair1Y;
vmCvar_t	cg_crosshair1Size;
vmCvar_t	cg_crosshair1Health;
vmCvar_t	cg_crosshair2X;
vmCvar_t	cg_crosshair2Y;
vmCvar_t	cg_crosshair2Size;
vmCvar_t	cg_crosshair2Health;
vmCvar_t	cg_healthInjuredColor;
vmCvar_t	cg_healthBadColor;
vmCvar_t	cg_healthDeadColor;
vmCvar_t	cg_healthUsualColor;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;

vmCvar_t	cg_drawAINodes;

vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t 	cg_forceModelTeamSkin;
vmCvar_t 	cg_teamModel;
vmCvar_t 	cg_enemyModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_menu;
vmCvar_t	cg_blood;
vmCvar_t	cg_shakeOnHit;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_noVoiceChats;
vmCvar_t	cg_noVoiceText;
vmCvar_t 	cg_scorePlum;
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_noTaunt;

vmCvar_t  cg_hudFiles;
vmCvar_t  cg_hudFilesEnable;

vmCvar_t 	cg_redTeamName;
vmCvar_t 	cg_blueTeamName;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
vmCvar_t	cg_singlePlayer;
vmCvar_t	cg_enableDust;
vmCvar_t	cg_enableBreath;
vmCvar_t	cg_singlePlayerActive;
vmCvar_t	cg_recordSPDemo;
vmCvar_t	cg_recordSPDemoName;

//unlagged - client options
vmCvar_t	cg_delag;
vmCvar_t	cg_debugDelag;
vmCvar_t	cg_drawBBox;
vmCvar_t	cg_cmdTimeNudge;
vmCvar_t	sv_fps;
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
vmCvar_t	cl_timeNudge;
vmCvar_t	cg_latentSnaps;
vmCvar_t	cg_latentCmds;
vmCvar_t	cg_plOut;
//unlagged - client options

/*
==========================
SG Cvars // by Spoon
==========================
*/

vmCvar_t		cg_buydebug;
vmCvar_t		cg_serverfraglimit;
vmCvar_t		cg_serverduellimit;

vmCvar_t		cg_impactparticles;
vmCvar_t		cg_gunsmoke;
vmCvar_t		cg_addguns;
vmCvar_t		cg_hitmsg;
vmCvar_t		cg_ownhitmsg;
vmCvar_t		cg_playownflysound;
vmCvar_t		cg_showescape;
vmCvar_t		cg_debug;
vmCvar_t		cg_glowflares;
vmCvar_t		cg_boostfps;
vmCvar_t		cg_drawdebug;

//music volume
vmCvar_t		cg_musicvolume;

//talk sound
vmCvar_t		cg_talksound;

//important button bindings
vmCvar_t		cg_button_attack[2];
vmCvar_t		cg_button_altattack[2];
/*vmCvar_t		cg_button_forward[2];
vmCvar_t		cg_button_back[2];
vmCvar_t		cg_button_moveleft[2];
vmCvar_t		cg_button_moveright[2];
vmCvar_t		cg_button_turnleft[2];
vmCvar_t		cg_button_turnright[2];*/

// key list, which button is down?
//qbool	cg_keylist[K_LAST_KEY];

vec3_t			ai_nodes[MAX_AINODES];
vec3_t			ai_angles[MAX_AINODES];
int				ai_nodepointer;

vmCvar_t		hit_origin_x;
vmCvar_t		hit_origin_y;
vmCvar_t		hit_origin_z;
vmCvar_t		hit_model;

vmCvar_t		cg_warmupmessage;

vmCvar_t    cg_WeaponsListChangesEnable;
vmCvar_t    cg_ItemsListChangesEnable;

// experimental
vmCvar_t		cg_newShotgunPattern;
//vmCvar_t		cg_availablePlaylist;
vmCvar_t		cg_roundNoMoveTime;

/*int				frame_lower;
int				oldframe_lower;
float			backlerp_lower;

int				frame_upper;
int				oldframe_upper;
float			backlerp_upper;*/
int				sa_engine_inuse;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int		cvarFlags;
} cvarTable_t;

static cvarTable_t		cvarTable[] = {
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_CHEAT },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_CHEAT },

	{ &cg_drawAINodes, "cg_drawAINodes", "0", CVAR_CHEAT },

	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
	{ &cg_crosshair1Color,			"cg_crosshair1Color", "7", CVAR_ARCHIVE },
	{ &cg_drawCrosshair1,			"cg_drawCrosshair1", "4", CVAR_ARCHIVE },
	{ &cg_crosshairDrawFriend,		"cg_crosshairDrawFriend", "1", CVAR_ARCHIVE },
	{ &cg_crosshair1FriendColor,	"cg_crosshair1FriendColor", "1", CVAR_ARCHIVE },
	{ &cg_crosshair1ActivateColor,	"cg_crosshair1ActivateColor", "3", CVAR_ARCHIVE },
	{ &cg_crosshair1OpponentColor,	"cg_crosshair1OpponentColor", "2", CVAR_ARCHIVE },
	{ &cg_crosshair1PickUpPulse,	"cg_crosshair1PickUpPulse", "1", CVAR_ARCHIVE },
	{ &cg_crosshair1WidthRatio,		"cg_crosshair1WidthRatio", "1", CVAR_ARCHIVE },
	{ &cg_crosshair1Transparency,	"cg_crosshair1Transparency", "1.0", CVAR_ARCHIVE },
	{ &cg_crosshair1X,				"cg_crosshair1X", "0", CVAR_ARCHIVE },
	{ &cg_crosshair1Y,				"cg_crosshair1Y", "0", CVAR_ARCHIVE },
	{ &cg_crosshair1Size,			"cg_crosshair1Size", "24", CVAR_ARCHIVE },
	{ &cg_crosshair1Health,			"cg_crosshair1Health", "0", CVAR_ARCHIVE },
	{ &cg_crosshair2Color,			"cg_crosshair2Color", "2", CVAR_ARCHIVE },
	{ &cg_drawCrosshair2,			"cg_drawCrosshair2", "0", CVAR_ARCHIVE },
	{ &cg_crosshair2FriendColor,	"cg_crosshair2FriendColor", "1", CVAR_ARCHIVE },
	{ &cg_crosshair2ActivateColor,	"cg_crosshair2ActivateColor", "3", CVAR_ARCHIVE },
	{ &cg_crosshair2OpponentColor,	"cg_crosshair2OpponentColor", "2", CVAR_ARCHIVE },
	{ &cg_crosshair2PickUpPulse,	"cg_crosshair2PickUpPulse", "0", CVAR_ARCHIVE },
	{ &cg_crosshair2WidthRatio,		"cg_crosshair2WidthRatio", "1", CVAR_ARCHIVE },
	{ &cg_crosshair2Transparency,	"cg_crosshair2Transparency", "1.0", CVAR_ARCHIVE },
	{ &cg_crosshair2X,				"cg_crosshair2X", "0", CVAR_ARCHIVE },
	{ &cg_crosshair2Y,				"cg_crosshair2Y", "0", CVAR_ARCHIVE },
	{ &cg_crosshair2Size,			"cg_crosshair2Size", "24", CVAR_ARCHIVE },
	{ &cg_crosshair2Health,			"cg_crosshair2Health", "1", CVAR_ARCHIVE },
	{ &cg_healthInjuredColor,		"cg_healthInjuredColor", "8", CVAR_ARCHIVE },
	{ &cg_healthBadColor,			"cg_healthBadColor", "25", CVAR_ARCHIVE },
	{ &cg_healthDeadColor,			"cg_healthDeadColor", "0", CVAR_ARCHIVE },
	{ &cg_healthUsualColor,			"cg_healthUsualColor", "7", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames,		"cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "2", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_forceModelTeamSkin, "cg_forceModelTeamSkin", "1", CVAR_ARCHIVE  },
	{ &cg_enemyModel, "cg_enemyModel", DEFAULT_MODEL, CVAR_ARCHIVE  },
	{ &cg_teamModel, "cg_teamModel", DEFAULT_MODEL, CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_menu, "cl_menu", "0", CVAR_ROM },
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_shakeOnHit, "cg_shakeOnHit", "1", CVAR_ARCHIVE },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo
	{ &cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
	{ &cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO},
	{ &cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO},
	{ &cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO},
	{ &cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
	{ &cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE},

  { &cg_WeaponsListChangesEnable, "cg_WeaponsListChangesEnable", "1", CVAR_ARCHIVE },
  { &cg_ItemsListChangesEnable, "cg_ItemsListChangesEnable", "1", CVAR_ARCHIVE },
	
	{ &cg_newShotgunPattern, "cg_newShotgunPattern", "0", CVAR_ROM},
//	{ &cg_availablePlaylist, "cg_availablePlaylist", "0", CVAR_ROM},
	{ &cg_roundNoMoveTime, "cg_roundNoMoveTime", "3", CVAR_ROM},
	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

  { &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE },
  { &cg_hudFilesEnable, "cg_hudFilesEnable", "0", CVAR_ARCHIVE },
	{ &pmove_fixed, "pmove_fixed", "0", 0},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
//unlagged - client options
	{ &cg_delag, "cg_delag", "1", CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_debugDelag, "cg_debugDelag", "0", CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_drawBBox, "cg_drawBBox", "0", CVAR_CHEAT },
	{ &cg_cmdTimeNudge, "cg_cmdTimeNudge", "0", CVAR_ARCHIVE | CVAR_USERINFO },
	// this will be automagically copied from the server
	{ &sv_fps, "sv_fps", "20", 0 },
	{ &cg_projectileNudge, "cg_projectileNudge", "0", CVAR_ARCHIVE },
	{ &cg_optimizePrediction, "cg_optimizePrediction", "1", CVAR_ARCHIVE },
	{ &cl_timeNudge, "cl_timeNudge", "0", CVAR_ARCHIVE },
	{ &cg_latentSnaps, "cg_latentSnaps", "0", CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_latentCmds, "cg_latentCmds", "0", CVAR_USERINFO | CVAR_CHEAT },
	{ &cg_plOut, "cg_plOut", "0", CVAR_USERINFO | CVAR_CHEAT },
//unlagged - client options
	//SG Cvars
//	{ &cg_sg_lefthanded, "sg_lefthanded", "0", CVAR_ARCHIVE},

	{ &cg_buydebug, "cg_buydebug", "0", CVAR_ARCHIVE },
	// used by the scoreboard, in score.menu
	{ &cg_serverfraglimit, "cg_serverfraglimit", "0", CVAR_ROM },
	{ &cg_serverduellimit, "cg_serverduellimit", "0", CVAR_ROM },

	//to get amount of players in ui
	//{ &cg_teamredcount, "cg_teamredcount", "0", CVAR_ROM },
	//{ &cg_teambluecount, "cg_teambluecount", "0", CVAR_ROM },

//	{ &cg_roundmusic, "cg_roundmusic", "1", CVAR_ARCHIVE },
	{ &cg_impactparticles, "cg_impactparticles", "2", CVAR_ARCHIVE },
	{ &cg_gunsmoke, "cg_gunsmoke", "1", CVAR_ARCHIVE },
	{ &cg_addguns, "cg_addguns", "1", CVAR_ARCHIVE },
	{ &cg_playownflysound, "cg_flysound", "0", CVAR_ARCHIVE },
	{ &cg_showescape, "cg_showescape", "1", CVAR_ARCHIVE },
	{ &cg_debug, "cg_debug", "0", CVAR_ARCHIVE },
	{ &cg_glowflares, "cg_glowflares", "1", CVAR_ARCHIVE },
	{ &cg_boostfps, "cg_boostfps", "0", CVAR_TEMP },
	{ &cg_drawdebug, "cg_drawdebug", "0", CVAR_CHEAT },
	{ &cg_hitmsg, "cg_hitmsg", "1", CVAR_ARCHIVE },
	{ &cg_ownhitmsg, "cg_ownhitmsg", "1", CVAR_ARCHIVE },

	//music volume
	{ &cg_musicvolume, "cg_musicvolume", "0.25", CVAR_ARCHIVE },

	//talk sound
	{ &cg_talksound, "cg_talksound", "1", CVAR_ARCHIVE },

	{ &hit_origin_x, "hit_origin_x", "0.5", CVAR_ARCHIVE },
	{ &hit_origin_y, "hit_origin_y", "0.5", CVAR_ARCHIVE },
	{ &hit_origin_z, "hit_origin_z", "0.5", CVAR_ARCHIVE },

	{ &hit_model, "hit_model", "0", CVAR_ARCHIVE|CVAR_CHEAT },
	{ &cg_warmupmessage, "cg_warmupmessage", "1", CVAR_ARCHIVE|CVAR_CHEAT },

	//important key bindings
	{ &cg_button_attack[0], "cl_button_attack1", "0", CVAR_ROM },
	{ &cg_button_attack[1], "cl_button_attack2", "0", CVAR_ROM },

	{ &cg_button_altattack[0], "cl_button_altattack1", "0", CVAR_ROM },
	{ &cg_button_altattack[1], "cl_button_altattack1", "0", CVAR_ROM },

	/*{ &cg_button_forward[0], "cl_button_forward1", "0", CVAR_ROM },
	{ &cg_button_forward[1], "cl_button_forward2", "0", CVAR_ROM },

	{ &cg_button_back[0], "cl_button_back1", "0", CVAR_ROM },
	{ &cg_button_back[1], "cl_button_back2", "0", CVAR_ROM },

	{ &cg_button_moveleft[0], "cl_button_moveleft1", "0", CVAR_ROM },
	{ &cg_button_moveleft[1], "cl_button_moveleft2", "0", CVAR_ROM },

	{ &cg_button_moveright[0], "cl_button_moveright1", "0", CVAR_ROM },
	{ &cg_button_moveright[1], "cl_button_moveright2", "0", CVAR_ROM },

	{ &cg_button_turnleft[0], "cl_button_turnleft1", "0", CVAR_ROM },
	{ &cg_button_turnleft[1], "cl_button_turnleft2", "0", CVAR_ROM },

	{ &cg_button_turnright[0], "cl_button_turnright1", "0", CVAR_ROM },
	{ &cg_button_turnright[1], "cl_button_turnright2", "0", CVAR_ROM },*/

//	{ &cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE }
};

static int		cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	// see if the Smokin' Guns standalone engine is used.
	// If it is set "maliciously" from command line ... what the hell ...
	// The client will get a "Bad cgame system trap" message, when used in a VM
	trap_Cvar_VariableStringBuffer( "sa_engine_inuse", var, sizeof( var ) );
	sa_engine_inuse = atoi( var );

	forceModelModificationCount = cg_forceModel.modificationCount;

	trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );
	trap_Cvar_Register(NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE );
}

/*
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
//unlagged - client options
		// clamp the value between 0 and 999
		// negative values would suck - people could conceivably shoot other
		// players *long* after they had left the area, on purpose
		if ( cv->vmCvar == &cg_cmdTimeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 999 );
		}
		// cl_timenudge less than -50 or greater than 50 doesn't actually
		// do anything more than -50 or 50 (actually the numbers are probably
		// closer to -30 and 30, but 50 is nice and round-ish)
		// might as well not feed the myth, eh?
		else if ( cv->vmCvar == &cl_timeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, -50, 50 );
		}
		// don't let this go too high - no point
		else if ( cv->vmCvar == &cg_latentSnaps ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 10 );
		}
		// don't let this get too large
		else if ( cv->vmCvar == &cg_latentCmds ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, MAX_LATENT_CMDS - 1 );
		}
		// no more than 100% packet loss
		else if ( cv->vmCvar == &cg_plOut ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 100 );
		}
//unlagged - client options
		trap_Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		// FIXME E3 HACK
		trap_Cvar_Set( "teamoverlay", "1" );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if (*item->pickup_sound) {
		trap_S_RegisterSound( item->pickup_sound, qfalse );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string",
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data, qfalse );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

	CG_LoadingStage(1);

	cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", qfalse );
	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav", qfalse );
	cgs.media.snd_pistol_raise = trap_S_RegisterSound( "sound/weapons/changeto_pistol.wav", qfalse);
	cgs.media.snd_winch66_raise = trap_S_RegisterSound( "sound/weapons/changeto_winch66.wav", qfalse);
	cgs.media.snd_lightn_raise = trap_S_RegisterSound( "sound/weapons/changeto_lightning.wav", qfalse);
	cgs.media.snd_sharps_raise = trap_S_RegisterSound( "sound/weapons/changeto_sharps.wav", qfalse);
	cgs.media.snd_shotgun_raise = trap_S_RegisterSound( "sound/weapons/changeto_shotgun.wav", qfalse);
	cgs.media.snd_winch97_raise = trap_S_RegisterSound( "sound/weapons/changeto_winch97.wav", qfalse);
	CG_LoadingStage(1);

	cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav", qfalse );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );
	cgs.media.talkSound1 = trap_S_RegisterSound( "sound/player/talk1.wav", qfalse );
	cgs.media.talkSound2 = trap_S_RegisterSound( "sound/player/talk2.wav", qfalse );
	cgs.media.talkSound3 = trap_S_RegisterSound( "sound/player/talk3.wav", qfalse );
	CG_LoadingStage(1);
	cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav", qfalse);

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse);
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse);
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse);

	// death sounds
	cgs.media.snd_death[0] = trap_S_RegisterSound ( "sound/player/death_default.wav", qfalse);
	cgs.media.snd_death[1] = trap_S_RegisterSound ( "sound/player/death_head.wav", qfalse);
	cgs.media.snd_death[2] = cgs.media.snd_death[3] =
		trap_S_RegisterSound ( "sound/player/death_arm.wav", qfalse);
	cgs.media.snd_death[4] = trap_S_RegisterSound ( "sound/player/death_chest.wav", qfalse);
	cgs.media.snd_death[5] = trap_S_RegisterSound ( "sound/player/death_stomach.wav", qfalse);
	cgs.media.snd_death[6] = cgs.media.snd_death[7] =
		trap_S_RegisterSound ( "sound/player/death_leg.wav", qfalse);
	cgs.media.snd_death[8] = cgs.media.snd_death[9] =
		trap_S_RegisterSound ( "sound/player/death_falloff.wav", qfalse);
	cgs.media.snd_death[10] = trap_S_RegisterSound ( "sound/player/death_land.wav", qfalse);

	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/footsteps/step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/splash%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/clank%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/snow%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SNOW][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/sand%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SAND][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/grass%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_GRASS][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/footsteps/cloth%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_CLOTH][i] = trap_S_RegisterSound (name, qfalse);
	}

	CG_LoadingStage(1);

	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for ( i = 1 ; i <= IT_NUM_ITEMS ; i++ ) {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );

			if(!(i%5))
				CG_LoadingStage(1);
//		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		if(!(i%5))
			CG_LoadingStage(1);
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName, qfalse );
	}

	// FIXME: only needed with item
	for (i=0 ; i<6 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/weapons/ric%i.wav", i+1);
		cgs.media.sfx_ric[i] = trap_S_RegisterSound (name, qfalse);
	}
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/exp_1.wav", qfalse);
	//Spoon Sounds
	//round sounds
	/*cgs.media.roundstart = trap_S_RegisterSound( "sound/music/roundstart.wav", qfalse);
	cgs.media.roundwin = trap_S_RegisterSound( "sound/music/roundwin.wav", qfalse );
	cgs.media.roundlost = trap_S_RegisterSound( "sound/music/roundlost.wav", qfalse );*/

	cgs.media.knifehit = trap_S_RegisterSound( "sound/weapons/knife_hitflesh.wav", qfalse );
	cgs.media.knifehit2 = trap_S_RegisterSound ("sound/weapons/knife_hitwall.wav", qfalse);
	cgs.media.dynamiteburn = trap_S_RegisterSound("sound/weapons/dyn_burn.wav", qfalse);

	CG_LoadingStage(1);

	cgs.media.du_medal_snd = trap_S_RegisterSound( "sound/misc/du_medal.wav", qfalse);
	cgs.media.du_won_snd = trap_S_RegisterSound( "sound/misc/du_won.wav", qfalse);

	cgs.media.boilerhit = trap_S_RegisterSound("sound/damage/boiler.wav", qfalse);
	cgs.media.dynamitezundan = trap_S_RegisterSound("sound/weapons/dyn_burn2.wav", qfalse);
	cgs.media.sfxglassgib[0] = trap_S_RegisterSound("sound/gibs/glass1.wav", qfalse);
	cgs.media.sfxglassgib[1] = trap_S_RegisterSound("sound/gibs/glass2.wav", qfalse);
	cgs.media.sfxstonegib = trap_S_RegisterSound("sound/gibs/stone1.wav", qfalse);

	cgs.media.underwater = trap_S_RegisterSound("sound/player/underwater.wav", qfalse);
	cgs.media.taunt[0] = trap_S_RegisterSound("sound/player/taunt.wav", qfalse);
	cgs.media.taunt[1] = trap_S_RegisterSound("sound/player/spin.wav", qfalse);

	cgs.media.peacem_reload2 = trap_S_RegisterSound("sound/weapons/peacem_reload2.wav", qfalse);
	cgs.media.winch97_reload2 = trap_S_RegisterSound("sound/weapons/winch97_reload2.wav", qfalse);
	cgs.media.rifle_reload2 = trap_S_RegisterSound("sound/weapons/rifle_reload2.wav", qfalse);

	cgs.media.scopePutSound = trap_S_RegisterSound("sound/weapons/scope.wav", qfalse);
	cgs.media.gatling_build = trap_S_RegisterSound("sound/weapons/gatling_build.wav", qfalse);
	cgs.media.gatling_dism = trap_S_RegisterSound("sound/weapons/gatling_dism.wav", qfalse);

//	cgs.media.buySound = trap_S_RegisterSound("sound/misc/buy_sound.wav", qfalse);

	CG_LoadingStage(1);

	//cgs.media.reloadRifle  = trap_S_RegisterSound("sound/weapons/shotgun/reload.wav", qfalse);
	cgs.media.reloadShotgun = trap_S_RegisterSound("sound/weapons/shotgun_reload.wav", qfalse);
	//cgs.media.reloadRifle2 = trap_S_RegisterSound("sound/weapons/shotgun/reload2.wav", qfalse);
	cgs.media.sfx_gatling_loop = trap_S_RegisterSound("sound/weapons/gatling_loop.wav", qfalse);

	for (i=0 ; i<3 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/weapons/ric_wood%i.wav", i+1);
		cgs.media.impact[IMPACT_WOOD][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_metal%i.wav", i+1);
		cgs.media.impact[IMPACT_METAL][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_water%i.wav", i+1);
		cgs.media.impact[IMPACT_WATER][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_glass%i.wav", i+1);
		cgs.media.impact[IMPACT_GLASS][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_stone%i.wav", i+1);
		cgs.media.impact[IMPACT_STONE][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_sand%i.wav", i+1);
		cgs.media.impact[IMPACT_SAND][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_grass%i.wav", i+1);
		cgs.media.impact[IMPACT_GRASS][i] = trap_S_RegisterSound (name, qfalse);

		Com_sprintf (name, sizeof(name), "sound/weapons/ric_default%i.wav", i+1);
		cgs.media.impact[IMPACT_DEFAULT][i] = trap_S_RegisterSound (name, qfalse);

		// bang sounds
		Com_sprintf( name, sizeof(name), "sound/misc/bang%i.wav", i+1);
		cgs.media.bang[i] = trap_S_RegisterSound (name, qfalse);
	}

	for(i=0; i < 5; i++){
		Com_sprintf(name, sizeof(name), "music/duel_start%i.wav", i+1);
		// duel sounds
		cgs.media.duelstart[i] = trap_S_RegisterSound(name, qfalse);
	}
}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i, j;
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"hud/0",
		"hud/1",
		"hud/2",
		"hud/3",
		"hud/4",
		"hud/5",
		"hud/6",
		"hud/7",
		"hud/8",
		"hud/9",
		"hud/minus",
	};

	static char		*sb_nums_money[11] = {
		"hud/money0",
		"hud/money1",
		"hud/money2",
		"hud/money3",
		"hud/money4",
		"hud/money5",
		"hud/money6",
		"hud/money7",
		"hud/money8",
		"hud/money9",
		"hud/moneyminus",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	CG_LoadingStage(4);

	// precache status bar pics
	CG_LoadingString( "game media" );

	CG_LoadingStage(1);

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numbermoneyShaders[i] = trap_R_RegisterShader( sb_nums_money[i] );
	}

	// Spoon

	CG_LoadingStage(1);

	cgs.media.dead = trap_R_RegisterShaderNoMip( "gfx/score/dead.tga" );
	for( i= 0; i<5; i++){
		char name[64];

		Com_sprintf(name, sizeof(name), "gfx/score/medal%i.tga", i+1);
		cgs.media.won[i] = trap_R_RegisterShaderNoMip( name );
	}

	CG_LoadingStage(1);

	CG_LoadingStage(1);

	cgs.media.hud_ammo_bullet	= trap_R_RegisterShaderNoMip( "hud/ammo_bullet.tga" );
	cgs.media.hud_ammo_sharps	= trap_R_RegisterShaderNoMip( "hud/ammo_sharps.tga" );

	CG_LoadingStage(1);

	cgs.media.hud_ammo_cart		= trap_R_RegisterShaderNoMip( "hud/ammo_cart.tga");
	cgs.media.hud_ammo_shell	= trap_R_RegisterShaderNoMip( "hud/ammo_shell.tga");
	cgs.media.hud_button		= trap_R_RegisterShaderNoMip( "ui/wq3_assets/button.tga");
	cgs.media.hud_button_selected	= trap_R_RegisterShaderNoMip( "ui/wq3_assets/button_selected.tga");

	CG_LoadingStage(1);

	cgs.media.wqfx_sparks		= trap_R_RegisterShader( "wqfx/sparks" );
	cgs.media.wqfx_matchfire	= trap_R_RegisterShader( "wqfx/matchfire" );
	cgs.media.wqfx_smokeexp		= trap_R_RegisterShader( "wqfx/smokeexp");
	cgs.media.menu_end			= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_end");

	CG_LoadingStage(1);

	cgs.media.menu_tex			= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_tex");
	cgs.media.menu_top			= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_top");
	cgs.media.menu_arrow		= trap_R_RegisterShaderNoMip( "ui/wq3_assets/menu_arrow");

	CG_LoadingStage(1);

	cgs.media.hud_wp_box		= trap_R_RegisterShaderNoMip( "hud/wp_box");
	cgs.media.hud_wp_top		= trap_R_RegisterShaderNoMip( "hud/wp_top");
	cgs.media.hud_wp_box_quad	= trap_R_RegisterShaderNoMip( "hud/wp_box_quad");

	CG_LoadingStage(1);

	CG_LoadingStage(1);

	//scope
	cgs.media.scopeShader = trap_R_RegisterShaderNoMip("gfx/2d/scope");
	cgs.media.scopecrossShader = trap_R_RegisterShaderNoMip("gfx/2d/scopecrosshair");

	CG_LoadingStage(1);

	// Spoon End

	cgs.media.botSkillShaders[0] = trap_R_RegisterShaderNoMip( "gfx/score/botskill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShaderNoMip( "gfx/score/botskill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShaderNoMip( "gfx/score/botskill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShaderNoMip( "gfx/score/botskill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShaderNoMip( "gfx/score/botskill5.tga" );

	CG_LoadingStage(1);

	cgs.media.viewBloodShader = trap_R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	CG_LoadingStage(1);

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "shotgunSmokePuff" );
	CG_LoadingStage(1);
	cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail" );
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader1[i] = trap_R_RegisterShader(va("gfx/2d/crosshair1%c", 'a'+i) );
	}
	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader2[i] = trap_R_RegisterShader(va("gfx/2d/crosshair2%c", 'a'+i) );
	}
	cgs.media.crosshairFriendShader = trap_R_RegisterShader( "gfx/2d/crosshair_friend");

	CG_LoadingStage(1);

	cgs.media.backTileShader = trap_R_RegisterShader( "hud/tile" );//"gfx/2d/backtile" );

	CG_LoadingStage(1);

	cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff" );

	cgs.media.machinegunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
	cgs.media.balloonShader = trap_R_RegisterShader( "gfx/bubbles/talk.tga" );
	cgs.media.balloonBuyShader = trap_R_RegisterShader( "gfx/bubbles/buy.tga" );

	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	CG_LoadingStage(1);

	// special models for missile-weapons
	cgs.media.e_knife = trap_R_RegisterModel("models/weapons2/knife/e_knife_hold.md3");
	cgs.media.e_dynamite = trap_R_RegisterModel("models/weapons2/dyn/e_dynamite_hold.md3");
	cgs.media.e_moneybag = trap_R_RegisterModel("models/items/e_moneybag.md3");

	// scope
	cgs.media.model_scope = trap_R_RegisterModel("models/weapons2/scope/scope.md3");

	// smoke effects
	cgs.media.pistol_smoke = trap_R_RegisterShaderNoMip("wqfx/pistol_smoke.tga");
	cgs.media.ent_smoke = trap_R_RegisterShaderNoMip("wqfx/smoke.tga");

	// additional weapon icons
	cgs.media.sharps_scope = trap_R_RegisterShaderNoMip("hud/weapons/sharps_scope.tga");
	cgs.media.scope_sharps = trap_R_RegisterShaderNoMip("hud/weapons/scope_sharps.tga");

	CG_LoadingStage(1);

	//additional SG models
	for(i=0;i<NUM_GIBS; i++){
		char name[64];

		Com_sprintf(name, sizeof(name), "models/gibs/glassgib%i.md3", i+1);
		cgs.media.glassgib[i] = trap_R_RegisterModel(name);

		if(i<5){
			Com_sprintf(name, sizeof(name), "models/gibs/papergib%i.md3", i+1);
			cgs.media.clothgib[i] = trap_R_RegisterModel(name);
		}
	}

	CG_LoadingStage(1);

	cgs.media.stonegibshader = trap_R_RegisterShader("stone");
	cgs.media.metalgibshader = trap_R_RegisterShader("metal");

	cgs.media.woodgib[0]	= trap_R_RegisterModel("models/gibs/woodgib1.md3");
	cgs.media.woodgib[1]	= trap_R_RegisterModel("models/gibs/woodgib2.md3");
	cgs.media.woodgib[2]	= trap_R_RegisterModel("models/gibs/woodgib3.md3");

	cgs.media.dirtgib[0]	= trap_R_RegisterModel("models/gibs/dirtgib1.md3");
	cgs.media.dirtgib[1]	= trap_R_RegisterModel("models/gibs/dirtgib2.md3");
	cgs.media.dirtgib[2]	= trap_R_RegisterModel("models/gibs/dirtgib3.md3");

	CG_LoadingStage(1);

	// molotov gibs
	cgs.media.molotovgib[0] = trap_R_RegisterModel("models/weapons2/molotov/gib1.md3");
	cgs.media.molotovgib[1] = trap_R_RegisterModel("models/weapons2/molotov/gib2.md3");
	cgs.media.molotovgib[2] = trap_R_RegisterModel("models/weapons2/molotov/gib3.md3");
	cgs.media.molotovgib[3] = trap_R_RegisterModel("models/weapons2/molotov/gib4.md3");

	//player model additions
	cgs.media.holster_l		= trap_R_RegisterModel("models/wq3_players/holster_l.md3");
	cgs.media.holster_r		= trap_R_RegisterModel("models/wq3_players/holster_r.md3");

	cgs.media.wqfx_fire		= trap_R_RegisterShader("wqfx/fire");
	cgs.media.firesound		= trap_S_RegisterSound("sound/world/fire.wav", qfalse);

	//particles
	for(j=0;j<NUM_PREFIXINFO; j++){
		for(i=0;i<3;i++){
			char name[64];

			Com_sprintf(name, sizeof(name), "particles/particle_%s_%i", prefixInfo[j].name, i+1);
			cgs.media.particles[j][i] = trap_R_RegisterShader(name);
		}
	}
	//marks
	for(j=0;j<NUM_PREFIXINFO; j++){
		char name[64];

		if(prefixInfo[j].surfaceFlags == SURF_WATER)
			continue;

		Com_sprintf(name, sizeof(name), "gfx/wq_marks/mark_%s", prefixInfo[j].name);
		cgs.media.breakmarks[j] = trap_R_RegisterShader(name);

		for(i=0;i<2;i++){
			Com_sprintf(name, sizeof(name), "gfx/wq_marks/mark_%s_%i", prefixInfo[j].name, i+1);
			cgs.media.marks[j][i] = trap_R_RegisterShader(name);
		}
	}

	// firedrop
	cgs.media.wqfx_firedrop = trap_R_RegisterShader("wqfx/fire_drop");

	//blood and whiskey particles
	for(i=0;i<6;i++){
		char name[64];

		// blood
		Com_sprintf(name, sizeof(name), "gfx/damage/blood_%i", i+1);
		cgs.media.blood_particles[i] = trap_R_RegisterShader(name);

		// whiskey
		Com_sprintf(name, sizeof(name), "wqfx/whiskey_%i", i+1);
		cgs.media.whiskey_drops[i] = trap_R_RegisterShader(name);
	}
	//blood and whiskey marks
	for(i=0;i<3;i++){
		char name[64];

		Com_sprintf(name, sizeof(name), "gfx/damage/blood_mrk_%i", i+1);
		cgs.media.blood_marks[i] = trap_R_RegisterShader(name);

		Com_sprintf(name, sizeof(name), "wqfx/whiskey_mrk_%i", i+1);
		cgs.media.whiskey_pools[i] = trap_R_RegisterShader(name);
	}

	cgs.media.flare = trap_R_RegisterShader("wqfx/flare");
	//cgs.media.sparktrail = trap_R_RegisterShader("wqfx/spark");

	cgs.media.heartShader = trap_R_RegisterShaderNoMip( "ui/assets/statusbar/selectedhealth.tga" );

	CG_LoadingStage(1);

	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	Q_strncpyz(items, CG_ConfigString(CS_ITEMS), sizeof(items));

	for ( i = 1 ; i <= IT_NUM_ITEMS ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	// wall marks
	cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );
	cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake" );
	CG_LoadingStage(1);

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
		if(!(i %3))
			CG_LoadingStage(1);
	}

       //2d graphic of item bound
       //cgs.media.itembound = trap_R_RegisterShaderNoMip("gfx/2d/itembound.tga");

       //gatling gun models
       cgs.media.g_tripod = trap_R_RegisterModel( "models/weapons2/gatling/g_tripod.md3");
       cgs.media.g_middle = trap_R_RegisterModel( "models/weapons2/gatling/g_middle.md3");
       cgs.media.g_gatling = trap_R_RegisterModel( "models/weapons2/gatling/g_gatling.md3");
       cgs.media.g_barrel = trap_R_RegisterModel( "models/weapons2/gatling/g_barrel.md3");
       cgs.media.g_crank = trap_R_RegisterModel( "models/weapons2/gatling/g_crank.md3");
       cgs.media.g_hand = trap_R_RegisterModel( "models/weapons2/gatling/g_hand.md3");
       cgs.media.g_mag = trap_R_RegisterModel("models/weapons2/gatling/g_mag.md3");
       cgs.media.g_mag_v = trap_R_RegisterModel("models/weapons2/gatling/g_mag_v.md3");

       // duel
       // medal
       cgs.media.du_medal = trap_R_RegisterModel("models/misc/medal.md3");
       // ai_node
       cgs.media.ai_node = trap_R_RegisterModel("models/misc/ai_node.md3");
       // spectatorfly
       cgs.media.fly = trap_R_RegisterModel("models/misc/fly.md3");
       cgs.media.bpoint = trap_R_RegisterShader("wqfx/blackpoint");
       cgs.media.fly_sound = trap_S_RegisterSound("sound/misc/fly.wav", qfalse);
       // other money-models
       cgs.media.coins = trap_R_RegisterModel("models/items/coins.md3");
       cgs.media.bills = trap_R_RegisterModel("models/items/bills.md3");
       cgs.media.coins_pic = trap_R_RegisterShader("hud/weapons/coins");
       cgs.media.bills_pic = trap_R_RegisterShader("hud/weapons/bills");
       // indicate escape point
       cgs.media.indicate_lf = trap_R_RegisterShader("gfx/misc/indicator_lf");
       cgs.media.indicate_fw = trap_R_RegisterShader("gfx/misc/indicator_fw");
       cgs.media.indicate_rt = trap_R_RegisterShader("gfx/misc/indicator_rt");
       cgs.media.indicate_bk = trap_R_RegisterShader("gfx/misc/indicator_bk");

       cgs.media.quad = trap_R_RegisterModel("models/misc/quad.md3");
       // not quad damage, but a model consisting of a simple square
       // (needed for bullet holes in breakables

	// new stuff
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
	cgs.media.cursor = trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
	cgs.media.sizeCursor = trap_R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
	cgs.media.selectCursor = trap_R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );
	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );

		if(!(i %3))
			CG_LoadingStage(1);
	}
}



/*
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString(void) {
	int i;
	cg.spectatorList[0] = 0;
	for (i = 0; i < MAX_CLIENTS; i++) {
		int j, client = -1;
		// find scores for player
		for(j = 0; j < cg.numScores; j++){
			if(cg.scores[j].client == i){
				client = j;
				break;
			}
		}
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR
			&& (cgs.gametype != GT_DUEL || client == -1 || cg.scores[client].realspec)) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
		Vector4Copy( colorWhite, cg.spectatorCurrentColor );
	}
}


/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
	CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

static void CG_ClearTrackStats(void){
	int i;

	for ( i=0; i<trackInfoNum; i++){
		trackInfo[i].played = qfalse;
	}
}
static qbool CG_CheckTrackStats(void){
	int i;
	int count = 0;

	for ( i=0; i<trackInfoNum; i++){
		if(trackInfo[i].played)
			count++;
	}
	if(count == trackInfoNum)
		return qtrue;

	return qfalse;
}

/*
======================
CG_PlayMusic

======================
*/
#define	MUSICFADE_TIME	10000

void CG_PlayMusic( void ) {
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];
	int		trackNum;
	char	buffer[64];
	int		fadetime;

	// if it's duel and startup, don't play anything
	if(cgs.gametype == GT_DUEL && cg.introend >= cg.time){
		trap_Cvar_Set("s_musicvolume", "0.0");
		return;
	}

	if ( cg.custommusic ) {  // start custom music
		if ( cg.custommusic_started) {
			return;
		} else {
			char	*s;
			// make sure volume is at user's set level, in case of a fade in/out earlier
			trap_Cvar_VariableStringBuffer("cg_musicvolume", buffer, sizeof(buffer));
			trap_Cvar_Set("s_musicvolume", va("%f", cg_musicvolume.value));

			// read and parse name of custom wav file
			s = (char *)CG_ConfigString( CS_MUSIC );
			Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
			Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );
			cg.custommusic_started = qtrue;
		}
	} else { // random music system
		if ( !cg.musicfile )
			return;

		trap_Cvar_VariableStringBuffer("cg_musicvolume", buffer, sizeof(buffer));
		cg.s_volume = atof(buffer);

		//fade in
		fadetime = cg.time - cg.playmusic_starttime;
		if(fadetime < MUSICFADE_TIME && fadetime >= 0 && cg.playmusic_starttime){
			float	newvolume;

			newvolume = (float)fadetime/MUSICFADE_TIME*(float)cg.s_volume;

			/*CG_Printf("%f %i", cg.s_volume, fadetime);
			CG_Printf(" %f\n", newvolume);*/

			trap_Cvar_Set("s_musicvolume", va("%f", newvolume));
			return;
		}

		//fade out
		fadetime = cg.playmusic_endtime - cg.time;
		if(fadetime < MUSICFADE_TIME && fadetime >= 0 && cg.playmusic_endtime){
			float	newvolume;

			newvolume = (float)fadetime/MUSICFADE_TIME*(float)cg.s_volume;

			/*CG_Printf("%f %i", cg.s_volume, fadetime);
			CG_Printf(" %f\n", newvolume);*/

			trap_Cvar_Set("s_musicvolume", va("%f", newvolume));
			return;
		}

		if(cg.playmusic_starttime)
			trap_Cvar_Set("s_musicvolume", va("%f", cg.s_volume));

		if(cg.playmusic_endtime > cg.time)
			return;

		if(CG_CheckTrackStats())
			CG_ClearTrackStats();

		do {
			trackNum = rand()%trackInfoNum;

		} while(trackInfo[trackNum].played || trackNum == cg.oldTrackNum);

		Com_sprintf( parm1, sizeof(parm1), trackInfo[trackNum].path);
		Com_sprintf( parm2, sizeof(parm2), trackInfo[trackNum].path);
		trackInfo[trackNum].played = qtrue;
		cg.playmusic_endtime = cg.time + trackInfo[trackNum].replays*trackInfo[trackNum].length;
		cg.playmusic_starttime = cg.time;
		cg.oldTrackNum = trackNum;
	}

	trap_S_StartBackgroundTrack( parm1, parm2 );
}

void CG_SelectMusic( void ) {
	char *s = (char *)CG_ConfigString( CS_MUSIC );

	if ( s[0] ) {  // enable random music system
		cg.custommusic = qtrue;
	} else {  // enable custom music for this map
		cg.custommusic = qfalse;
		cg.custommusic_started = qfalse;
	}
}

char *CG_GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qbool CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}

		if (Q_stricmp(token.string, "scrollbarSize") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.scrollbarsize)) {
				return qfalse;
			}
			continue;
		}
		if (Q_stricmp(token.string, "sliderWidth") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.sliderwidth)) {
			   return qfalse;
			}
			continue;
		}
		if (Q_stricmp(token.string, "sliderHeight") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.sliderheight)) {
			   return qfalse;
			}
			continue;
		}
		if (Q_stricmp(token.string, "sliderthumbWidth") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.sliderthumbwidth)) {
			   return qfalse;
			}
			continue;
		}
		if (Q_stricmp(token.string, "sliderthumbHeight") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.sliderthumbheight)) {
			   return qfalse;
			}
			continue;
		}
		if (Q_stricmp(token.string, "sliderBar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "sliderThumb") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "sliderThumbSel") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.sliderThumb_sel = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarHorz") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarHorz = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarVert") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarVert = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarThumb") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarArrowUp") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarArrowDown") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarArrowLeft") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "scrollBarArrowRight") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxBase") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxRed") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxYellow") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxGreen") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxTeal") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxBlue") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxCyan") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}
		if (Q_stricmp(token.string, "fxWhite") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
			   return qfalse;
			}
			cgDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip( tempStr);
			continue;
		}

	}
	return qfalse;
}

void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource(menuFile);
	if (!handle)
		return;

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}


		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	trap_PC_FreeSource(handle);
}

qbool CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, qtrue);

		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token || token[0] == 0 ) {
			return qfalse;
		}

		CG_ParseMenu(token);
	}
	return qfalse;
}



void CG_LoadMenus(const char *menuFile) {
	char	*token;
	char *p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
	if (!f) {
		trap_Error( va( S_COLOR_RED "menu file not found: ui/hud.txt, unable to continue!\n" ) );
	}

	if ( len >= MAX_MENUDEFFILE ) {
		trap_Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) {
			if (CG_Load_Menu(&p)) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}



static qbool CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
	return qfalse;
}


static int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;
	if (feederID == FEEDER_REDTEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_RED || cg.scores[i].team == TEAM_RED_SPECTATOR) {
				count++;
			}
		}
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_BLUE || cg.scores[i].team == TEAM_BLUE_SPECTATOR) {
				count++;
			}
		}
	} else if (feederID == FEEDER_SCOREBOARD) {
		for(i = 0; i < cg.numScores; i++){
			if(cg.scores[i].team == TEAM_FREE || (cgs.gametype == GT_DUEL &&
				!cg.scores[i].realspec && cg.scores[i].team == TEAM_SPECTATOR)){
				count++;
			}
		}
	}
	return count;
}


void CG_SetScoreSelection(void *p) {
	menuDef_t *menu = (menuDef_t*)p;
	playerState_t *ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == TEAM_RED) {
			red++;
		} else if (cg.scores[i].team == TEAM_BLUE) {
			blue++;
		}
		if (ps->clientNum == cg.scores[i].client) {
			cg.selectedScore = i;
		}
	}

	if (menu == NULL) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM ) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
	}
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM ) {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team || cg.scores[i].team-3 == team) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	} else {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_FREE ||
				(cgs.gametype == GT_DUEL && !cg.scores[i].realspec
				&& cg.scores[i].team == TEAM_SPECTATOR)) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) {
	int trueteam;
	int scoreIndex = 0;
	clientInfo_t *info = NULL;
	int team = -1;
	score_t *sp = NULL;

	*handle = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

	trueteam = info->team;

	if (info && info->infoValid) {

		if(column >= 2 && cgs.gametype >= GT_TEAM)
			column += 1;

		switch (column) {
			case 0:
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
			break;
			case 1:
				if (team == -1 && cgs.gametype != GT_DUEL) {
					return "";
				} else {
					if(trueteam == TEAM_RED_SPECTATOR || trueteam == TEAM_BLUE_SPECTATOR)
						*handle = cgs.media.dead;//CG_StatusHandle(info->teamTask);
					if(cgs.gametype == GT_DUEL && !sp->realspec && trueteam >= TEAM_SPECTATOR)
						*handle = cgs.media.dead;
				}
			break;
			case 2:
				// give the number of stars
				if(info->wins && cgs.gametype == GT_DUEL){
					if(info->wins <= 5)
						*handle = cgs.media.won[info->wins-1];
					return "";
				}
			break;
			case 3:
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
					return "Ready";
				}
			break;
			case 4:
				if(cgs.gametype == GT_DUEL)
					return va("(%i) %s", info->losses, info->name);
				return info->name;
			break;
			case 5:
				return va("%i", info->score);
			break;
			case 6:
				return va("%4i", sp->time);
			break;
			case 7:
				if ( sp->ping == -1 ) {
					return "connecting";
				}
				return va(" %4i", sp->ping);
			break;
		}
	}

	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) {
	return 0;
}

static void CG_FeederSelection(float feederID, int index) {
	if ( cgs.gametype >= GT_TEAM ) {
		int i, count;
		int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (index == count) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}
}
static float CG_Cvar_Get(const char *cvar) {
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
	return atof(buff);
}

void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style) {
	UI_Text_Paint(x, y, scale, color, text, 0, limit, style);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
	switch (ownerDraw) {
	  case CG_GAME_TYPE:
			return UI_Text_Width(CG_GameTypeString(), scale, 0);
	  case CG_GAME_STATUS:
			return UI_Text_Width(CG_GetGameStatusText(), scale, 0);
			break;
	  case CG_KILLER:
			return UI_Text_Width(CG_GetKillerText(), scale, 0);
			break;
	  case CG_RED_NAME:
			return UI_Text_Width(cg_redTeamName.string, scale, 0);
			break;
	  case CG_BLUE_NAME:
			return UI_Text_Width(cg_blueTeamName.string, scale, 0);
			break;


	}
	return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
  return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
  trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
  trap_CIN_RunCinematic(handle);
}

/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu( void ) {
	const char	*hudSet;
	char		buff[1024];

	cgDC.aspectWidthScale = ( ( 640.0f * cgs.glconfig.vidHeight ) /
							( 480.0f * cgs.glconfig.vidWidth ) );
	cgDC.xscale = cgs.glconfig.vidWidth / 640.0f;
	cgDC.yscale = cgs.glconfig.vidHeight / 480.0f;

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	cgDC.registerFont = &trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = trap_Cvar_Set;
	cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &trap_S_StartLocalSound;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &trap_Key_SetBinding;
	//cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	//cgDC.executeText = &trap_Cmd_ExecuteText;
	cgDC.Error = &Com_Error;
	cgDC.Print = &Com_Printf;
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &trap_S_RegisterSound;
	cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;
	cgDC.hudloading = qtrue;

	Init_Display(&cgDC);

	Menu_Reset();

	trap_Cvar_VariableStringBuffer( "cg_hudFiles", buff, sizeof( buff ) );
	hudSet = buff;

	if( !cg_hudFilesEnable.integer || hudSet[ 0 ] == '\0' )
		hudSet = "ui/hud.txt";

	CG_LoadMenus(hudSet);
	cgDC.hudloading = qfalse;
}

void CG_AssetCache( void ) {
	//if (Assets.textFont == NULL) {
	//  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBarHorz = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarVert = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
	cgDC.Assets.sliderThumb_sel = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB_SEL );
	cgDC.Assets.scrollbarsize = SCROLLBAR_SIZE;
	cgDC.Assets.sliderwidth = SLIDER_WIDTH;
	cgDC.Assets.sliderheight = SLIDER_HEIGHT;
	cgDC.Assets.sliderthumbwidth = SLIDER_THUMB_WIDTH;
	cgDC.Assets.sliderthumbheight = SLIDER_THUMB_HEIGHT;
}
/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;
	char		name[64], name2[64];

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= trap_R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropB		= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	//Load shared information:
	CG_Printf ("-----------------------------------\n");
	config_GetInfos( PFT_WEAPONS | PFT_ITEMS );
	CG_Printf ("-----------------------------------\n");

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	String_Init();

	CG_AssetCache();
	CG_LoadHudMenu();      // load new hud stuff

	cg.weaponSelect = WP_REM58;
	cg.lastusedweapon = WP_NONE;

	//menu
	cg.menu = MENU_NONE;
	cg.menustat = 0;
	cg.menuitem = 0;
	trap_Cvar_Set("cl_menu", "0");

	// reset aipointer
	ai_nodepointer = 0;

	// old servers

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

	//load shader-infos
	COM_StripExtension( cgs.mapname, name2, sizeof(name2) );
	Com_sprintf(name, sizeof(name), "%s.tex", name2);
	CG_ParseTexFile( name );

	//load music.cfg
	if(CG_ParseMusicFile()){
		cg.musicfile = qtrue;
	} else {
		cg.musicfile = qfalse;
	}

	cg.loading = qtrue;		// force players to load instead of defer

	//set to zero
	CG_LoadingStage(0);
	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

	cg.loading = qfalse;	// future players will be deferred

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_SelectMusic();
	trap_Cvar_Set("s_musicvolume", "0.0");

	CG_LoadingString( "" );

	CG_InitTeamChat();

	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds( qtrue );
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}

