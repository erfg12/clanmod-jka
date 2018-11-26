//
// NPC_move.cpp
//
#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

void G_Cylinder( vec3_t start, vec3_t end, float radius, vec3_t color );

qboolean G_BoundsOverlap(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
int NAV_Steer( gentity_t *self, vec3_t dir, float distance );
extern int GetTime ( int lastTime );

navInfo_t	frameNavInfo;
extern qboolean FlyingCreature( gentity_t *ent );

#include "../namespace_begin.h"
extern qboolean PM_InKnockDown( playerState_t *ps );
#include "../namespace_end.h"

/*
-------------------------
NPC_ClearPathToGoal
-------------------------
*/

qboolean NPC_ClearPathToGoal( vec3_t dir, gentity_t *goal )
{
	trace_t	trace;
	float radius, dist, tFrac;

	//FIXME: What does do about area portals?  THIS IS BROKEN
	//if ( gi.inPVS( NPC->r.currentOrigin, goal->r.currentOrigin ) == qfalse )
	//	return qfalse;

	//Look ahead and see if we're clear to move to our goal position
	if ( NAV_CheckAhead( NPC, goal->r.currentOrigin, &trace, ( NPC->clipmask & ~CONTENTS_BODY )|CONTENTS_BOTCLIP ) )
	{
		//VectorSubtract( goal->r.currentOrigin, NPC->r.currentOrigin, dir );
		return qtrue;
	}

	if (!FlyingCreature(NPC))
	{
		//See if we're too far above
		if ( fabs( NPC->r.currentOrigin[2] - goal->r.currentOrigin[2] ) > 48 )
			return qfalse;
	}

	//This is a work around
	radius = ( NPC->r.maxs[0] > NPC->r.maxs[1] ) ? NPC->r.maxs[0] : NPC->r.maxs[1];
	dist = Distance( NPC->r.currentOrigin, goal->r.currentOrigin );
	tFrac = 1.0f - ( radius / dist );

	if ( trace.fraction >= tFrac )
		return qtrue;

	//See if we're looking for a navgoal
	if ( goal->flags & FL_NAVGOAL )
	{
		//Okay, didn't get all the way there, let's see if we got close enough:
		if ( NAV_HitNavGoal( trace.endpos, NPC->r.mins, NPC->r.maxs, goal->r.currentOrigin, NPCInfo->goalRadius, FlyingCreature( NPC ) ) )
		{
			//VectorSubtract(goal->r.currentOrigin, NPC->r.currentOrigin, dir);
			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NPC_CheckCombatMove
-------------------------
*/

/*ID_INLINE */qboolean NPC_CheckCombatMove( void )
{
	//return NPCInfo->combatMove;
	if ( ( NPCInfo->goalEntity && NPC->enemy && NPCInfo->goalEntity == NPC->enemy ) || ( NPCInfo->combatMove ) )
	{
		return qtrue;
	}

	if ( NPCInfo->goalEntity && NPCInfo->watchTarget )
	{
		if ( NPCInfo->goalEntity != NPCInfo->watchTarget )
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NPC_LadderMove
-------------------------
*/

static void NPC_LadderMove( vec3_t dir )
{
	//FIXME: this doesn't guarantee we're facing ladder
	//ALSO: Need to be able to get off at top
	//ALSO: Need to play an anim
	//ALSO: Need transitionary anims?
	
	if ( ( dir[2] > 0 ) || ( dir[2] < 0 && NPC->client->ps.groundEntityNum == ENTITYNUM_NONE ) )
	{
		//Set our movement direction
		ucmd.upmove = (dir[2] > 0) ? 127 : -127;

		//Don't move around on XY
		ucmd.forwardmove = ucmd.rightmove = 0;
	}
}

/*
-------------------------
NPC_GetMoveInformation
-------------------------
*/

qboolean NPC_GetMoveInformation( vec3_t dir, float *distance )
{
	//NOTENOTE: Use path stacks!

	//Make sure we have somewhere to go
	if ( NPCInfo->goalEntity == NULL )
		return qfalse;

	//Get our move info
	VectorSubtract( NPCInfo->goalEntity->r.currentOrigin, NPC->r.currentOrigin, dir );
	*distance = VectorNormalize( dir );
	
	VectorCopy( NPCInfo->goalEntity->r.currentOrigin, NPCInfo->blockedDest );

	return qtrue;
}

/*
-------------------------
NAV_GetLastMove
-------------------------
*/

void NAV_GetLastMove( navInfo_t *info )
{
	*info = frameNavInfo;
}

/*
-------------------------
NPC_GetMoveDirection
-------------------------
*/

qboolean NPC_GetMoveDirection( vec3_t out, float *distance )
{
	vec3_t		angles;

	//Clear the struct
	memset( &frameNavInfo, 0, sizeof( frameNavInfo ) );

	//Get our movement, if any
	if ( NPC_GetMoveInformation( frameNavInfo.direction, &frameNavInfo.distance ) == qfalse )
		return qfalse;

	//Setup the return value
	*distance = frameNavInfo.distance;

	//For starters
	VectorCopy( frameNavInfo.direction, frameNavInfo.pathDirection );

	//If on a ladder, move appropriately
	if ( NPC->watertype & CONTENTS_LADDER )
	{
		NPC_LadderMove( frameNavInfo.direction );
		return qtrue;
	}

	//Attempt a straight move to goal
	if ( NPC_ClearPathToGoal( frameNavInfo.direction, NPCInfo->goalEntity ) == qfalse )
	{
		//See if we're just stuck
		if ( NAV_MoveToGoal( NPC, &frameNavInfo ) == WAYPOINT_NONE )
		{
			//Can't reach goal, just face
			vectoangles( frameNavInfo.direction, angles );
			NPCInfo->desiredYaw	= AngleNormalize360( angles[YAW] );		
			VectorCopy( frameNavInfo.direction, out );
			*distance = frameNavInfo.distance;
			return qfalse;
		}

		frameNavInfo.flags |= NIF_MACRO_NAV;
	}

	//Avoid any collisions on the way
	if ( NAV_AvoidCollision( NPC, NPCInfo->goalEntity, &frameNavInfo ) == qfalse )
	{
		//FIXME: Emit a warning, this is a worst case scenario
		//FIXME: if we have a clear path to our goal (exluding bodies), but then this
		//			check (against bodies only) fails, shouldn't we fall back 
		//			to macro navigation?  Like so:
		if ( !(frameNavInfo.flags&NIF_MACRO_NAV) )
		{//we had a clear path to goal and didn't try macro nav, but can't avoid collision so try macro nav here
			//See if we're just stuck
			if ( NAV_MoveToGoal( NPC, &frameNavInfo ) == WAYPOINT_NONE )
			{
				//Can't reach goal, just face
				vectoangles( frameNavInfo.direction, angles );
				NPCInfo->desiredYaw	= AngleNormalize360( angles[YAW] );		
				VectorCopy( frameNavInfo.direction, out );
				*distance = frameNavInfo.distance;
				return qfalse;
			}

			frameNavInfo.flags |= NIF_MACRO_NAV;
		}
	}

	//Setup the return values
	VectorCopy( frameNavInfo.direction, out );
	*distance = frameNavInfo.distance;

	return qtrue;
}

/*
-------------------------
NPC_GetMoveDirectionAltRoute
-------------------------
*/
extern int	NAVNEW_MoveToGoal( gentity_t *self, navInfo_t *info );
extern qboolean NAVNEW_AvoidCollision( gentity_t *self, gentity_t *goal, navInfo_t *info, qboolean setBlockedInfo, int blockedMovesLimit );
qboolean NPC_GetMoveDirectionAltRoute( vec3_t out, float *distance, qboolean tryStraight )
{
	vec3_t		angles;

	NPCInfo->aiFlags &= ~NPCAI_BLOCKED;

	//Clear the struct
	memset( &frameNavInfo, 0, sizeof( frameNavInfo ) );

	//Get our movement, if any
	if ( NPC_GetMoveInformation( frameNavInfo.direction, &frameNavInfo.distance ) == qfalse )
		return qfalse;

	//Setup the return value
	*distance = frameNavInfo.distance;

	//For starters
	VectorCopy( frameNavInfo.direction, frameNavInfo.pathDirection );

	//If on a ladder, move appropriately
	if ( NPC->watertype & CONTENTS_LADDER )
	{
		NPC_LadderMove( frameNavInfo.direction );
		return qtrue;
	}

	//Attempt a straight move to goal
	if ( !tryStraight || NPC_ClearPathToGoal( frameNavInfo.direction, NPCInfo->goalEntity ) == qfalse )
	{//blocked
		//Can't get straight to goal, use macro nav
		if ( NAVNEW_MoveToGoal( NPC, &frameNavInfo ) == WAYPOINT_NONE )
		{
			//Can't reach goal, just face
			vectoangles( frameNavInfo.direction, angles );
			NPCInfo->desiredYaw	= AngleNormalize360( angles[YAW] );		
			VectorCopy( frameNavInfo.direction, out );
			*distance = frameNavInfo.distance;
			return qfalse;
		}
		//else we are on our way
		frameNavInfo.flags |= NIF_MACRO_NAV;
	}
	else
	{//we have no architectural problems, see if there are ents inthe way and try to go around them
		//not blocked
		if ( d_altRoutes.integer )
		{//try macro nav
			navInfo_t	tempInfo;
			memcpy( &tempInfo, &frameNavInfo, sizeof( tempInfo ) );
			if ( NAVNEW_AvoidCollision( NPC, NPCInfo->goalEntity, &tempInfo, qtrue, 5 ) == qfalse )
			{//revert to macro nav
				//Can't get straight to goal, dump tempInfo and use macro nav
				if ( NAVNEW_MoveToGoal( NPC, &frameNavInfo ) == WAYPOINT_NONE )
				{
					//Can't reach goal, just face
					vectoangles( frameNavInfo.direction, angles );
					NPCInfo->desiredYaw	= AngleNormalize360( angles[YAW] );		
					VectorCopy( frameNavInfo.direction, out );
					*distance = frameNavInfo.distance;
					return qfalse;
				}
				//else we are on our way
				frameNavInfo.flags |= NIF_MACRO_NAV;
			}
			else
			{//otherwise, either clear or can avoid
				memcpy( &frameNavInfo, &tempInfo, sizeof( frameNavInfo ) );
			}
		}
		else
		{//OR: just give up
			if ( NAVNEW_AvoidCollision( NPC, NPCInfo->goalEntity, &frameNavInfo, qtrue, 30 ) == qfalse )
			{//give up
				return qfalse;
			}
		}
	}

	//Setup the return values
	VectorCopy( frameNavInfo.direction, out );
	*distance = frameNavInfo.distance;

	return qtrue;
}

void G_UcmdMoveForDir( gentity_t *self, usercmd_t *cmd, vec3_t dir )
{
	vec3_t	forward, right;
	float	fDot, rDot;

	AngleVectors( self->r.currentAngles, forward, right, NULL );

	dir[2] = 0;
	VectorNormalize( dir );
	//NPCs cheat and store this directly because converting movement into a ucmd loses precision
	VectorCopy( dir, self->client->ps.moveDir );

	fDot = DotProduct( forward, dir ) * 127.0f;
	rDot = DotProduct( right, dir ) * 127.0f;
	//Must clamp this because DotProduct is not guaranteed to return a number within -1 to 1, and that would be bad when we're shoving this into a signed byte
	if ( fDot > 127.0f )
	{
		fDot = 127.0f;
	}
	if ( fDot < -127.0f )
	{
		fDot = -127.0f;
	}
	if ( rDot > 127.0f )
	{
		rDot = 127.0f;
	}
	if ( rDot < -127.0f )
	{
		rDot = -127.0f;
	}
	cmd->forwardmove = floor(fDot);
	cmd->rightmove = floor(rDot);

	/*
	vec3_t	wishvel;
	for ( int i = 0 ; i < 3 ; i++ ) 
	{
		wishvel[i] = forward[i]*cmd->forwardmove + right[i]*cmd->rightmove;
	}
	VectorNormalize( wishvel );
	if ( !VectorCompare( wishvel, dir ) )
	{
		Com_Printf( "PRECISION LOSS: %s != %s\n", vtos(wishvel), vtos(dir) );
	}
	*/
}

/*
-------------------------
NPC_MoveToGoal

  Now assumes goal is goalEntity, was no reason for it to be otherwise
-------------------------
*/
#if	AI_TIMERS
extern int navTime;
#endif//	AI_TIMERS
qboolean NPC_MoveToGoal( qboolean tryStraight ) 
{
	float	distance;
	vec3_t	dir;

	//hack...
	if ( NPC->client->ps.eFlags2 & EF2_FLYING )
	{
		NPC_FlyToGoal();
		return qtrue;	//FIXME?
	}

#if	AI_TIMERS
	int	startTime = GetTime(0);
#endif//	AI_TIMERS
	//If taking full body pain, don't move
	if ( PM_InKnockDown( &NPC->client->ps ) || ( ( NPC->s.legsAnim >= BOTH_PAIN1 ) && ( NPC->s.legsAnim <= BOTH_PAIN18 ) ) )
	{
		return qtrue;
	}

	/*
	if( NPC->s.eFlags & EF_LOCKED_TO_WEAPON )
	{//If in an emplaced gun, never try to navigate!
		return qtrue;
	}
	*/
	//rwwFIXMEFIXME: emplaced support

	//FIXME: if can't get to goal & goal is a target (enemy), try to find a waypoint that has line of sight to target, at least?
	//Get our movement direction
#if 1
	if ( NPC_GetMoveDirectionAltRoute( dir, &distance, tryStraight ) == qfalse )
#else
	if ( NPC_GetMoveDirection( dir, &distance ) == qfalse )
#endif
		return qfalse;

	NPCInfo->distToGoal		= distance;

	//Convert the move to angles
	vectoangles( dir, NPCInfo->lastPathAngles );
	if ( (ucmd.buttons&BUTTON_WALKING) )
	{
		NPC->client->ps.speed = NPCInfo->stats.walkSpeed;
	}
	else
	{
		NPC->client->ps.speed = NPCInfo->stats.runSpeed;
	}

	//FIXME: still getting ping-ponging in certain cases... !!!  Nav/avoidance error?  WTF???!!!
	//If in combat move, then move directly towards our goal
	//if ( NPC_CheckCombatMove() )
	if ( 1 )
	{//keep current facing
		G_UcmdMoveForDir( NPC, &ucmd, dir );

		if ( !NPC_CheckCombatMove() )
		{
			NPCInfo->desiredPitch	= 0.0f;
			NPCInfo->desiredYaw		= AngleNormalize360( NPCInfo->lastPathAngles[YAW] );
			
			//Pitch towards the goal and also update if flying or swimming
			if ( (NPC->client->ps.eFlags2&EF2_FLYING) )//moveType == MT_FLYSWIM )
			{
				NPCInfo->desiredPitch = AngleNormalize360( NPCInfo->lastPathAngles[PITCH] );
				
				if ( dir[2] )
				{
					float scale = (dir[2] * distance);
					if ( scale > 64 )
					{
						scale = 64;
					}
					else if ( scale < -64 )
					{
						scale = -64;
					}
					NPC->client->ps.velocity[2] = scale;
					//NPC->client->ps.velocity[2] = (dir[2] > 0) ? 64 : -64;
				}
			}
		}
	}
	else
	{//face our goal
		//FIXME: strafe instead of turn if change in dir is small and temporary
		NPCInfo->desiredPitch	= 0.0f;
		NPCInfo->desiredYaw		= AngleNormalize360( NPCInfo->lastPathAngles[YAW] );
		
		//Pitch towards the goal and also update if flying or swimming
		if ( (NPC->client->ps.eFlags2&EF2_FLYING) )//moveType == MT_FLYSWIM )
		{
			NPCInfo->desiredPitch = AngleNormalize360( NPCInfo->lastPathAngles[PITCH] );
			
			if ( dir[2] )
			{
				float scale = (dir[2] * distance);
				if ( scale > 64 )
				{
					scale = 64;
				}
				else if ( scale < -64 )
				{
					scale = -64;
				}
				NPC->client->ps.velocity[2] = scale;
				//NPC->client->ps.velocity[2] = (dir[2] > 0) ? 64 : -64;
			}
		}

		//Set any final info
		ucmd.forwardmove = 127;
	}

#if	AI_TIMERS
	navTime += GetTime( startTime );
#endif//	AI_TIMERS
	return qtrue;
}

/*
-------------------------
void NPC_SlideMoveToGoal( void )

  Now assumes goal is goalEntity, if want to use tempGoal, you set that before calling the func
-------------------------
*/
qboolean NPC_SlideMoveToGoal( void )
{
	float	saveYaw = NPC->client->ps.viewangles[YAW];
	qboolean ret;

	NPCInfo->combatMove = qtrue;
	
	ret = NPC_MoveToGoal( qtrue );

	NPCInfo->desiredYaw	= saveYaw;

	return ret;
}

/*
---------------------
NPC_FlyToGoal
---------------------
*/
#if 0
void NPC_GetFlyToLeader( vec3_t moveDir )
{
	vec3_t	dir, target, t_angs, t_dir;
	float	dist, groundDist = G_GroundDistance( NPC );
	const float	minyaw = 35 /*sina(96/176)*/, maxyaw = 75 /*90-15*/;
	float	speed, maxSpeed;
	const float	speedD = 8;

	VectorSubtract( NPC->client->leader->r.currentOrigin, NPC->r.currentOrigin, dir );
	dist = VectorLength( dir );

	if ( dist > 96 && dist < 256 && groundDist > 96 )
	{
		return;
	}

	GetAnglesForDirection( NPC->client->leader->r.currentOrigin, NPC->r.currentOrigin, t_angs );
	t_angs[PITCH] = flrand( minyaw, maxyaw );	//FIXME?

	AngleVectors( t_angs, t_dir, NULL, NULL );

	VectorMA( NPC->client->leader->r.currentOrigin, 176, t_dir, target );

	VectorSubtract( target, NPC->r.currentOrigin, moveDir );
	dist = VectorLength( moveDir );
	VectorNormalize( moveDir );

	maxSpeed = (dist>384)?NPCInfo->stats.runSpeed:NPCInfo->stats.walkSpeed;

	speed = NPCInfo->flySpeed + NPCInfo->stats.acceleration;
	speed *= sqrt(dist) / speedD;

	if ( speed > maxSpeed )
		speed = maxSpeed;
	else if ( speed < 0 )
		speed = 0;

	NPCInfo->flySpeed = DotProduct( moveDir, NPCInfo->lastFlyDir ) * speed;
	if ( NPCInfo->flySpeed < 0 )
		NPCInfo->flySpeed = 0;

	if ( DotProduct( NPC->client->ps.velocity, dir ) > dist )
		speed = -speed;

	VectorCopy( moveDir, NPCInfo->lastFlyDir );
	VectorScale( moveDir, speed, moveDir );

}
#endif

void NPC_GetFlyToGoal( vec3_t moveDir ) 
{
#if 0
	vec3_t	dir, a, b;
	float	dot, dist;
	float	acceleration = NPCInfo->stats.acceleration, speed, maxSpeed = ((NPCInfo->scriptFlags&SCF_WALKING)?NPCInfo->stats.walkSpeed:NPCInfo->stats.runSpeed);

	if ( !NPCInfo->goalEntity )
		return;

	VectorSubtract( NPCInfo->goalEntity->r.currentOrigin, NPC->r.currentOrigin, dir );

	dist = VectorLengthSquared( dir );
	if ( VectorLengthSquared( dir ) < 4096 )
	{
		VectorClear( moveDir );
		return;
	}

	VectorNormalize( dir );

	acceleration *= 5.0f;
	maxSpeed *= 5.0f;
	dot = DotProduct( NPC->client->ps.velocity, dir );
	speed = maxSpeed - dot;
	if ( speed > acceleration )
		speed = acceleration;
	else if ( speed < -acceleration )
		speed = -acceleration;

	VectorMA( NPC->r.currentOrigin, dist * 2.0f, dir, a );
	VectorMA( NPC->r.currentOrigin, 0.5f, NPC->client->ps.velocity, b );	//FIXME?
	VectorSubtract( a, b, moveDir );
	
	VectorNormalize( moveDir );
	VectorScale( moveDir, acceleration, moveDir );
#else
	vec3_t	vel, dir;
	float	dist;
	float	lMin, lMax;
	const float	lMinD = -(0.015625f*0.015625f), lMaxD = 0.0234375f*0.0234375f;
	/*const */float	speedD = 16;
	float	speed, maxSpeed = ((ucmd.buttons&BUTTON_WALKING)?NPCInfo->stats.walkSpeed:NPCInfo->stats.runSpeed);

	if ( !NPCInfo->goalEntity )
	{
		if ( VectorCompare( NPC->client->ps.velocity, vec3_origin ) )
		{
			VectorClear( moveDir );
		}
		else
		{
			//on d\E9c\E9l\E8re
			speed = VectorLength( NPC->client->ps.velocity );
			if ( speed > maxSpeed )
				speed = maxSpeed;
			VectorNormalize2( NPC->client->ps.velocity, moveDir );
			VectorScale( moveDir, -speed, moveDir );
		}
		return;
	}

	VectorSubtract( NPCInfo->goalEntity->r.currentOrigin, NPC->r.currentOrigin, moveDir );
	VectorNormalize2( moveDir, dir );

	dist = VectorLength( moveDir );

	lMax = sqrt(dist);
	lMin = lMax * lMinD;	//-0.015625f
	lMax *= lMaxD;	//0.0234375f

	if ( VectorLengthSquared( NPC->client->ps.velocity ) <= 1 )
	{
		vel[0] = flrand(-1.0f,1.0f);
		vel[1] = flrand(-1.0f,1.0f);
		vel[2] = flrand(-1.0f,1.0f);
	}
	else
	{
		VectorCopy( NPC->client->ps.velocity, vel );
	}

#if 1	//du gro hack ki pu
	if ( (NPC->r.currentOrigin[2] - NPCInfo->goalEntity->r.currentOrigin[2]) > 0 && NPCInfo->goalEntity == NPC->client->leader )
	{
		lMin = lMax = VectorLength( NPC->client->ps.velocity );
	}
	if ( NPCInfo->behaviorState == BS_FOLLOW_LEADER )
	{
		speedD = 8;
	}
#endif

	VectorNormalize( vel );
	VectorScale( vel, flrand( lMin, lMax ), vel );

	VectorSubtract( moveDir, vel, moveDir );

	VectorNormalize( moveDir );

	speed = NPCInfo->flySpeed + NPCInfo->stats.acceleration;
	speed *= sqrt(dist) / speedD;

	if ( speed > maxSpeed )
		speed = maxSpeed;
	else if ( speed < -maxSpeed )
		speed = -maxSpeed;

	NPCInfo->flySpeed = DotProduct( moveDir, NPCInfo->lastFlyDir ) * speed;
	if ( NPCInfo->flySpeed < 0 )
		NPCInfo->flySpeed = 0;

	if ( DotProduct( NPC->client->ps.velocity, dir ) > dist )
		speed = -speed;

	VectorCopy( moveDir, NPCInfo->lastFlyDir );
	VectorScale( moveDir, speed, moveDir );

#endif
}

void NPC_FlyToGoal( void )
{
	vec3_t	moveDir;

#if 0
	if ( NPCInfo->behaviorState == BS_FOLLOW_LEADER && NPC->client->leader )
	{
		NPC_GetFlyToLeader( moveDir );
	}
	else
#endif
	{
		NPC_GetFlyToGoal( moveDir );
	}

#if 1
#if 1
	if ( NPCInfo->behaviorState == BS_FOLLOW_LEADER && NPC->client->leader )	//hack ki pu
	{
		float dist = DistanceSquared( NPC->client->leader->r.currentOrigin, NPC->r.currentOrigin );
		float groundDist = G_GroundDistance( NPC );

		if ( dist < 9216 /*96*96*/ )
		{
			VectorInverse( moveDir );
		}
		else if ( dist < 25600 /*160*160*/ )
		{
			//on freine
			float speed = VectorLength( NPC->client->ps.velocity ) / 4.0f;
			if ( speed > NPCInfo->stats.acceleration )
				speed = NPCInfo->stats.acceleration;
			VectorNormalize2( NPC->client->ps.velocity, moveDir );
			VectorScale( moveDir, -speed, moveDir );
		}

		if ( groundDist < 64 )
			moveDir[2] += NPCInfo->stats.acceleration / 5.0f;
	}
#endif
	VectorAdd( NPC->client->ps.velocity, moveDir, NPC->client->ps.velocity );
	VectorCopy( moveDir, NPC->client->ps.moveDir );
	ucmd.forwardmove = ucmd.rightmove = ucmd.upmove = 0;
#else
	G_UcmdMoveForDir( NPC, &ucmd, moveDir );
#endif

	NPC_FaceEntity( NPCInfo->goalEntity, qtrue );
}

/*
-------------------------
NPC_ApplyRoff
-------------------------
*/

void NPC_ApplyRoff(void)
{
	BG_PlayerStateToEntityState( &NPC->client->ps, &NPC->s, qfalse );
	//VectorCopy ( NPC->r.currentOrigin, NPC->lastOrigin );
	//rwwFIXMEFIXME: Any significance to this?

	// use the precise origin for linking
	trap_LinkEntity(NPC);
}
