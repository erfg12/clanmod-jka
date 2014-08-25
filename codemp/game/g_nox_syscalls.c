#include "g_local.h"

//undo re-routing for calls made from here
#undef trap_Trace
#undef trap_G2Trace
void	trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void	trap_G2Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int g2TraceType, int traceLod );

//lmo code for duel-non-interferance

typedef struct content_save_s {
	int entityNum;
	int contents;
} content_save_t;

content_save_t content_save[MAX_GENTITIES];

void nox_trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	static int i, j; //this gets called a lot, make it static
	static int reset;
	static int done;

	done = 0;
	j = 0; //count number of entities in the save array
	//consider passEntities that are clients
	if ( passEntityNum >= 0 && passEntityNum < MAX_CLIENTS ) // dealing with a client
	{
		if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress ) 
		{ // passEntityNum is dueling
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{ // save contents and change as appropriate
				if ( i == passEntityNum ) continue;
				if ( i == g_entities[passEntityNum].client->ps.duelIndex ) continue; // skip player he is dueling too!
				if ( g_entities[i].inuse && g_entities[i].client ) {
						if ( g_entities[i].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = i;
							content_save[j].contents = g_entities[i].r.contents;
							j++;
							g_entities[i].r.contents = 0;
						}
					if ( g_entities[i].client->ps.saberEntityNum )
					{ // set their lightsaber content too (if they have one
						if ( g_entities[g_entities[i].client->ps.saberEntityNum].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = g_entities[i].client->ps.saberEntityNum;
							content_save[j].contents = g_entities[g_entities[i].client->ps.saberEntityNum].r.contents;
							j++;
							g_entities[g_entities[i].client->ps.saberEntityNum].r.contents = 0;
						}
					}
				}
			}
		}
	}
	else if ( g_entities[passEntityNum].classname && !strcmp(g_entities[passEntityNum].classname, "lightsaber") )
	{// dealing with a lightsaber 
		if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress )
		{	// owner of the lightsaber is in a duel
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{ // save contents and change as appropriate
				if ( i == g_entities[passEntityNum].r.ownerNum ) continue; //skip owner of saber
				if ( i == g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex ) continue; //skip player he is dueling too
				if ( g_entities[i].inuse && g_entities[i].client )
				{
					if ( g_entities[i].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = i;
						content_save[j].contents = g_entities[i].r.contents;
						j++;
						g_entities[i].r.contents = 0;
					}
					if ( g_entities[i].client->ps.saberEntityNum )
					{ // set their lightsaber content too (if they have one
						if ( g_entities[g_entities[i].client->ps.saberEntityNum].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = g_entities[i].client->ps.saberEntityNum;
							content_save[j].contents = g_entities[g_entities[i].client->ps.saberEntityNum].r.contents;
							j++;
							g_entities[g_entities[i].client->ps.saberEntityNum].r.contents = 0;
						}
					}
				}
			}
		}
	}

	trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );

	//consider results
	while ( results->fraction < 1.0 || results->startsolid ) // add start solid test????
	{
		if (results->entityNum >=0 && results->entityNum < MAX_CLIENTS )
		{ // hit entity is a client
			if ( g_entities[results->entityNum].client->ps.duelInProgress )
			{ // hit a client currently dueling
				//who are you? if a client then your duel index should equal the results entitynum
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& g_entities[passEntityNum].client->ps.duelIndex == results->entityNum )
				{ // you have hit the person you are dueling with so,
					done = 1;
				}
				//who are you? if a lightsaber then your owners duel index shout be the results entitynum
				else if ( g_entities[passEntityNum].classname && !strcmp(g_entities[passEntityNum].classname, "lightsaber")
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == results->entityNum )
				{ // your saber has hit the person you are dueling with so,
					done = 1;
				}
				else if ( g_entities[passEntityNum].classname && !strcmp(g_entities[passEntityNum].classname, "laserTrap")
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&&  ( g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == results->entityNum
								|| g_entities[passEntityNum].r.ownerNum == results->entityNum ) )
				{ // your laser trap has hit your opponent or you!
					done = 1;
				}
				else
				{
				//	G_Printf("!!!!!!!!!!!!!!! CLIENT HIT BY %s\n", g_entities[passEntityNum].classname );
					// otherwise
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // change contents of hit entity and run trace again
					}
					if ( g_entities[results->entityNum].client->ps.saberEntityNum )
					{ // set their lightsaber content too (if they have one)
						if ( g_entities[g_entities[results->entityNum].client->ps.saberEntityNum].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = g_entities[results->entityNum].client->ps.saberEntityNum;
							content_save[j].contents = g_entities[g_entities[results->entityNum].client->ps.saberEntityNum].r.contents;
							j++;
							g_entities[g_entities[results->entityNum].client->ps.saberEntityNum].r.contents = 0;
						}
					}
				}
			}
			else
			{
				done = 1; // hit client is not part of a duel
			}
		}
		else if( g_entities[results->entityNum].classname && !strcmp(g_entities[results->entityNum].classname, "lightsaber") )
		{ // hit entity is a lightsaber 
			if ( g_entities[g_entities[results->entityNum].r.ownerNum].client && g_entities[g_entities[results->entityNum].r.ownerNum].client->ps.duelInProgress )
			{ // it is owned by someone in a duel
				//who are you? if a client then your duel index should equal the owner of the results entitynum
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& g_entities[passEntityNum].client->ps.duelIndex == g_entities[results->entityNum].r.ownerNum )
				{ // you have hit the saber of a person you are dueling with so,
					done = 1;
				}
				//who are you? if not a client then your owners duel index should be the owner of the results entitynum
				else if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == g_entities[results->entityNum].r.ownerNum )
				{ // something of yours (eg. saber, missile) has hit the saber of a person you are dueling with so,
					done = 1;
				}
				else
				{
					// otherwise
					if ( g_entities[g_entities[results->entityNum].r.ownerNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh
						content_save[j].entityNum = g_entities[results->entityNum].r.ownerNum;
						content_save[j].contents = g_entities[g_entities[results->entityNum].r.ownerNum].r.contents;
						j++;
						g_entities[g_entities[results->entityNum].r.ownerNum].r.contents = 0; // set saber owner contents to zero
					}
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // set the lightsaber contents to zero, run the trace again
					}
				}
			}
			else
			{
				done = 1; // hit lightsaber is not part of a duel
			}
		}
		else if( g_entities[results->entityNum].classname && !strcmp(g_entities[results->entityNum].classname, "sentryGun") )
		{ // hit entity is a sentryGun
			//NB we use 's' data here rather than 'r' data. First, the r data is not set up for the sentry gun when the
			// entity is created. Second, if we set it up, the behaviour of the trace call is changed. This is because the
			// trace call will check owner info in 'r' and not trace againsts things owned by passEntityNum
			if ( g_entities[g_entities[results->entityNum].s.owner].client
				&& g_entities[g_entities[results->entityNum].s.owner].client->ps.duelInProgress )
			{ // sentry gun is owned by someone in a duel
				//who are you? if a client then consider if the sentry gun is yours or your opponents
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& (g_entities[passEntityNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
						     passEntityNum == g_entities[results->entityNum].s.owner))
				{ // you have hit the sentry gun of a person you are dueling with so,
					done = 1;
				}
				//who are you? if not a client then your owners duel index should be the owner of the results entitynum
				else if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& ( g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
							  g_entities[passEntityNum].r.ownerNum == g_entities[results->entityNum].s.owner))
				{ // something of yours (eg. saber, missile) has hit your sentry gun or the sentry gun of a person you are dueling with so,
					done = 1;
				}
				else
				{
					// otherwise
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
					}
				}
			}
			else if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress )
			{ // and we have to deal with one other case: if *you* are dueling and we did not hit above
				// because the owner of the sentry gun is not dueling, we want to skip this entitiy
				// NB. when you are dueling we try to set up contents before the trace, but we only change
				// the contents of clients and lightsabers, so it is still possilbe you could hit a sentry gun
				if ( g_entities[results->entityNum].r.contents )
				{ // there are some strange situations where contents can get over-written twice... sigh						
					content_save[j].entityNum = results->entityNum;
					content_save[j].contents = g_entities[results->entityNum].r.contents;
					j++;
					g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
				}
			}
			else
			{
				done = 1; // hit sentry gun is not part of a duel nor are you
			}
		}			
		else if( g_entities[results->entityNum].classname && !strcmp(g_entities[results->entityNum].classname, "item_shield") )
		{ // hit entity is a portable shield
			//NB we use 's' data here rather than 'r' data. First, the r data is not set up for the shield when the
			// entity is created. Second, if we set it up, the behaviour of the trace call is changed. This is because the
			// trace call will check owner info in 'r' and not trace againsts things owned by passEntityNum
			if ( g_entities[g_entities[results->entityNum].s.owner].client
				&& g_entities[g_entities[results->entityNum].s.owner].client->ps.duelInProgress )
			{ // shield is owned by someone in a duel
				//who are you? if a client then consider if the sentry gun is yours or your opponents
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& (g_entities[passEntityNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
						     passEntityNum == g_entities[results->entityNum].s.owner))
				{ // you have hit the shield of a person you are dueling with so,
					done = 1;
				}
				//who are you? if not a client then your owners duel index should be the owner of the results entitynum
				else if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& ( g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
							  g_entities[passEntityNum].r.ownerNum == g_entities[results->entityNum].s.owner))
				{ // something of yours (eg. saber, missile) has hit your shield or the shield of a person you are dueling with so,
					done = 1;
				}
				else
				{
					// otherwise
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
					}
				}
			}
			else if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress )
			{ // and we have to deal with one other case: if *you* are dueling and we did not hit above
				// because the owner of the shield is not dueling, we want to skip this entitiy
				// NB. when you are dueling we try to set up contents before the trace, but we only change
				// the contents of clients and lightsabers, so it is still possilbe you could hit a sheild
				if ( g_entities[results->entityNum].r.contents )
				{ // there are some strange situations where contents can get over-written twice... sigh						
					content_save[j].entityNum = results->entityNum;
					content_save[j].contents = g_entities[results->entityNum].r.contents;
					j++;
					g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
				}
			}
			else
			{
				done = 1; // hit sentry gun is not part of a duel nor are you
			}
		}			
		else
		{
			done = 1; // hit entity is not a client or lightsaber or sentry gun so proceed...
		}
		if ( done )
		{
			break; //while
		}
		trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );
	}

	// now reset all entity contents that have been saved
	for ( i = 0; i < j; i++ )
	{
		g_entities[content_save[i].entityNum].r.contents = content_save[i].contents;
	}
}




void nox_trap_G2Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask,  int g2TraceType, int traceLod ) {
	static int i, j; //this gets called a lot, make it static
	static int reset;
	static int done;

	done = 0;
	j = 0; //count number of entities in the save array
	//consider passEntities that are clients
	if ( passEntityNum >= 0 && passEntityNum < MAX_CLIENTS ) // dealing with a client
	{
		if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress ) 
		{ // passEntityNum is dueling
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{ // save contents and change as appropriate
				if ( i == passEntityNum ) continue;
				if ( i == g_entities[passEntityNum].client->ps.duelIndex ) continue; // skip player he is dueling too!
				if ( g_entities[i].inuse && g_entities[i].client ) {
						if ( g_entities[i].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = i;
							content_save[j].contents = g_entities[i].r.contents;
							j++;
							g_entities[i].r.contents = 0;
						}
					if ( g_entities[i].client->ps.saberEntityNum )
					{ // set their lightsaber content too (if they have one
						if ( g_entities[g_entities[i].client->ps.saberEntityNum].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = g_entities[i].client->ps.saberEntityNum;
							content_save[j].contents = g_entities[g_entities[i].client->ps.saberEntityNum].r.contents;
							j++;
							g_entities[g_entities[i].client->ps.saberEntityNum].r.contents = 0;
						}
					}
				}
			}
		}
	}
	else if ( g_entities[passEntityNum].classname && !strcmp(g_entities[passEntityNum].classname, "lightsaber") )
	{// dealing with a lightsaber 
		if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress )
		{	// owner of the lightsaber is in a duel
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{ // save contents and change as appropriate
				if ( i == g_entities[passEntityNum].r.ownerNum ) continue; //skip owner of saber
				if ( i == g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex ) continue; //skip player he is dueling too
				if ( g_entities[i].inuse && g_entities[i].client )
				{
					if ( g_entities[i].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = i;
						content_save[j].contents = g_entities[i].r.contents;
						j++;
						g_entities[i].r.contents = 0;
					}
					if ( g_entities[i].client->ps.saberEntityNum )
					{ // set their lightsaber content too (if they have one
						if ( g_entities[g_entities[i].client->ps.saberEntityNum].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = g_entities[i].client->ps.saberEntityNum;
							content_save[j].contents = g_entities[g_entities[i].client->ps.saberEntityNum].r.contents;
							j++;
							g_entities[g_entities[i].client->ps.saberEntityNum].r.contents = 0;
						}
					}
				}
			}
		}
	}

	trap_G2Trace( results, start, mins, maxs, end, passEntityNum, contentmask, g2TraceType, traceLod );

	//consider results
	while ( results->fraction < 1.0 || results->startsolid ) // add start solid test????
	{
		if (results->entityNum >=0 && results->entityNum < MAX_CLIENTS )
		{ // hit entity is a client
			if ( g_entities[results->entityNum].client->ps.duelInProgress )
			{ // hit a client currently dueling
				//who are you? if a client then your duel index should equal the results entitynum
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& g_entities[passEntityNum].client->ps.duelIndex == results->entityNum )
				{ // you have hit the person you are dueling with so,
					done = 1;
				}
				//who are you? if a lightsaber then your owners duel index shout be the results entitynum
				else if ( g_entities[passEntityNum].classname && !strcmp(g_entities[passEntityNum].classname, "lightsaber")
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == results->entityNum )
				{ // your saber has hit the person you are dueling with so,
					done = 1;
				}
				else if ( g_entities[passEntityNum].classname && !strcmp(g_entities[passEntityNum].classname, "laserTrap")
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&&  ( g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == results->entityNum
								|| g_entities[passEntityNum].r.ownerNum == results->entityNum ) )
				{ // your laser trap has hit your opponent or you!
					done = 1;
				}
				else
				{
				//	G_Printf("!!!!!!!!!!!!!!! CLIENT HIT BY %s\n", g_entities[passEntityNum].classname );
					// otherwise
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // change contents of hit entity and run trace again
					}
					if ( g_entities[results->entityNum].client->ps.saberEntityNum )
					{ // set their lightsaber content too (if they have one)
						if ( g_entities[g_entities[results->entityNum].client->ps.saberEntityNum].r.contents )
						{ // there are some strange situations where contents can get over-written twice... sigh						
							content_save[j].entityNum = g_entities[results->entityNum].client->ps.saberEntityNum;
							content_save[j].contents = g_entities[g_entities[results->entityNum].client->ps.saberEntityNum].r.contents;
							j++;
							g_entities[g_entities[results->entityNum].client->ps.saberEntityNum].r.contents = 0;
						}
					}
				}
			}
			else
			{
				done = 1; // hit client is not part of a duel
			}
		}
		else if( g_entities[results->entityNum].classname && !strcmp(g_entities[results->entityNum].classname, "lightsaber") )
		{ // hit entity is a lightsaber 
			if ( g_entities[g_entities[results->entityNum].r.ownerNum].client && g_entities[g_entities[results->entityNum].r.ownerNum].client->ps.duelInProgress )
			{ // it is owned by someone in a duel
				//who are you? if a client then your duel index should equal the owner of the results entitynum
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& g_entities[passEntityNum].client->ps.duelIndex == g_entities[results->entityNum].r.ownerNum )
				{ // you have hit the saber of a person you are dueling with so,
					done = 1;
				}
				//who are you? if not a client then your owners duel index should be the owner of the results entitynum
				else if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == g_entities[results->entityNum].r.ownerNum )
				{ // something of yours (eg. saber, missile) has hit the saber of a person you are dueling with so,
					done = 1;
				}
				else
				{
					// otherwise
					if ( g_entities[g_entities[results->entityNum].r.ownerNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh
						content_save[j].entityNum = g_entities[results->entityNum].r.ownerNum;
						content_save[j].contents = g_entities[g_entities[results->entityNum].r.ownerNum].r.contents;
						j++;
						g_entities[g_entities[results->entityNum].r.ownerNum].r.contents = 0; // set saber owner contents to zero
					}
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // set the lightsaber contents to zero, run the trace again
					}
				}
			}
			else
			{
				done = 1; // hit lightsaber is not part of a duel
			}
		}
		else if( g_entities[results->entityNum].classname && !strcmp(g_entities[results->entityNum].classname, "sentryGun") )
		{ // hit entity is a sentryGun
			//NB we use 's' data here rather than 'r' data. First, the r data is not set up for the sentry gun when the
			// entity is created. Second, if we set it up, the behaviour of the trace call is changed. This is because the
			// trace call will check owner info in 'r' and not trace againsts things owned by passEntityNum
			if ( g_entities[g_entities[results->entityNum].s.owner].client
				&& g_entities[g_entities[results->entityNum].s.owner].client->ps.duelInProgress )
			{ // sentry gun is owned by someone in a duel
				//who are you? if a client then consider if the sentry gun is yours or your opponents
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& (g_entities[passEntityNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
						     passEntityNum == g_entities[results->entityNum].s.owner))
				{ // you have hit the sentry gun of a person you are dueling with so,
					done = 1;
				}
				//who are you? if not a client then your owners duel index should be the owner of the results entitynum
				else if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& ( g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
							  g_entities[passEntityNum].r.ownerNum == g_entities[results->entityNum].s.owner))
				{ // something of yours (eg. saber, missile) has hit your sentry gun or the sentry gun of a person you are dueling with so,
					done = 1;
				}
				else
				{
					// otherwise
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
					}
				}
			}
			else if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress )
			{ // and we have to deal with one other case: if *you* are dueling and we did not hit above
				// because the owner of the sentry gun is not dueling, we want to skip this entitiy
				// NB. when you are dueling we try to set up contents before the trace, but we only change
				// the contents of clients and lightsabers, so it is still possilbe you could hit a sentry gun
				if ( g_entities[results->entityNum].r.contents )
				{ // there are some strange situations where contents can get over-written twice... sigh						
					content_save[j].entityNum = results->entityNum;
					content_save[j].contents = g_entities[results->entityNum].r.contents;
					j++;
					g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
				}
			}
			else
			{
				done = 1; // hit sentry gun is not part of a duel nor are you
			}
		}			
		else if( g_entities[results->entityNum].classname && !strcmp(g_entities[results->entityNum].classname, "item_shield") )
		{ // hit entity is a portable shield
			//NB we use 's' data here rather than 'r' data. First, the r data is not set up for the shield when the
			// entity is created. Second, if we set it up, the behaviour of the trace call is changed. This is because the
			// trace call will check owner info in 'r' and not trace againsts things owned by passEntityNum
			if ( g_entities[g_entities[results->entityNum].s.owner].client
				&& g_entities[g_entities[results->entityNum].s.owner].client->ps.duelInProgress )
			{ // shield is owned by someone in a duel
				//who are you? if a client then consider if the sentry gun is yours or your opponents
				if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress 
						&& (g_entities[passEntityNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
						     passEntityNum == g_entities[results->entityNum].s.owner))
				{ // you have hit the shield of a person you are dueling with so,
					done = 1;
				}
				//who are you? if not a client then your owners duel index should be the owner of the results entitynum
				else if ( g_entities[g_entities[passEntityNum].r.ownerNum].client && g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelInProgress
						&& ( g_entities[g_entities[passEntityNum].r.ownerNum].client->ps.duelIndex == g_entities[results->entityNum].s.owner ||
							  g_entities[passEntityNum].r.ownerNum == g_entities[results->entityNum].s.owner))
				{ // something of yours (eg. saber, missile) has hit your shield or the shield of a person you are dueling with so,
					done = 1;
				}
				else
				{
					// otherwise
					if ( g_entities[results->entityNum].r.contents )
					{ // there are some strange situations where contents can get over-written twice... sigh						
						content_save[j].entityNum = results->entityNum;
						content_save[j].contents = g_entities[results->entityNum].r.contents;
						j++;
						g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
					}
				}
			}
			else if ( g_entities[passEntityNum].client && g_entities[passEntityNum].client->ps.duelInProgress )
			{ // and we have to deal with one other case: if *you* are dueling and we did not hit above
				// because the owner of the shield is not dueling, we want to skip this entitiy
				// NB. when you are dueling we try to set up contents before the trace, but we only change
				// the contents of clients and lightsabers, so it is still possilbe you could hit a sheild
				if ( g_entities[results->entityNum].r.contents )
				{ // there are some strange situations where contents can get over-written twice... sigh						
					content_save[j].entityNum = results->entityNum;
					content_save[j].contents = g_entities[results->entityNum].r.contents;
					j++;
					g_entities[results->entityNum].r.contents = 0; // set sentry gun owner contents to zero
				}
			}
			else
			{
				done = 1; // hit sentry gun is not part of a duel nor are you
			}
		}			
		else
		{
			done = 1; // hit entity is not a client or lightsaber or sentry gun so proceed...
		}
		if ( done )
		{
			break; //while
		}
		trap_G2Trace( results, start, mins, maxs, end, passEntityNum, contentmask, g2TraceType, traceLod );
	}

	// now reset all entity contents that have been saved
	for ( i = 0; i < j; i++ )
	{
		g_entities[content_save[i].entityNum].r.contents = content_save[i].contents;
	}
}