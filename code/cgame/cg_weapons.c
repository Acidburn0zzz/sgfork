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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"
#include "../game/bg_weapon.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];
	vec3_t			angles;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 0;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 20;
	offset[1] = 0;
	offset[2] = 8;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	VectorClear(angles);
	angles[YAW] = 90;

	AnglesToAxis(angles, re->axis);
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for(item= &bg_itemlist[1]; ITEM_INDEX(item) < IT_NUM_ITEMS; item++) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !(ITEM_INDEX(item) < IT_NUM_ITEMS) ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( ITEM_INDEX(item) );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = trap_R_RegisterShader( item->icon );

	for(ammo = &bg_itemlist[1]; ITEM_INDEX(ammo) < IT_NUM_ITEMS; ammo++) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if (ITEM_INDEX(ammo) < IT_NUM_ITEMS) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}

	strcpy( path, bg_weaponlist[weaponNum].path );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_barrel.md3" );
	weaponInfo->barrelModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

	// first person view weaponmodel
	//right handed
	if (*bg_weaponlist[weaponNum].v_model) {
		strcpy( path, bg_weaponlist[weaponNum].v_model);
		weaponInfo->r_weaponModel = trap_R_RegisterModel( path);

		// left handed
#ifdef AKIMBO
		if(bg_weaponlist[weaponNum].wp_sort == WPS_PISTOL){
			COM_StripExtension( path, path, sizeof(path) );
			strcat( path, "_l.md3" );
			weaponInfo->l_weaponModel = trap_R_RegisterModel( path);
		}
#endif
	}

	if (*bg_weaponlist[weaponNum].snd_fire && weaponNum >= WP_REM58
		&& weaponNum < WP_GATLING){
		for(i = 0; i < 3; i++){
			char name[MAX_QPATH];

			Com_sprintf (name, sizeof(name), "%s%i.wav", bg_weaponlist[weaponNum].snd_fire, i+1);
			weaponInfo->flashSound[i] = trap_S_RegisterSound( name, qfalse );
		}
	} else if(weaponNum == WP_GATLING){
		for(i = 0; i < 4; i++){
			char name[MAX_QPATH];

			Com_sprintf (name, sizeof(name), "%s%i.wav", bg_weaponlist[weaponNum].snd_fire, i+1);
			weaponInfo->flashSound[i] = trap_S_RegisterSound( name, qfalse );
		}
	} else {
		weaponInfo->flashSound[0] = trap_S_RegisterSound( bg_weaponlist[weaponNum].snd_fire, qfalse );
	}

	if (*bg_weaponlist[weaponNum].snd_reload)
		weaponInfo->reloadSound = trap_S_RegisterSound( bg_weaponlist[weaponNum].snd_reload, qfalse );

	switch ( weaponNum ) {
	case WP_KNIFE:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/knife/e_knife.md3");
		break;

	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "waterExplosion" );
		cgs.media.bulletPuffShader = trap_R_RegisterShader("smokePuff");
		cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
		break;

	case WP_LIGHTNING:
	case WP_WINCHESTER66:
	case WP_SHARPS:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;

	case WP_GATLING:
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;

	case WP_REMINGTON_GAUGE:
	case WP_SAWEDOFF:
	case WP_WINCH97:
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.8f, 0.6f );
		break;

	case WP_DYNAMITE:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/dyn/e_dynamite.md3" ); // Spoon
		cgs.media.dynamiteExplosionShader = trap_R_RegisterShader( "wqfx/dynexp");
		weaponInfo->missileSound = cgs.media.dynamiteburn;
		weaponInfo->missileDlight = 30;
		VectorSet(weaponInfo->missileDlightColor, 1, 0.75f, 0.4f);
		break;

	case WP_MOLOTOV:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weapons2/molotov/e_molotov_missile.md3" ); // Spoon
		weaponInfo->missileDlight = 30;
		VectorSet(weaponInfo->missileDlightColor, 1, 0.75f, 0.4f);
		break;

	 default:
		MAKERGB( weaponInfo->flashDlightColor, 0, 0, 0 );
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	if ( itemNum < 0 || itemNum > IT_NUM_ITEMS ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, IT_NUM_ITEMS );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH ||
		item->giType == IT_ARMOR ) {
		if (*item->world_model[1]) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
==============
CG_CalculateWeaponPosition
==============
*/
#define MAX_WP_DISTANCE	0.75f
#define	ANGLE_REDUCTION	30
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;
	vec3_t	forward;
	float	pitch = cg.refdefViewAngles[PITCH];
	float	value;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	VectorCopy(cg_entities[cg.snap->ps.clientNum].currentState.pos.trDelta, forward);
	VectorNormalize(forward);
	VectorMA( origin, cg.bobfracsin*0.5f, forward, origin);

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 *
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	value = pitch/ANGLE_REDUCTION;

	angles[PITCH] += scale * fracsin * 0.01 + value;

	//angle: look up and down
	if(pitch < 0)
		pitch *= -1;

	AngleVectors( cg.refdefViewAngles, forward, NULL, NULL );
	VectorMA( origin, MAX_WP_DISTANCE*(pitch/90.0f), forward, origin );
}

/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.3
#define		COAST_TIME	500
float	CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;
	qbool firing = ((cent->currentState.eFlags & EF_FIRING) /*&& !reloading*/);

	// if this is the player which usese the gatling do prediction
	if(cg.snap->ps.clientNum == cent->currentState.clientNum &&
		!cg_nopredict.integer && !cg_synchronousClients.integer){
		firing = ( cg.predictedPlayerState.eFlags & EF_FIRING);
	}

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_gatling_loop );
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );

		if(COAST_TIME - delta)
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_gatling_loop );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if ( cent->pe.barrelSpinning == !firing) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!firing;
	}

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups ) {
	// add powerup effects
	trap_R_AddRefEntityToScene( gun );
}

/*
=============
CG_InitWeaponAnim
=============
*/
void CG_InitWeaponAnim(centity_t *cent, int weapon, int weaponAnim, lerpFrame_t *anim,
					   qbool weapon2){

	int realanim = weaponAnim;
	realanim &= ~ANIM_TOGGLEBIT;

	if(!cg.snap->ps.ammo[WP_REMINGTON_GAUGE])
		bg_weaponlist[WP_REMINGTON_GAUGE].animations[WP_ANIM_CHANGE].numFrames = 20;
	else
		bg_weaponlist[WP_REMINGTON_GAUGE].animations[WP_ANIM_CHANGE].numFrames = 49;

	//special functions
	if(realanim == WP_ANIM_SPECIAL && weapon == WP_WINCHESTER66){
		if(cg.snap->ps.powerups[PW_SCOPE] == 2)
			bg_weaponlist[WP_WINCHESTER66].animations[WP_ANIM_SPECIAL].reversed = qtrue;
		else
			bg_weaponlist[WP_WINCHESTER66].animations[WP_ANIM_SPECIAL].reversed = qfalse;
	}

	//set the animation
	anim->weaponAnim = weaponAnim;
}

/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team ) {
	refEntity_t	gun;
	refEntity_t	gun2;
	refEntity_t	flash;
	vec3_t		angles;
	weapon_t	weaponNum, weapon2Num;
	weaponInfo_t	*weapon, *weapon2=NULL;
	int i;
	qbool render = qtrue;
	centity_t	*nonPredictedCent;

	if(ps && ps->weapon == WP_SHARPS && cg.snap->ps.stats[STAT_WP_MODE]==1)
		render = qfalse;

	weaponNum = cent->currentState.weapon;
	weapon2Num = cent->currentState.frame;

	// set weapon prediction
	if(ps){
		weaponNum = cg.weapon;
		weapon2Num = cg.weapon2;
	}

	//Animation of weapons
	if(ps){
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum];

		// no prediction
		if(cg.animtime < cg.time){
			cg.weaponAnim = ps->weaponAnim;
			cg.weapon2Anim = ps->weapon2Anim;
			weaponNum = ps->weapon;
			weapon2Num = ps->weapon2;
		}

		// 2nd weapon
		if(ps->weapon2){
			// copy the animation
			if(cg.weapon != cg.weaponold){
				/*int anim = cg.weapon2Anim;
				anim &= ~ANIM_TOGGLEBIT;*/

				BG_CopyLerpFrame(&cent->pe.weapon, &cent->pe.weapon2);

				/*if(bg_weaponlist[cg.weaponold].wp_sort == WPS_PISTOL &&
					(anim == WP_ANIM_RELOAD ||
					anim == WP_ANIM_SPECIAL2) )
					cent->pe.weapon2.animationNumber = cg.weapon2Anim;*/

				if(!cg.weaponold && cg.weapon){
					cg.weapon2Anim = WP_ANIM_CHANGE;
				}
			}

			CG_InitWeaponAnim(cent, weapon2Num, cg.weapon2Anim, &cent->pe.weapon2, qtrue);
			CG_RunLerpFrame( ci, &cent->pe.weapon2, cent->pe.weapon2.weaponAnim, 1.0, weapon2Num);
		}

		// 1st weapon
		CG_InitWeaponAnim(cent, weaponNum, cg.weaponAnim, &cent->pe.weapon, qfalse);
		CG_RunLerpFrame(ci, &cent->pe.weapon, cent->pe.weapon.weaponAnim, 1.0, weaponNum);
	}

	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];

	if(weaponNum == WP_GATLING){ //doesn't have a weapon, "tagged" on the player

		int anim = cg.weaponAnim;
		anim &= ~ANIM_TOGGLEBIT;

		if(ps && cg.gatlingmodeOld && !cg.gatlingmode &&
			!(ps->stats[STAT_WEAPONS] & (1 << WP_GATLING))){
			if(anim != WP_ANIM_DROP &&
				anim != WP_ANIM_CHANGE){
				return;
			}
		}

		if(ps && ps->stats[STAT_GATLING_MODE]){

			if(cent->pe.weapon.frame == bg_weaponlist[WP_GATLING].animations[anim].numFrames){
				return;
			}

			if(anim != WP_ANIM_DROP &&
				anim != WP_ANIM_CHANGE){
				return;
			}
		}
		if(!ps){
			if(cent->currentState.powerups & (1 << PW_GATLING))
				return;
		}
	}

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	//2nd weapon if akimbo
	if((ps && ps->weapon2) || weapon2Num){

		memset( &gun2, 0, sizeof( gun2 ) );
		VectorCopy( parent->lightingOrigin, gun2.lightingOrigin );
		//VectorNegate(gun2.lightingOrigin, gun2.lightingOrigin);
		gun2.shadowPlane = parent->shadowPlane;
		gun2.renderfx = parent->renderfx;
	}

	if(ps && *bg_weaponlist[weaponNum].v_model){
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum];

		//2nd Pistol
		if(ps->weapon2){
			weapon2 = &cg_weapons[ps->weapon2];

			//assign
			gun2.frame = cent->pe.weapon2.frame;
			gun2.oldframe = cent->pe.weapon2.oldFrame;
			gun2.backlerp = cent->pe.weapon2.backlerp;

			gun2.hModel = weapon2->r_weaponModel;
		}

		// first person view
		//assign
		gun.frame = cent->pe.weapon.frame;
		gun.oldframe = cent->pe.weapon.oldFrame;
		gun.backlerp = cent->pe.weapon.backlerp;

		if(ps->weapon2)
			gun.hModel = weapon->l_weaponModel;
		else
			gun.hModel = weapon->r_weaponModel;

	} else {
		//if the player has the scope on the weapon
		if(weaponNum == WP_DYNAMITE){
			gun.hModel = cgs.media.e_dynamite;
		} else if(weaponNum == WP_KNIFE)
			gun.hModel = cgs.media.e_knife;
		else
			gun.hModel = weapon->weaponModel;

		if(weapon2Num){
			CG_RegisterWeapon(weapon2Num);
			weapon2 = &cg_weapons[weapon2Num];
			gun2.hModel = weapon2->weaponModel;
		}
	}

	if (!gun.hModel) {
		return;
	}

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
	}

	// first weapon
	if(!ps){
		// The player hold two guns.
		// weaponNum is the one on the left hand
		// weapon2Num is the one on the right hand
		if (weapon2Num) // left hand
			CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon2");
		else
			CG_PositionEntityOnTag( &gun, parent, parent->hModel, "tag_weapon");
		if(weaponNum ==  WP_DYNAMITE){
			VectorScale(gun.axis[0], 0.5, gun.axis[0]);
			VectorScale(gun.axis[1], 0.5, gun.axis[1]);
			VectorScale(gun.axis[2], 0.5, gun.axis[2]);
		} else if(weaponNum == WP_MOLOTOV){
			VectorScale(gun.axis[0], 0.65, gun.axis[0]);
			VectorScale(gun.axis[1], 0.65, gun.axis[1]);
			VectorScale(gun.axis[2], 0.65, gun.axis[2]);
		}
	} else {
		vec3_t forward;

		CG_PositionViewWeaponOnTag( &gun, parent, gun.hModel, "tag_weapon");
		// we'll use the gun.hModel as the parent model here, doesn't matter

		// go backwards some units
		AngleVectors(cg.refdefViewAngles, forward, NULL, NULL);
		VectorMA(gun.origin, -2, forward, gun.origin);

		if(ps->weapon2)
			VectorNegate(gun.axis[1], gun.axis[1]);
	}

	// add scope if nessecary
	if(ps && ps->weapon == WP_SHARPS &&
		(ps->powerups[PW_SCOPE] == 2 || ps->powerups[PW_SCOPE] == -1 )){
		refEntity_t scope;

		// make a move from the scope to the eye
		if(cg.scopetime + SCOPE_TIME > cg.time && cg.scopetime){
			refEntity_t temp;
			vec3_t	dist;
			int		delta;
			float factor;

			delta = cg.time - cg.scopetime;
			factor = (float)delta/(float)SCOPE_TIME;
			factor = sqrt(factor);

			if(cg.scopedeltatime < 0){
				factor = (float)(SCOPE_TIME-delta)/(float)SCOPE_TIME;
				factor = factor*factor;
			}
			memset( &temp, 0, sizeof( temp ) );
			CG_PositionViewWeaponOnTag( &temp, &gun, gun.hModel, "tag_eye");

			VectorSubtract(cg.refdef.vieworg, temp.origin, dist);
			//VectorMA(cg.refdef.vieworg, factor, dist, cg.refdef.vieworg);
			// now move the weapon
			VectorMA(gun.origin, factor, dist, gun.origin);
		}

		memset( &scope, 0, sizeof( scope ) );
		VectorCopy( gun.lightingOrigin, scope.lightingOrigin );
		scope.shadowPlane = gun.shadowPlane;
		scope.renderfx = gun.renderfx;

		scope.hModel = cgs.media.model_scope;
		CG_PositionViewWeaponOnTag( &scope, &gun, gun.hModel, "tag_scope");

		if(render)
			trap_R_AddRefEntityToScene( &scope );
	}

	if(render)
		CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups );

	// 2nd weapon
	if(ps && ps->weapon2){

		CG_PositionViewWeaponOnTag( &gun2, parent, gun2.hModel, "tag_weapon");

		if(render)
		CG_AddWeaponWithPowerups( &gun2, cent->currentState.powerups );

	} else if(weapon2Num){

		// The player hold two guns.
		// weapon2Num is the one on the right hand
		CG_PositionEntityOnTag( &gun2, parent, parent->hModel, "tag_weapon");

		if(render)
			CG_AddWeaponWithPowerups( &gun2, cent->currentState.powerups );
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add sparks to the burning dynamite
	if ((cent->currentState.powerups & (1 << PW_BURN)) &&
		(cg.snap->ps.clientNum != cent->currentState.clientNum ||
		cg_thirdPerson.integer)){

		if(weaponNum == WP_DYNAMITE){
			refEntity_t		sparks;
			vec3_t			origin;

			memset( &sparks, 0, sizeof( sparks ) );
			sparks.reType = RT_SPRITE;
			sparks.customShader = cgs.media.wqfx_sparks;
			sparks.radius = 10;
			sparks.renderfx = 0;

			AnglesToAxis( vec3_origin, sparks.axis );

			CG_PositionRotatedEntityOnTag( &sparks, &gun, gun.hModel, "tag_sparks");
			trap_R_AddRefEntityToScene( &sparks );

			VectorCopy(sparks.origin, origin);

			origin[2] += 20;

			// add dynamic light
			trap_R_AddLightToScene(origin, weapon->missileDlight,
				weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );

		// add burning flame to the molotov
		} else if(weaponNum == WP_MOLOTOV){
			refEntity_t		fire;
			vec3_t			origin;

			memset( &fire, 0, sizeof( fire ) );
			fire.reType = RT_SPRITE;
			fire.customShader = cgs.media.wqfx_matchfire;
			fire.radius = 2;
			fire.renderfx = parent->renderfx;

			AnglesToAxis( vec3_origin, fire.axis );

			CG_PositionRotatedEntityOnTag( &fire, &gun, gun.hModel, "tag_fire");
			trap_R_AddRefEntityToScene( &fire );

			VectorCopy(fire.origin, origin);

			origin[2] += 20;
			trap_R_AddLightToScene(origin, weapon->missileDlight,
				weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
		}
	}

	if(weaponNum == WP_GATLING)
		return;

#define SMOKE_TIME	1000
#define DELTA_TIME	75
#define LIFE_TIME	4000

	// add smoke if nessecary
	if(cg.time >= cent->lastSmokeTime && cent->lastSmokeTime && cg_gunsmoke.integer){
		refEntity_t		re;
		int contents;

		if(!cg_thirdPerson.integer && !ps)
			goto muzzle;

		if(cent->lastSmokeTime - cent->smokeTime > SMOKE_TIME){
			cent->lastSmokeTime = 0;
			cent->smokeTime = 0;
		} else
			cent->lastSmokeTime += DELTA_TIME;

		if(ps){

			// check if player is in the water
			contents = trap_CM_PointContents( cg.refdef.vieworg, 0 );
			if ( contents & CONTENTS_WATER ) {
				goto muzzle;
			}

			// get position of tag_flash
			CG_PositionRotatedEntityOnTag( &re, &gun, gun.hModel, "tag_flash");

			CG_AddSmokeParticle(re.origin, 4, 2, LIFE_TIME, 5, 0, vec3_origin);
		}
	}

	if(	cg.time >= cent->lastSmokeTime2 && cent->lastSmokeTime2 && cg_gunsmoke.integer){
		refEntity_t		re;
		int contents;

		if(!cg_thirdPerson.integer && !ps)
			goto muzzle;

		if(cent->lastSmokeTime2 - cent->smokeTime2 > SMOKE_TIME){
			cent->lastSmokeTime2 = 0;
			cent->smokeTime2 = 0;
		} else
			cent->lastSmokeTime2 += DELTA_TIME;

		if(ps){
			// check if player is in the water
			contents = trap_CM_PointContents( cg.refdef.vieworg, 0 );
			if ( contents & CONTENTS_WATER ) {
				goto muzzle;
			}

			// get position of tag_flash
			CG_PositionRotatedEntityOnTag( &re, &gun2, gun2.hModel, "tag_flash");

			CG_AddSmokeParticle(re.origin, 4, 2, LIFE_TIME, 5, 0, vec3_origin);
		}
	}

muzzle:

	// add the flash
	if ( (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME) &&
		(cg.time - cent->muzzleFlashTime2 > MUZZLE_FLASH_TIME)) {
		return;
	}

	// decide which weapon to add the flash
	for(i = 0; i < 2; i++){
		memset( &flash, 0, sizeof( flash ) );
		VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
		flash.shadowPlane = parent->shadowPlane;
		flash.renderfx = parent->renderfx;

		if((!(cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME)) && !i){

			flash.hModel = weapon->flashModel;

		} else if(!(cg.time - cent->muzzleFlashTime2 > MUZZLE_FLASH_TIME) && cent->currentState.frame){

			flash.hModel = weapon2->flashModel;
			i = 2;
		}
		if (!flash.hModel) {
			if(!i)
				return;
			else
				break;
		}

		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash.axis );

		if(i == 2){
			CG_PositionRotatedEntityOnTag( &flash, &gun2, gun2.hModel, "tag_flash");
		} else {
			CG_PositionRotatedEntityOnTag( &flash, &gun, gun.hModel, "tag_flash");
		}
		trap_R_AddRefEntityToScene( &flash );

		if ( ps || cg.renderingThirdPerson ||
			cent->currentState.number != cg.predictedPlayerState.clientNum ) {

			if(i == 2){
				if ( weapon2->flashDlightColor[0] || weapon2->flashDlightColor[1] || weapon2->flashDlightColor[2] ) {
					trap_R_AddLightToScene( flash.origin, 100 + (rand()&31), weapon2->flashDlightColor[0],
						weapon2->flashDlightColor[1], weapon2->flashDlightColor[2] );
				}

			} else {
				if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
					trap_R_AddLightToScene( flash.origin, 100 + (rand()&31), weapon->flashDlightColor[0],
						weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
				}
			}
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;
	vec3_t		angles;
	weaponInfo_t	*weapon;

	if ( ps->persistant[PERS_TEAM] >= TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( !cg_drawGun.integer ) {
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	// drop gun lower at higher fov
	if ( cg_fov.integer > 90 ) {
		fovOffset = -0.2 * ( cg_fov.integer - 90 );
	} else {
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ ps->weapon ];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = 0;
		hand.oldframe = 0;
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM] );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/

#define FIELD_WIDTH		135
#define	FIELD_HEIGHT	47
#define	TOP_HEIGHT		24

#define AMMO_BULLET_WIDTH	15
#define AMMO_CART_WIDTH		19
#define AMMO_SHARP_WIDTH	24
#define AMMO_SHELL_WIDTH	14.5
#define AMMO_HEIGHT				4.5

void CG_DrawWeaponSelect( void ) {

	vec4_t	color2[4] = {
		{1.0f, 1.0f, 1.0f, 0.5f},
		{0.0f, 0.0f, 0.0f, 0.75f},
		{0.4f, 0.4f, 0.4f, 0.75f},
		{1.0f, 0.8f, 0.0f, 1.0f},
	};
	int		i;
	int		bits;
	int		x, y, w;
	char	*name;
	int		wp_sort;
	qbool akimbo, singlepistolSelect;
	int		ammo, weapon;
	float	ammowidth;
	qhandle_t	clip;

	// in duel mode don't let them choose the weapon before the intro ends
	if(cgs.gametype == GT_DUEL && cg.introend && (cg.introend-DU_INTRO_DRAW) >= cg.time)
		return;

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if( !cg.markedweapon)
		return;

	if( cg.menu ){
		cg.markedweapon = 0;
		return;
	}

	if( cg.markedweapon != WP_AKIMBO &&
		!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << cg.markedweapon))){
		cg.markedweapon = 0;
		return;
	}

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	// draw the category-numbers
	for( i = 1; i < WPS_NUM_ITEMS; i++){
		CG_DrawPic((i-1)*FIELD_WIDTH, 0, FIELD_WIDTH, TOP_HEIGHT, cgs.media.hud_wp_top);
		//draw the number
		trap_R_SetColor(color2[0]);
		CG_DrawPic((i-1)*FIELD_WIDTH + FIELD_WIDTH/2 -15/2, (TOP_HEIGHT-15)/2, 15, 15, cgs.media.numbermoneyShaders[i]);
		trap_R_SetColor(NULL);
	}

	if(cg.markedweapon == WP_AKIMBO)
		wp_sort = WPS_PISTOL;
	else
		wp_sort = bg_weaponlist[cg.markedweapon].wp_sort;

	akimbo = (wp_sort == WPS_PISTOL &&
		(BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
		(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) ) );

	// has the player got one pistol in hand ?
	singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	x = (wp_sort-1)*FIELD_WIDTH;
	y = TOP_HEIGHT;

	for ( i = WP_KNIFE ; i < WP_NUM_WEAPONS ; i++ ) {
		gitem_t	*item = BG_ItemForWeapon(i);
		qhandle_t	shader;

		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

		if(bg_weaponlist[i].wp_sort != wp_sort)
			continue;

		CG_RegisterWeapon( i );
		shader = cg_weapons[i].weaponIcon;

		if( i == WP_SHARPS && cg.snap->ps.powerups[PW_SCOPE] == 2)
			shader = cgs.media.sharps_scope;
		else if(i == WP_SHARPS && cg.snap->ps.powerups[PW_SCOPE] )
			shader = cgs.media.scope_sharps;

		if(item->xyrelation == 1){

			//selection marker
			if( i == cg.markedweapon){
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_WIDTH-4, color2[2]);
			} else {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_WIDTH-4, color2[1]);
			}

			// draw weapon icon
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_WIDTH - 8, shader );

			//draw field
			CG_DrawPic( x, y, FIELD_WIDTH, FIELD_WIDTH, cgs.media.hud_wp_box_quad);

			weapon = 0;
			switch (bg_weaponlist[ i ].clip) {
			case WP_GATLING_CLIP:
				weapon = i;
				ammowidth = AMMO_SHARP_WIDTH;
				clip = cgs.media.hud_ammo_sharps;
			}

			// Draw the ammo amount with ammo pics
			if (weapon && cg.snap->ps.ammo[ weapon ] > 0) {
				for (ammo = 0; ammo < cg.snap->ps.ammo[ weapon ]; ammo++) {
					CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), ammowidth, AMMO_HEIGHT, clip );
				}
			}

			y += FIELD_WIDTH;

		} else {
			//selection marker
			if ( i == cg.markedweapon
				&& (bg_weaponlist[i].wp_sort != WPS_PISTOL
				|| (bg_weaponlist[i].wp_sort == WPS_PISTOL
					&& (
						!( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
						|| (( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL ) && cg.markedfirstpistol)
						)
					)
				)
			) {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[2]);
			} else {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[1]);
			}

			// draw weapon icon
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, shader );

			//draw field
			CG_DrawPic( x, y, FIELD_WIDTH, FIELD_HEIGHT, cgs.media.hud_wp_box);

			weapon = 0;
			switch (bg_weaponlist[ i ].clip) {
			case WP_BULLETS_CLIP:
				weapon = i;
				ammowidth = AMMO_BULLET_WIDTH;
				clip = cgs.media.hud_ammo_bullet;

				if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)
				// The top pistol is the left one when the player has two pistols
					weapon = WP_AKIMBO;
				break;
			case WP_CART_CLIP:
				weapon = i;
				ammowidth = AMMO_CART_WIDTH;
				clip = cgs.media.hud_ammo_cart;
				break;
			case WP_SHARPS_CLIP:
				weapon = i;
				ammowidth = AMMO_SHARP_WIDTH;
				clip = cgs.media.hud_ammo_sharps;
				break;
			case WP_SHELLS_CLIP:
				weapon = i;
				ammowidth = AMMO_SHELL_WIDTH;
				clip = cgs.media.hud_ammo_shell;
				break;
			}

			// Draw the ammo amount with ammo pics
			if (weapon && cg.snap->ps.ammo[ weapon ] > 0) {
				for (ammo = 0; ammo < cg.snap->ps.ammo[ weapon ]; ammo++) {
					CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), ammowidth, AMMO_HEIGHT, clip );
				}
			}
			else if ( ( i == WP_KNIFE || i == WP_DYNAMITE || i == WP_MOLOTOV )
				&& cg.snap->ps.ammo[ i ] > 1) {
			// Display number of knives, dynamites and molotoves
				CG_DrawSmallStringColor(x+4 + FIELD_WIDTH, y+6 + FIELD_HEIGHT / 2, va("x%i", cg.snap->ps.ammo[ i ]), color2[0]);
			}

			// if player has pistol: give option to combine it with another weapon
			/*if(bg_weaponlist[cg.snap->ps.weapon].wp_sort == WPS_PISTOL &&
				bg_weaponlist[i].wp_sort == WPS_PISTOL &&
				i != cg.snap->ps.weapon && !cg.snap->ps.weapon2) {
				CG_DrawStringExt( x+66, y+24, "FIRE2: Add", color2[3], qtrue, qfalse, 6, 15, -1);
			}*/

			y += FIELD_HEIGHT;
		}
	}

	// add akimbo pistol
	if(akimbo){
		int weapon1, weapon2;
		int leftweapon, rightweapon;

		weapon1 = BG_SearchTypeWeapon(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS], 0);
		if (weapon1 < WP_PEACEMAKER && cg.weaponSelect == WP_PEACEMAKER
		                    && cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_PEACEMAKER))
		// Player holds a Peacemaker and has a Remington58 or a Schofield.
		// The Peacemaker will be the one in the right hand.
			weapon2 = WP_PEACEMAKER;
		else if (weapon1 < WP_SCHOFIELD && cg.weaponSelect == WP_SCHOFIELD
		                     && cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_SCHOFIELD))
		// Player holds a Schofield and has a Remington58.
		// The Schofield will be the one in the right hand.
			weapon2 = WP_SCHOFIELD;
		else
			weapon2 = BG_SearchTypeWeapon(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS], weapon1);

		if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) {
		// Player has two equal pistols
			leftweapon = WP_AKIMBO;
			rightweapon = weapon1;
		}
		else {
		// The player has two different pistols
		// Try to figure out what the left one is, and what the right one is
			if (cg.weaponSelect == weapon1) {
			// Player holds the weakest pistol, so the left one is the strongest
				leftweapon = weapon2;
				rightweapon = weapon1;
			}
			else if (cg.weaponSelect == weapon2) {
			// Player holds the strongest pistol, so the left one is the weakest
				leftweapon = weapon1;
				rightweapon = weapon2;
			}
			else if (cg.weaponSelect == WP_AKIMBO) {
			// Player holds both
				leftweapon = cg.snap->ps.weapon;
				rightweapon = cg.snap->ps.weapon2;
			}
			else {
			// Player doesn't hold any pistols
				leftweapon = weapon1;		// The weakest one
				rightweapon = weapon2;	// The strongest one
			}
		}

		if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL
			&& !singlepistolSelect) {
		// Player has two equal pistols.  Draw the second one here,
		// but *ONLY* if the player is not already in single pistol mode
			if( weapon1 == cg.markedweapon && !cg.markedfirstpistol){
			// Second pistol is marked
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[2]);
			} else {
				CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[1]);
			}

			// draw weapon icon
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, cg_weapons[weapon1].weaponIcon );

			//draw field
			CG_DrawPic( x, y, FIELD_WIDTH, FIELD_HEIGHT, cgs.media.hud_wp_box);

			// Draw the ammo amount with the bullet pic
			if (cg.snap->ps.ammo[ weapon1 ] > 0) {
				for (ammo = 0; ammo < cg.snap->ps.ammo[ weapon1 ]; ammo++) {
					CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), AMMO_BULLET_WIDTH, AMMO_HEIGHT, cgs.media.hud_ammo_bullet );
				}
			}

			y += FIELD_HEIGHT;
		}

		//selection marker
		if( WP_AKIMBO == cg.markedweapon){
			CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[2]);
		} else {
			CG_FillRect(x + 2, y+2, FIELD_WIDTH - 4, FIELD_HEIGHT-4, color2[1]);
		}

		// draw weapon icon
		if(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL){
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, cg_weapons[weapon1].weaponIcon );
		} else {
			// Draw the weapon icon on the left side
			CG_DrawPic( x + 2, y+4, FIELD_WIDTH-4 , FIELD_HEIGHT - 8, cg_weapons[leftweapon].weaponIcon );
		}
		// Draw the weapon icon on the right side
		CG_DrawPic( x-2+FIELD_WIDTH, y-4+FIELD_HEIGHT, -(FIELD_WIDTH-4) , -(FIELD_HEIGHT - 8), cg_weapons[rightweapon].weaponIcon );

		//draw field
		CG_DrawPic( x, y, FIELD_WIDTH, FIELD_HEIGHT, cgs.media.hud_wp_box);

		// Draw the ammo amount with the bullet image
		// on both the left side and the right side
		if (cg.snap->ps.ammo[ leftweapon ] > 0) {
			for (ammo = 0; ammo < cg.snap->ps.ammo[ leftweapon ]; ammo++) {
				CG_DrawPic( x-4 - AMMO_BULLET_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), AMMO_BULLET_WIDTH, AMMO_HEIGHT, cgs.media.hud_ammo_bullet );
			}
		}
		if (cg.snap->ps.ammo[ rightweapon ] > 0) {
			for (ammo = 0; ammo < cg.snap->ps.ammo[ rightweapon ]; ammo++) {
				CG_DrawPic( x+4 + FIELD_WIDTH, y+4 + (ammo * (AMMO_HEIGHT + 1)), AMMO_BULLET_WIDTH, AMMO_HEIGHT, cgs.media.hud_ammo_bullet );
			}
		}

		y += FIELD_HEIGHT;
	}

	if ( cg_weapons[ cg.markedweapon ].item || cg.markedweapon == WP_AKIMBO) {

		if(cg.markedweapon == WP_AKIMBO)
			name = "Double Pistols";
		else
			name = cg_weapons[ cg.markedweapon ].item->pickup_name;

		if ( name ) {
			w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
			CG_DrawBigStringColor(x, y, name, color2[0]);
		}
	}
}


/*
===============
CG_WeaponSelectable
===============
*/
static qbool CG_WeaponSelectable( int i ) {
	if (bg_weaponlist[i].clip){
		if ( !cg.snap->ps.ammo[i] && !cg.snap->ps.ammo[bg_weaponlist[i].clip]) {
			return qfalse;
		}
	} else {
		if ( !cg.snap->ps.ammo[i]) {
			return qfalse;
		}
	}

	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		num;
	int		i, j, weapon;
	int startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qbool first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qbool singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		// Tequila comment: By torhu, see http://www.quake3world.com/forum/viewtopic.php?f=16&t=17815
		trap_SendConsoleCommand("+button14; wait; -button14");
	}

	if(!cg.markedweapon)
		startnum = cg.weaponSelect;
	else
		startnum = cg.markedweapon;

	for(j = 0; j < WPS_NUM_ITEMS; j++){

		if(startnum == WP_NONE){
			num = WPS_PISTOL + j;
		} else if(startnum == WP_AKIMBO){
			num = WPS_PISTOL + 1 + j;
		} else {
			num = bg_weaponlist[startnum].wp_sort + j;
		}

		if(num >= WPS_NUM_ITEMS){
			num = WPS_MELEE;
			cg.markedweapon = WP_KNIFE;
			break;
		}

		// Player has two equal pistols.
		// Detect a change from knife to a single pistol.
		// Weapon is set to the first pistol.
		if ( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
			&& num == WPS_PISTOL && j > 0)
			cg.markedfirstpistol = qtrue;

		for(i = 1, weapon = 0; i <= WP_NUM_WEAPONS; i++){

			// check if akimbo has to be set
			if(num == WPS_PISTOL && weapon+1 == WP_NUM_WEAPONS &&
				(BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
				( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
					&& ( !cg.markedfirstpistol || singlepistolSelect )))) {
					// One of the following is true :
					// 1) Player has two different pistols.
					// 2) Player has two equal pistols and the second pistol is selected.
					// 3) Player has two equal pistols and has already got one pistol in hand.
				cg.markedweapon = WP_AKIMBO;
				goto end;
				break;

			}
			else if (num == WPS_PISTOL
				&& ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
				&& cg.markedfirstpistol && !singlepistolSelect
				&& startnum != WP_AKIMBO && bg_weaponlist[startnum].wp_sort == WPS_PISTOL) {
			// Player has two equal pistols.
			// Change from the first pistol to the second one.

			// This must happen *ONLY* if the player is not already in single pistol mode
				cg.markedfirstpistol = qfalse;
				cg.markedweapon = startnum;
				goto end;
				break;

			} else if(!cg.markedweapon){
				if(cg.weaponSelect == WP_AKIMBO)
					weapon = WP_PEACEMAKER+i;
				else
					weapon = cg.weaponSelect+i;

				if(weapon == cg.weaponSelect)
					continue;

				if(weapon >= WP_NUM_WEAPONS)
					weapon -= WP_NUM_WEAPONS + 1;

			} else if(cg.markedweapon == WP_AKIMBO){
				cg.markedweapon = BG_SearchTypeWeapon(num, cg.snap->ps.stats[STAT_WEAPONS], 0);

				if(!cg.markedweapon)
					continue;

				goto end;
				break;
			} else {
				weapon = cg.markedweapon+i;
			}

			if(bg_weaponlist[weapon].wp_sort == num &&
				( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon ))){

				cg.markedweapon = weapon;
				goto end;
				break;
			}
		}
	}
end:

	if(first_time)
		trap_Cvar_Set("cl_menu", "1");
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		num = 0;
	int		i, j, weapon;
	int startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qbool first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qbool singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		// Tequila comment: By torhu, see http://www.quake3world.com/forum/viewtopic.php?f=16&t=17815
		trap_SendConsoleCommand("+button14; wait; -button14");
	}

	if(!cg.markedweapon)
		startnum = cg.weaponSelect;
	else
		startnum = cg.markedweapon;

	for(j = 0; j < WPS_NUM_ITEMS; j++){

		if(startnum != WP_AKIMBO && startnum != WP_NONE){
			num = bg_weaponlist[startnum].wp_sort - j;
		} else if(startnum == WP_NONE){
			num = WPS_OTHER - j;
		} else if(startnum == WP_AKIMBO){
			num = WPS_PISTOL - j;
		}

		if(num <= 0){
			num = WPS_OTHER - j + 1;
			cg.markedweapon = WP_NUM_WEAPONS;
			//break;
		}

		// Player has two equal pistols.
		// Detect a change from akimbo to a single pistol.
		if ( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
			&& num == WPS_PISTOL && startnum == WP_AKIMBO ) {
		// If the player hasn't already got a pistol in hand,
		// set to the second one from akimbo.
			if (!singlepistolSelect)
				cg.markedfirstpistol = qfalse;
			else // else, set to the first one from akimbo.
				cg.markedfirstpistol = qtrue;
		}

		for(i = 1, weapon = 0; i <= WP_NUM_WEAPONS; i++){

			// check if akimbo has to be set
			if(num == WPS_GUN && weapon == 1 &&
				(BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
				(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL))){

				cg.markedweapon = WP_AKIMBO;
				goto end;
				break;

			}
			else if (num == WPS_PISTOL
				&& ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
				&& !cg.markedfirstpistol && startnum != WP_AKIMBO
				&& bg_weaponlist[startnum].wp_sort == WPS_PISTOL) {
			// Player has two equal pistols.
			// Change from the second pistol to the first one.
				cg.markedfirstpistol = qtrue;
				cg.markedweapon = startnum;
				goto end;
				break;

			} else if(!cg.markedweapon){
				weapon = cg.weaponSelect-i;

				if(weapon == WP_NUM_WEAPONS)
					weapon--;

				if(weapon == cg.weaponSelect)
					continue;

				if(weapon == 0)
					weapon = WP_NUM_WEAPONS-1;

			} else if(cg.markedweapon == WP_AKIMBO){
				weapon = WP_NUM_WEAPONS;
				cg.markedweapon = weapon;
			} else {
				weapon = cg.markedweapon-i;
			}

			if(bg_weaponlist[weapon].wp_sort == num &&
				( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon ))){

				cg.markedweapon = weapon;
				goto end;
				break;
			}
		}
	}
end:

	if(first_time)
		trap_Cvar_Set("cl_menu", "1");
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;
	int		i, weapon;
	int		startnum;
	// is it the first time weapon_f was called since the last wp-change?
	qbool first_time = !cg.markedweapon;

	// has the player got one pistol in hand ?
	qbool singlepistolSelect = cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > 4 ) {
		return;
	}

	if(first_time) { // signal BUTTON_CHOOSE_MODE to the server
		// Tequila comment: By torhu, see http://www.quake3world.com/forum/viewtopic.php?f=16&t=17815
		trap_SendConsoleCommand("+button14; wait; -button14");
	}

	if(!cg.markedweapon)
		startnum = cg.weaponSelect;
	else
		startnum = cg.markedweapon;

	// Player has two equal pistols.
	// Change from another weapon type to pistol weapon type
	// The first pistol must be marked
	if ( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
		&& num == WPS_PISTOL
		&& startnum < WP_NUM_WEAPONS && bg_weaponlist[ startnum ].wp_sort != WPS_PISTOL) {
		cg.markedfirstpistol = qtrue;
	}

	for(i = 1, weapon = 0; i <= WP_NUM_WEAPONS; i++){

		// check if akimbo has to be set
		if(num == WPS_PISTOL && weapon+1 == WP_NUM_WEAPONS &&
			(BG_CountTypeWeapons(num, cg.snap->ps.stats[STAT_WEAPONS]) >= 2 ||
			( ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
					&& ( !cg.markedfirstpistol || singlepistolSelect )))) {
					// One of the following is true :
					// 1) Player has two different pistols.
					// 2) Player has two equal pistols and the second pistol is selected.
					// 3) Player has two equal pistols and has already got one pistol in hand.

			cg.markedweapon = WP_AKIMBO;
			break;

		}
		else if (num == WPS_PISTOL
			&& ( cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL )
			&& cg.markedfirstpistol && !singlepistolSelect
			&& startnum != WP_AKIMBO && bg_weaponlist[startnum].wp_sort == WPS_PISTOL) {
		// Player has two equal pistols.
		// Change from the first pistol to the second one.

		// This must happen *ONLY* if the player is not already in single pistol mode
			cg.markedfirstpistol = qfalse;
			if (!cg.markedweapon)
				cg.markedweapon = cg.weaponSelect;
			break;

		} else if(!cg.markedweapon){
			int delta;

			if(cg.weaponSelect == WP_AKIMBO && num == WPS_PISTOL){
				cg.markedweapon = BG_SearchTypeWeapon(num, cg.snap->ps.stats[STAT_WEAPONS], 0);

				// Player has two equal pistols.
				// Switch from akimbo to the first pistol
				if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)
					cg.markedfirstpistol = qtrue;
				break;
			}

			if(bg_weaponlist[cg.weaponSelect].wp_sort != num)
				weapon = WP_KNIFE + i;
			else
				weapon = cg.weaponSelect+i;

			if(weapon >= WP_NUM_WEAPONS){
				delta = weapon - WP_NUM_WEAPONS;
				weapon = WP_KNIFE + delta;
			}

			//if(weapon == cg.weaponSelect)
			//	continue;

		} else if(cg.markedweapon == WP_AKIMBO){
			cg.markedweapon = BG_SearchTypeWeapon(num, cg.snap->ps.stats[STAT_WEAPONS], 0);

			// Player has two equal pistols.
			// Switch from akimbo to the first pistol
			if (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)
				cg.markedfirstpistol = qtrue;
			break;
		} else {
			// always begin at the top of the list when changing the wp_sort
			if(bg_weaponlist[cg.markedweapon].wp_sort != num)
				weapon = WP_KNIFE+i;
			else
				weapon = cg.markedweapon+i;
		}

		if(weapon >= WP_NUM_WEAPONS)
			weapon = weapon - WP_NUM_WEAPONS +1;

		if(bg_weaponlist[weapon].wp_sort == num &&
			( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << weapon ))){

			cg.markedweapon = weapon;
			break;
		}
	}

	if(first_time)
		trap_Cvar_Set("cl_menu", "1");
}

/*
===============
CG_LastUsedWeapon_f
===============
*/
void CG_LastUsedWeapon_f(void) {
	int honeymug;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}
	if( cg.introend > cg.time)
		return;

	if(cg.snap->ps.stats[STAT_GATLING_MODE])
		return;

	//don't change if lastusedweapon is null
    if (!cg.lastusedweapon) {
		return;
	}
	//return if player doesn't have two pistols when using akimbo
	if (cg.lastusedweapon == WP_AKIMBO
	    && BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) < 2
	    && !(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)) {
		return;
	}
	//return if player doesn't has exactly one more pistol of the same kind.
	//NB : WP_SEC_PISTOL is only used when the player has two equal pistols.
	if (cg.lastusedweapon == WP_SEC_PISTOL
		&& ( BG_CountTypeWeapons(WPS_PISTOL, cg.snap->ps.stats[STAT_WEAPONS]) != 1
			|| !(cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) )) {
		return;
	}
	//return if player doesn't have the previously used weapon
	if (cg.lastusedweapon != WP_AKIMBO
		&& cg.lastusedweapon != WP_SEC_PISTOL
		&& !(cg.snap->ps.stats[STAT_WEAPONS] & (1 << cg.lastusedweapon)) ) {
		return;
	}

	honeymug = cg.lastusedweapon;
	// Keep cg.lastusedweapon to WP_SEC_PISTOL if the current selected weapon
	// is the other single same type pistol, because it will become the last used
	// weapon as the secondary same type pistol.
	if (honeymug == WP_SEC_PISTOL
		&& cg.weaponSelect != WP_AKIMBO
		&& bg_weaponlist[ cg.weaponSelect ].wp_sort == WPS_PISTOL) {
		// cg.weaponSelect will be restored in
		// -> ./cgame/cg_view.c, CG_DrawActiveFrame()
		cg._weaponSelect = cg.weaponSelect;
	}
	else
		cg.lastusedweapon = cg.weaponSelect;
	cg.weaponSelect = honeymug;
	cg.weaponSelectTime = cg.time;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo

***not used anymore***
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	for ( i = WP_NUM_WEAPONS -1 ; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( i ) ) {
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent, qbool altfire, int weapon ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( weapon == WP_NONE ) {
		return;
	}
	if ( weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = &cg_weapons[ weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	if(!altfire || !cent->currentState.frame){
		cent->muzzleFlashTime = cg.time;

		if((bg_weaponlist[weapon].wp_sort == WPS_PISTOL ||
			bg_weaponlist[weapon].wp_sort == WPS_GUN) && cg_gunsmoke.integer){

			if(!cent->smokeTime)
				cent->smokeTime = cg.time + 500;
			else
				cent->smokeTime = cg.time;

			cent->lastSmokeTime = cent->smokeTime;
		}
	} else {
		cent->muzzleFlashTime2 = cg.time;

		if((bg_weaponlist[weapon].wp_sort == WPS_PISTOL ||
			bg_weaponlist[weapon].wp_sort == WPS_GUN) && cg_gunsmoke.integer){

			if(!cent->smokeTime2)
				cent->smokeTime2 = cg.time + 500;
			else
				cent->smokeTime2 = cg.time;

			cent->lastSmokeTime2 = cent->smokeTime2;
		}
	}

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}

	// do brass ejection
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		weap->ejectBrassFunc( cent );
	}
}

/*
==============
CG_BottleBreak

creates 4 gibs of the bottle
==============
*/
void CG_BottleBreak(vec3_t org, vec3_t bottledir, qbool fire, vec3_t dirs[ALC_COUNT]){
	int i;
	vec3_t dir, origin;
	vec3_t	temp;

	for(i = 0; i < 4; i++){
		VectorMA(org, 8, bottledir, origin);

		VectorSet(dir, ((rand()%10)/5)-1, ((rand()%10)/5)-1, 1);
		CG_LaunchParticle(origin, dir, cgs.media.molotovgib[i], LEBS_NONE, BOUNCE_LIGHT, 4000 + rand()%2000, /*LEF_BREAKS*/0);
	}

	// launch the predicted alcohol drops
	for(i=0; i<8;i++){
		localEntity_t *le;

		VectorCopy(dirs[i], dir);

		// just set the origin first
		VectorCopy(org, temp);
		BG_SetWhiskeyDrop(NULL, org, bottledir, dir);
		VectorCopy(org, origin);

		le = CG_LaunchSpriteParticle(origin, dir, BOUNCE_LIGHT, 4000, (LEF_PARTICLE|LEF_BLOOD));
		le->refEntity.customShader = cgs.media.whiskey_drops[rand()%6];
		le->refEntity.radius = 1.0 + (rand()%10)/6;
		le->leFlags |= LEF_WHISKEY; // this marks a whiskey drop
		le->leFlags |= LEF_REMOVE;

		if(fire){
			le->leFlags |= LEF_FIRE;
			le->lightColor[0] = 1;
		} else
			le->lightColor[0] = 0;

		le->color[0] = 1;
		le->color[1] = 1;
		le->color[2] = 1;
		le->color[3] = 1;

		// now really set up the drop
		VectorCopy(temp, org);
		BG_SetWhiskeyDrop(&le->pos, org, bottledir, dir);
		VectorCopy(le->pos.trDelta, temp);
	}
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
#define SMOKEPUFF_SIZE 6
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir,
					   impactSound_t soundType,
					   int surfaceFlags, int shaderNum,
					   qbool fire, vec3_t bottledirs[ALC_COUNT],
					   int entityNum) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius_mrk;
	float			radius_exp;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	qbool		alphaFade;
	qbool		isSprite;
	int				duration;
	int				i;

	// set defaults
	mark = 0;
	radius_mrk = 32;
	radius_exp = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 0.8f;
	lightColor[1] = 0.8f;
	lightColor[2] = 0.6f;
	isSprite = qfalse;
	duration = 600;

	switch ( weapon ) {
	case WP_DYNAMITE:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.dynamiteExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		duration = 1000;
		radius_mrk = radius_exp = 64*2.5 + rand()%30; //old 64
		light = 150;//300
		isSprite = qtrue;
		break;
	case WP_MOLOTOV:
		//mod = cgs.media.dishFlashModel;
		//shader = cgs.media.dynamiteExplosionShader;
		sfx = cgs.media.impact[IMPACT_GLASS][rand()%3];
		//mark = cgs.media.burnMarkShader;
		//duration = 1000;
		//radius_mrk = radius_exp = 64;
		//light = 150;
		//isSprite = qtrue;
		break;
	case WP_REMINGTON_GAUGE:
	case WP_SAWEDOFF:
	case WP_WINCH97:
	case WP_SHARPS:
	case WP_WINCHESTER66:
	case WP_LIGHTNING:
	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
	case WP_GATLING:
		shader = cgs.media.bulletPuffShader;
		isSprite = qtrue;

		if(weapon == WP_GATLING){
			if(!(rand()%4))
				sfx = cgs.media.sfx_ric[rand()%6];
		} else
			sfx = cgs.media.sfx_ric[rand()%6];

		radius_mrk = (rand()%1) + 2;
		radius_exp = SMOKEPUFF_SIZE;
		break;
	}

	if(surfaceFlags != -1){
		for(i=0; i<NUM_PREFIXINFO; i++){
			if((surfaceFlags & prefixInfo[i].surfaceFlags) ||
				!prefixInfo[i].surfaceFlags){
				mark = cgs.media.marks[i][rand()%2];
				radius_mrk = 3;
				break;
			}
		}
	}


	if ( sfx && weapon != WP_SAWEDOFF && weapon != WP_REMINGTON_GAUGE) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	} else if(sfx){
		if(Distance(origin, cg.refdef.vieworg) > 650)
			trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod || shader) {
		le = CG_MakeExplosion( origin, dir,
							   mod,	shader,
							   duration, isSprite , radius_exp, (weapon == WP_DYNAMITE));
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
	}

	//
	// impact mark
	//
	if(mark){
		float red, green, blue;
		alphaFade = qtrue;

		if(shaderNum > -1){
			vec4_t	light, color;
			int		i;

			Vector4Copy(shaderInfo[shaderNum].color, color);
			CG_LightForPoint(origin, dir, light);

			for(i=0;i<3;i++){
				float temp = light[i]/255;
				temp = sqrt(temp);

				color[i] *= temp;
			}

			red = color[0];
			green = color[1];
			blue = color[2];

		} else {
			red = 1;
			green = 1;
			blue = 1;
		}
		if((surfaceFlags & SURF_BREAKABLE) &&
			(bg_weaponlist[weapon].wp_sort == WPS_PISTOL ||
			bg_weaponlist[weapon].wp_sort == WPS_GUN))
			CG_CreateBulletHole(origin, dir, surfaceFlags, entityNum);
		else {
			CG_ImpactMark( mark, origin, dir, random()*360,red, green, blue, 1, alphaFade, radius_mrk, qfalse, -1 );
		}
	}

	if(surfaceFlags != -1){
		CG_LaunchImpactParticle( origin, dir, surfaceFlags, shaderNum, weapon, qfalse);
	}

	// break the bottle
	if(weapon == WP_MOLOTOV){
		CG_BottleBreak(origin, dir, fire, bottledirs);
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum ) {
	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon ) {
	case WP_DYNAMITE:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH, -1, -1, qfalse, NULL, -1 );
		break;
	case WP_MOLOTOV:
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH, -1, -1, qfalse, NULL, -1 );
		break;
	default:
		break;
	}
}

/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if ( end > len ) {
		end = len;
	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, cg_tracerWidth.value, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -cg_tracerWidth.value, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -cg_tracerWidth.value, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, cg_tracerWidth.value, right, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
qbool	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors( cg.snap->ps.viewangles, forward, NULL, NULL );
		VectorMA( muzzle, 14, forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_CROUCH_WALK || anim == LEGS_CROUCHED_IDLE ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}


void CG_RailTrail (const vec3_t start, const vec3_t end, int color) {
	localEntity_t *le;
	refEntity_t   *re;
 
	le = CG_AllocLocalEntity();
	re = &le->refEntity;
 
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;
 
	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);
 
	re->shaderRGBA[0] = g_color_table[color][0] * 255;
    re->shaderRGBA[1] = g_color_table[color][1] * 255;
    re->shaderRGBA[2] = g_color_table[color][2] * 255;
    re->shaderRGBA[3] = 255;

	le->color[0] = g_color_table[color][0] * 0.75;
	le->color[1] = g_color_table[color][1] * 0.75;
	le->color[2] = g_color_table[color][2] * 0.75;
	le->color[3] = 1.0f;
}

void CG_MakeSmokePuff(entityState_t *es){
	if (cgs.glconfig.hardwareType != GLHW_RAGEPRO) {
		// ragepro can't alpha fade, so don't even bother with smoke
		int		contents;
		vec3_t	up;

		contents = trap_CM_PointContents(es->pos.trBase, 0);
		if (!(contents & CONTENTS_WATER) && (cg.snap->ps.clientNum != es->otherEntityNum ||
			cg.renderingThirdPerson || !cg_gunsmoke.integer)) {
			vec3_t	v;

			VectorSubtract(es->origin2, es->pos.trBase, v);
			VectorNormalize(v);
			VectorScale(v, 32, v);
			VectorAdd(es->pos.trBase, v, v);
			VectorSet(up, 0, 0, 8);
			CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
		}
	}
}

void CG_Do_Bubbles(const vec3_t muzzle, const vec3_t end, const vec3_t smallStep, int weapon) {
	trace_t	tr;					//Trace object
	vec3_t	currentOrigin;		//Muzzle origin for current trace
	int		tracesCount;		//Number of passed traces
	qbool	lastTrace;			//Break on that trace

	VectorCopy(muzzle,currentOrigin);

	for (tracesCount = 0; tracesCount < MAX_TRACES_COUNT; tracesCount++) {
		int shaderNum = trap_CM_BoxTrace_New(&tr, currentOrigin, end, NULL, NULL, 0, CONTENTS_WATER);

		lastTrace = (tr.fraction == 1.0f || tr.allsolid);

		if (trap_CM_PointContents(currentOrigin, 0) & CONTENTS_WATER) {
			vec3_t	shootBackOrigin;
			int		internalShaderNum;
			trace_t	internalTrace;

			if (lastTrace)
				VectorCopy(end, shootBackOrigin);
			else
				VectorCopy(tr.endpos, shootBackOrigin);

			internalShaderNum = trap_CM_BoxTrace_New(&internalTrace, shootBackOrigin, currentOrigin, NULL, NULL, 0, CONTENTS_WATER);
			if (internalTrace.fraction != 1.0f && !internalTrace.allsolid) {
				CG_LaunchImpactParticle(internalTrace.endpos, internalTrace.plane.normal, SURF_WATER, internalShaderNum, weapon, qfalse);
				CG_BubbleTrail(currentOrigin, internalTrace.endpos, 32);
			} else {
				CG_BubbleTrail(currentOrigin, shootBackOrigin, 32);
			}
		}

		if (lastTrace)
			break;

		CG_LaunchImpactParticle(tr.endpos, tr.plane.normal, SURF_WATER, shaderNum, weapon, qfalse);

		VectorAdd(tr.endpos, smallStep, currentOrigin);
	}
}

void CG_BulletFire(entityState_t *es) {
	int			seed = es->eventParm;	//Seed for spread pattern
	vec3_t		smallStep;				//Small step (~1 unit) in the fire direction
	vec3_t		traceEnd;				//Trace end point at the maximum allowed distance
	vec3_t		end;					//Trace end point with spread taken into account
	vec3_t		right, up;

	CG_MakeSmokePuff(es);

	BG_GetProjectileTraceEnd(es->pos.trBase, es->origin2, right, up, traceEnd);
	BG_GetProjectileEndAndSmallStep(es->angles[0], &seed, es->pos.trBase, traceEnd, right, up, end, smallStep);
	BG_DoProjectile(es->weapon, es->angles[1], es->pos.trBase, smallStep, end, es->otherEntityNum);
	CG_Tracer(es->pos.trBase, end);
/*DEBUG_MOD
	CG_RailTrail(es->pos.trBase, end, 1);
*/
}

void CG_ShotgunFire(entityState_t *es) {
	int			i;
	int			seed = es->eventParm;	//Seed for spread pattern
	vec3_t		smallStep;				//Small step (~1 unit) in the fire direction
	vec3_t		traceEnd;				//Trace end point at the maximum allowed distance
	vec3_t		end;					//Trace end point with spread taken into account
	vec3_t		right, up;

	CG_MakeSmokePuff(es);

	BG_GetProjectileTraceEnd(es->pos.trBase, es->origin2, right, up, traceEnd);

	for (i = 0; i < es->otherEntityNum2; i++) {
		BG_GetProjectileEndAndSmallStep(es->angles[0], &seed, es->pos.trBase, traceEnd, right, up, end, smallStep);
		BG_DoProjectile(es->weapon, es->angles[1], es->pos.trBase, smallStep, end, es->otherEntityNum);
/*DEBUG_MOD
		CG_RailTrail(es->pos.trBase, end, (i+1)%Q_COLORS_COUNT);
*/
	}
}

void CG_ProjectileHitWall(int weapon, vec3_t origin, vec3_t dir,
					   int surfaceFlags, int shaderNum,
					   int entityNum) {
	int	i;

	if (cg_entities[entityNum].currentState.eType == ET_BREAKABLE)
		surfaceFlags |= SURF_BREAKABLE;

	if (surfaceFlags == -1)
		return;

	for (i=0; i<NUM_PREFIXINFO; i++)
		if ((surfaceFlags & prefixInfo[i].surfaceFlags) || !prefixInfo[i].surfaceFlags) {
			if (surfaceFlags & SURF_BREAKABLE)
				CG_CreateBulletHole(origin, dir, surfaceFlags, entityNum);
			else
				CG_ImpactMark(cgs.media.marks[i][rand()%2], origin, dir, random()*360, shaderInfo[shaderNum].color[0], shaderInfo[shaderNum].color[1], shaderInfo[shaderNum].color[2], 1.0f, qtrue, 3, qfalse, -1);
			break;
		}

	CG_LaunchImpactParticle(origin, dir, surfaceFlags, shaderNum, weapon, qfalse);
}
