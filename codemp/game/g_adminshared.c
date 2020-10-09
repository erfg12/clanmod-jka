//Originally for the JAE mod, but has none of the original code
//Fully edited to fit the Clan Mod (just same file name basically)

#include "g_local.h"
#include "g_adminshared.h"

void Cmd_NPC_f( gentity_t *ent );
void SP_CreateSnow( gentity_t *ent );
void SP_CreateRain( gentity_t *ent );
void SP_fx_runner( gentity_t *ent );
void SP_jakes_model( gentity_t *ent );
void fx_runner_think( gentity_t *ent );
void AddSpawnField(char *field, char *value);
void M_Svcmd_LockTeam_f(void);
extern void AddIP( char *str );
extern void AddAdminIP( char *str );
extern int G_ClientNumberFromArg( char *str);
extern void Admin_Teleport( gentity_t *ent );
char	*ConcatArgs( int start );
extern void StandardSetBodyAnim(gentity_t *self, int anim, int flags);

void G_RemoveWeather( void ) { //ensiform's whacky weather clearer code
    int i; 
    char s[MAX_STRING_CHARS]; 
    for (i=1 ; i<MAX_FX ; i++) {
        trap_GetConfigstring( CS_EFFECTS + i, s, sizeof( s ) );
        if (!*s || !s[0]) { 
            return;
        }
        if (s[0] == '*')
        { 
            trap_SetConfigstring( CS_EFFECTS + i, ""); 
        }}}


void uwRename(gentity_t *player, const char *newname) 
{ 
    char userinfo[MAX_INFO_STRING]; 
    int clientNum = player-g_entities;
    trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
    Info_SetValueForKey(userinfo, "name", newname);
    trap_SetUserinfo(clientNum, userinfo); 
    ClientUserinfoChanged(clientNum); 
    player->client->pers.netnameTime = level.time + 5000; // hmmm... 
}

void uw2Rename(gentity_t *player, const char *newname) 
{ 
    char userinfo[MAX_INFO_STRING]; 
    int clientNum = player-g_entities;
    trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo)); 
    Info_SetValueForKey(userinfo, "name", newname); 
    trap_SetUserinfo(clientNum, userinfo); 
    ClientUserinfoChanged(clientNum); 
    player->client->pers.netnameTime = level.time + Q3_INFINITE;
}

qboolean ScalePlayer( gentity_t *self, int scale );

//ADMIN BITRATES HAVE ARRIVED!!!!
typedef enum
{
    A_ADMINTELE = 0,
    A_FREEZE,
    A_SILENCE,
    A_PROTECT,
    A_ADMINBAN,
    A_ADMINKICK,
    A_NPC,
    A_INSULTSILENCE,
    A_TERMINATOR,
    A_DEMIGOD,
    A_ADMINSIT, //Administration banning
    A_SCALE,
    A_SPLAT,
    A_SLAY,
    A_GRANTADMIN,
    A_CHANGEMAP,
    A_EMPOWER,
    A_RENAME,
    A_LOCKNAME,
    A_CSPRINT,
    A_FORCETEAM,
    A_CHANGEMODE,
    A_HUMAN,
    A_WEATHER,
    A_PLACE,
    A_PUNISH,
    A_SLEEP,
    A_SLAP,
    A_LOCKTEAM,
    A_MODEL,
    A_WHOIP,
    A_AMVSTR,
    A_STATUS
} admin_type_t;
//RoAR mod END

/*
   ===========
   G_IgnoreClientChat

   Instructs all chat to be ignored by the given 
   ============
 */
void G_IgnoreClientChat ( int ignorer, int ignoree, qboolean ignore )
{
    // Cant ignore yourself
    if ( ignorer == ignoree )
    {
        return;
    }

    // If there is no client connected then dont bother
    if ( g_entities[ignoree].client->pers.connected != CON_CONNECTED )
    {
        return;
    }

    if ( ignore )
    {
        g_entities[ignoree].client->sess.chatIgnoreClients[ignorer/32] |= (1<<(ignorer%32));
    }
    else
    {
        g_entities[ignoree].client->sess.chatIgnoreClients[ignorer/32] &= ~(1<<(ignorer%32));
    }
}

/*
   ===========
   G_IsClientChatIgnored

   Checks to see if the given client is being ignored by a specific client
   ============
 */
qboolean G_IsClientChatIgnored ( int ignorer, int ignoree )
{
    if ( g_entities[ignoree].client->sess.chatIgnoreClients[ignorer/32] & (1<<(ignorer%32)) )
    {
        return qtrue;
    }

    return qfalse;
}
/*
   ===========
   G_RemoveFromAllIgnoreLists

   Clears any possible ignore flags that were set and not reset.
   ============
 */
void G_RemoveFromAllIgnoreLists( int ignorer ) 
{
    int i;

    for( i = 0; i < level.maxclients; i++) {
        g_entities[i].client->sess.chatIgnoreClients[ignorer/32] &= ~(1 << ( ignorer%32 ));
    }
}

qboolean cm_AdminRequirements(int requirement, qboolean gametypes, gentity_t *ent)
{
    if (ent->r.svFlags & SVF_ADMIN)
    {
        if (!(ent->client->pers.bitvalue & (1 << requirement)))
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Not allowed at this administration rank.\n\"") );
            return qfalse;
        }
    }
    if (!(ent->r.svFlags & SVF_ADMIN)){
        trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
        return qfalse;
    }
    if (gametypes == qtrue){
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return qfalse;
        }
    }
    return qtrue;
}

void cm_DefaultWeapon(int target){
    if (g_entities[target].client->ps.duelInProgress == 0){
        if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL))){
            g_entities[target].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
        }
        if (roar_starting_weapons.integer == 0){
            g_entities[target].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );
            g_entities[target].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
            g_entities[target].client->ps.weapon = WP_SABER;
        }
        else if (roar_starting_weapons.integer == 1){
            g_entities[target].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
            g_entities[target].client->ps.weapon = WP_MELEE;
        }
        else if (roar_starting_weapons.integer == 2){
            g_entities[target].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER ) | (1 << WP_MELEE);
            g_entities[target].client->ps.weapon = WP_SABER;
        }
    }
}

void cm_TerminatorTakeaway(int target){
    int ft, f;
    g_entities[target].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );
    g_entities[target].client->ps.weapon = WP_SABER;
    g_entities[target].client->ps.eFlags &= ~EF_SEEKERDRONE;
    g_entities[target].client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
    g_entities[target].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
        & ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
        & ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
    if (g_entities[target].client->pers.amterminator == 1 || g_entities[target].client->pers.amhuman == 1){
        for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
            g_entities[target].client->ps.fd.forcePowerLevel[ft] = g_entities[target].client->pers.forcePowerLevelSaved[ft];
        }
        g_entities[target].client->ps.fd.forcePowersKnown = g_entities[target].client->pers.forcePowersKnownSaved;
    }
    for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
        g_entities[target].client->pers.forcePowerLevelSaved[f] = g_entities[target].client->ps.fd.forcePowerLevel[f];
        g_entities[target].client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
    }
    g_entities[target].client->pers.forcePowersKnownSaved = g_entities[target].client->ps.fd.forcePowersKnown;
    g_entities[target].client->pers.amterminator = 0;
}

void cm_EmpowerTakeaway(int target){
    int f;
    if (g_entities[target].client->pers.amempower){
        g_entities[target].client->ps.persistant[PERS_MONK] = 0;
        g_entities[target].client->pers.amempower = 0;
        g_entities[target].client->ps.eFlags &= ~EF_BODYPUSH;
        for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
            // Save
            g_entities[target].client->ps.fd.forcePowerLevel[f] = g_entities[target].client->pers.forcePowerLevelSaved[f];
        }
        g_entities[target].client->ps.fd.forcePowersKnown = g_entities[target].client->pers.forcePowersKnownSaved;
    }
}

qboolean ClientIDRequirements (int target, gentity_t *ent, qboolean punish){
    if (target == -1){ 
        trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID\n\"") ); 
        return qfalse;
    }
    if (target == -2){ 
        trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID\n\"") ); 
        return qfalse;
    }
    if (target >= MAX_CLIENTS || target < 0){ 
        trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID\n\"") ); 
        return qfalse;
    }
    if (!g_entities[target].inuse){
        trap_SendServerCommand( ent-g_entities, va("print \"Client is not active\n\"") ); 
        return qfalse; 
    }
    if (punish == qtrue){
        if (g_entities[target].client->pers.ampunish){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Unpunish them to empower them.\n\"") ); 
            return qfalse; 
        }
    }
    return qtrue;
}

void AdminCmdSaying(int target, char *cmdname, vmCvar_t saying, char *applyremove, gentity_t *ent, qboolean all){
    if (all == qtrue){
        G_LogPrintf("%s admin command executed by %s on ALL. (%s)\n", cmdname, ent->client->pers.netname, applyremove);
        if (roar_sayings_display.integer == 0){
            trap_SendServerCommand( -1, va("cp \"^7%s\n\"", saying.string ) );
        } else if (roar_sayings_display.integer == 1){
            trap_SendServerCommand( -1, va("print \"^7%s\n\"", saying.string ) );
        } else if (roar_sayings_display.integer >= 2){
            trap_SendServerCommand( -1, va("cp \"^7%s\n\"", saying.string ) );
            trap_SendServerCommand( -1, va("print \"^7%s\n\"", saying.string ) );
        }
    } else {
        G_LogPrintf("%s admin command executed by %s on %s. (%s)\n", cmdname, ent->client->pers.netname, g_entities[target].client->pers.netname, applyremove);
        if (roar_sayings_display.integer == 0){
            trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[target].client->pers.netname, saying.string ) );
        } else if (roar_sayings_display.integer == 1){
            trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[target].client->pers.netname, saying.string ) );
        } else if (roar_sayings_display.integer >= 2){
            trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[target].client->pers.netname, saying.string ) );
            trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[target].client->pers.netname, saying.string ) );
        }
    }
    G_ScreenShake(g_entities[target].client->ps.origin, &g_entities[target],  3.0f, 2000, qtrue); 
    G_Sound(&g_entities[target], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav"));
}

void G_PerformAdminCMD(char *cmd, gentity_t *ent)
{
    if ((Q_stricmp(cmd, "empower") == 0) || (Q_stricmp(cmd, "amempower") == 0))
    {
        int i;
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS]; 
        if (cm_AdminRequirements (A_EMPOWER, qtrue, ent) == qfalse){
            return;
        }
        if ( trap_Argc() > 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp empower if you need help with this command.\n\"" ); 
            return; 
        }
        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        //Apply empower to all unpunished clients
        if( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                if( g_entities[i].client && g_entities[i].client->pers.connected ){
                    if (!g_entities[i].client->pers.amfreeze && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep){
                        cm_TerminatorTakeaway (i);
                        g_entities[i].client->pers.amempower = 1;
                        g_entities[i].client->pers.amhuman = 0;
                    }
                    AdminCmdSaying (i, "Empower", roar_empower_on_ALL_saying, "applying", ent, qtrue );
                }
            }
            return;
        }
        //Take empower away from all empowered clients
        else if( Q_stricmp( arg1, "-all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                if( g_entities[i].client && g_entities[i].client->pers.connected ){
                    cm_EmpowerTakeaway (i);
                    cm_DefaultWeapon (i);
                    AdminCmdSaying (i, "Empower", roar_empower_off_ALL_saying, "removing", ent, qtrue );
                }
            }
            return;
        }
        client_id = G_ClientNumberFromArg( arg1 );
        if ( trap_Argc() < 2 )
        {
            client_id = ent->client->ps.clientNum;
        }
        if (ClientIDRequirements(client_id, ent, qtrue) == qfalse){
            return;
        }
        if (g_entities[client_id].client->pers.amempower == 1){
            cm_EmpowerTakeaway (client_id);
            cm_DefaultWeapon (client_id);
            AdminCmdSaying (client_id, "Empower", roar_empower_off_saying, "removing", ent, qfalse );
        } else {
            cm_TerminatorTakeaway (client_id);
            AdminCmdSaying (client_id, "Empower", cm_empower_on_saying, "applying", ent, qfalse );
            g_entities[client_id].client->pers.amempower = 1;
            g_entities[client_id].client->pers.amhuman = 0;
        }
    }
    else if ( Q_stricmp( cmd, "NPC" ) == 0 )
    {
        if (ent->client->ps.duelInProgress){
            trap_SendServerCommand( ent-g_entities, va("print \"You cannot spawn NPC's while in a duel.\n\"") );
            return;
        }
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_NPC)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"NPC is not allowed at this administration rank.\n\"") );
                return;
            }
            else {
                Cmd_NPC_f( ent );
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
    }
    else if ((Q_stricmp(cmd, "rename") == 0) || (Q_stricmp(cmd, "amrename") == 0)) 
    { 
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        char   arg2[MAX_STRING_CHARS];
        //char   arg3[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_RENAME)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Rename is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 3) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp rename if you need help with this command.\n\"" ); 
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        //trap_Argv( 2,  arg2, sizeof(  arg2 ) );
        client_id = G_ClientNumberFromArg( arg1 );
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        }
        if (client_id >= MAX_CLIENTS || client_id < 0)  
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        trap_Argv( 2, arg2, sizeof( arg2 ) );
        trap_SendServerCommand( -1, va("print \"%s^7%s\n\"", g_entities[client_id].client->pers.netname, roar_rename_saying.string ) );
        G_LogPrintf("Rename (name) admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
        trap_SendServerCommand(client_id, va("cvar name %s", arg2));
        uwRename(&g_entities[client_id], arg2);
    }

    else if (Q_stricmp(cmd, "amkick") == 0)
    { // kick a player 
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS]; 
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINKICK)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"amKick is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp amkick if you need help with this command.\n\"" ); 
            return; 
        }
        trap_Argv( 1,  arg1, sizeof( arg1 ) ); 
        client_id = G_ClientNumberFromArg( arg1 ); 
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if ( client_id == ent->client->ps.clientNum )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"You cant kick yourself.\n\""));
            return;
        }
        if ( g_entities[client_id].client->sess.spectatorState == SPECTATOR_FOLLOW ) {
            g_entities[client_id].client->sess.spectatorState = SPECTATOR_FREE;
        }
        trap_DropClient(client_id, "was Kicked");
        G_LogPrintf("amKick command executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
    }
    else if ((Q_stricmp(cmd, "amban") == 0) || (Q_stricmp(cmd, "ban") == 0))
    { // kick n ban 
        int client_id = -1;
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINBAN)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"amBan is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp amban if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        client_id = G_ClientNumberFromArg(  arg1 ); 
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        }
        if (client_id >= MAX_CLIENTS || client_id < 0)  
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if ( client_id == ent->client->ps.clientNum )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"You cant ban yourself.\n\""));
            return;
        }
        if ( g_entities[client_id].client->sess.spectatorState == SPECTATOR_FOLLOW ) {
            g_entities[client_id].client->sess.spectatorState = SPECTATOR_FREE;
        }
        G_LogPrintf("amBan admin command executed by %s on %s. (IP: %s)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname, g_entities[client_id].client->sess.myip);
        if(!(g_entities[client_id].r.svFlags & SVF_BOT)){
            trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %s", g_entities[client_id].client->sess.myip ) );
        }
        trap_DropClient(client_id, "was Banned");
    }
    else if (Q_stricmp(cmd, "adminkick") == 0)
    {
        int client_id = -1;
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINSIT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AdminKick is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp AdminKick if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        client_id = G_ClientNumberFromArg(  arg1 ); 
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        }
        if (client_id >= MAX_CLIENTS || client_id < 0)  
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if ( client_id == ent->client->ps.clientNum )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"You cant admin kick yourself.\n\""));
            return;
        }
        G_LogPrintf("adminKick admin command executed by %s on %s. (IP: %s)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname, g_entities[client_id].client->sess.myip);
        if (g_entities[client_id].r.svFlags & SVF_ADMIN){ 
            g_entities[client_id].r.svFlags &= ~SVF_ADMIN;
            g_entities[client_id].client->pers.iamanadmin = 0;
            g_entities[client_id].client->pers.bitvalue = 0;
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.logout ));
            trap_SendServerCommand( client_id, va("print \"You have been forcefully logged out of administration by ^7%s\n\"", ent->client->pers.netname ));
        } else {
            trap_SendServerCommand(ent-g_entities, va("print \"%s ^7is not an administrator.\n\"", g_entities[client_id].client->pers.netname));
        }
    }
    else if (Q_stricmp(cmd, "adminban") == 0)
    {
        int client_id = -1;
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINSIT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AdminBan is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp AdminBan if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        client_id = G_ClientNumberFromArg(  arg1 ); 
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        }
        if (client_id >= MAX_CLIENTS || client_id < 0)  
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if ( client_id == ent->client->ps.clientNum )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"You cant admin ban yourself.\n\""));
            return;
        }
        G_LogPrintf("adminBan admin command executed by %s on %s. (IP: %s)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname, g_entities[client_id].client->sess.myip);
        if (g_entities[client_id].r.svFlags & SVF_ADMIN){
            g_entities[client_id].r.svFlags &= ~SVF_ADMIN;
            g_entities[client_id].client->pers.iamanadmin = 0;
            g_entities[client_id].client->sess.adminbanned = 1;
            g_entities[client_id].client->pers.bitvalue = 0;
            trap_SendConsoleCommand( EXEC_APPEND, va( "AddAdminIP %s", g_entities[client_id].client->sess.myip ) );
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.logout ));
            trap_SendServerCommand( client_id, va("print \"You have been banned from administration by ^7%s\n\"", g_entities[client_id].client->pers.netname ));
        } else {
            trap_SendServerCommand(ent-g_entities, va("print \"%s ^7is not an administrator.\n\"", g_entities[client_id].client->pers.netname));
        }
    }
    else if (Q_stricmp(cmd, "removeadminip") == 0) 
    { // unban
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINSIT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AdminBan is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp AdminBan if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        G_LogPrintf("RemoveAdminIP admin command executed by %s. (IP: %s)\n", ent->client->pers.netname, arg1);
        trap_SendConsoleCommand( EXEC_APPEND, va("removeadminip %s\n", arg1));
    }
    else if (Q_stricmp(cmd, "removeip") == 0) 
    { // unban
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINBAN)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"amBan is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp amBan if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        G_LogPrintf("RemoveIP admin command executed by %s. (IP: %s)\n", ent->client->pers.netname, arg1);
        trap_SendConsoleCommand( EXEC_APPEND, va("removeip %s\n", arg1));
    }
    else if (Q_stricmp(cmd, "addadminip") == 0) 
    { // manually add an IP
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINSIT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AdminBan is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp AdminBan if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        G_LogPrintf("AddAdminIP admin command executed by %s. (IP: %s)\n", ent->client->pers.netname, arg1);
        trap_SendConsoleCommand( EXEC_APPEND, va("addadminip %s\n", arg1));
    }
    else if (Q_stricmp(cmd, "addip") == 0) 
    { // manually add an IP
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINBAN)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"amBan is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp amBan if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        G_LogPrintf("AddIP admin command executed by %s. (IP: %s)\n", ent->client->pers.netname, arg1);
        trap_SendConsoleCommand( EXEC_APPEND, va("addip %s\n", arg1));
    }
    /* else if (Q_stricmp(cmd, "possess") == 0) {
       int client_id = -1; 
       char arg1[MAX_STRING_CHARS];
       if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
       g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
       g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
       trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
       return;
       }
       trap_Argv(1, arg1, sizeof(arg1));
       client_id = G_ClientNumberFromArg( arg1 );
       if (trap_Argc() > 3 || trap_Argc() == 1){
       return;
       }
       if (client_id == -1) 
       { 
       trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
       return;
       }
       if (client_id == -2) 
       { 
       trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
       return;
       } 
       if (client_id >= MAX_CLIENTS || client_id < 0) 
       { 
       trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
       return;
       }
    // either we have the client id or the string did not match 
    if (!g_entities[client_id].inuse) 
    { // check to make sure client slot is in use 
    trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
    return; 
    }
    if (g_entities[client_id].client->ps.duelInProgress == 1){
    trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
    return;
    }
    if ((ent->client->ps.pm_flags & PMF_FOLLOW) && ent->client->sess.sessionTeam == TEAM_SPECTATOR){
    ent->client->pers.possesser = client_id;
    g_entities[client_id].client->pers.possessee = qtrue;
    }
    }*/
    else if ((Q_stricmp(cmd, "scale") == 0) || (Q_stricmp(cmd, "amscale") == 0)) {
        int TargetNum = -1;
        int TheScale = 0;
        char arg1[MAX_STRING_CHARS];
        if (g_gametype.integer == GT_TEAM || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, "print \"Scale is not allowed in this gametype.\n\"");
            return;
        }
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_SCALE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Scale is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        trap_Argv(1, arg1, sizeof(arg1)); 
        if(trap_Argc() == 2){
            TargetNum = ent->client->ps.clientNum;
            TheScale = atoi(arg1);
        }
        else {
            TargetNum = G_ClientNumberFromArg( arg1 );
            trap_Argv(2, arg1, sizeof(arg1));
            TheScale = atoi(arg1);
        }
        if (trap_Argc() > 3 || trap_Argc() == 1){
            trap_SendServerCommand( ent-g_entities, "print \"^3Type in ^5/amhelp scale ^3if you need help with this command.\n\""); 
            return;
        }
        if (TargetNum == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (TargetNum == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return;
        } 
        if (TargetNum >= MAX_CLIENTS || TargetNum < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[TargetNum].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[TargetNum].client->ps.duelInProgress == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
            return;
        }
        if(TheScale >= 201 || TheScale <= 49){
            trap_SendServerCommand( ent-g_entities, "print \"Can't scale a player beyond 50 - 200!\n\""); 
            return;
        }
        G_LogPrintf("Scale admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[TargetNum].client->pers.netname);
        trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[TargetNum].client->pers.netname, roar_scale_saying.string ) );
        g_entities[TargetNum].client->pers.ammodelchanged3 = TheScale;
        //g_entities[TargetNum].client->ps.iModelScale = TheScale;
        ScalePlayer(&g_entities[TargetNum], TheScale);
    }
    else if (Q_stricmp (cmd, "myadmincommands") == 0){
        if (!(ent->r.svFlags & SVF_ADMIN)) {
            trap_SendServerCommand( ent-g_entities, va("print \"^1You are not an administrator on this server!\n\"") );
            return;
        }
        //TELEPORT CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_ADMINTELE))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Teleport\n\"") ); }
        //FREEZE CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_FREEZE))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Freeze\n\"") ); }
        //SILENCE CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_SILENCE))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Silence\n\"") ); }
        //PROTECT CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_PROTECT))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Protect\n\"") ); }
        //AMBAN CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_ADMINBAN))) {
            trap_SendServerCommand( ent-g_entities, va("print \"amBan, addip, removeip\n\"") ); }
        //AMKICK CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_ADMINKICK))) {
            trap_SendServerCommand( ent-g_entities, va("print \"amKick\n\"") ); }
        //NPC CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_NPC))) {
            trap_SendServerCommand( ent-g_entities, va("print \"NPC\n\"") ); }
        //INSULTSILENCE CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_INSULTSILENCE))) {
            trap_SendServerCommand( ent-g_entities, va("print \"InsultSilence\n\"") ); }
        //TERMINATOR CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_TERMINATOR))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Terminator\n\"") ); }
        //DEMIGOD CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_DEMIGOD))) {
            trap_SendServerCommand( ent-g_entities, va("print \"DemiGod\n\"") ); }
        //ADMINSIT CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_ADMINSIT))) {
            trap_SendServerCommand( ent-g_entities, va("print \"AdminKick, AdminBan, AddAdminIP, RemoveAdminIP\n\"") ); }
        //SCALE CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_SCALE))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Scale\n\"") ); }
        //SPLAT CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_SPLAT))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Splat\n\"") ); }
        //SLAY CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_SLAY))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Slay\n\"") ); }
        //GRANTADMIN CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_GRANTADMIN))) {
            trap_SendServerCommand( ent-g_entities, va("print \"GrantAdmin\n\"") ); }
        //CHANGEMAP CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_CHANGEMAP))) {
            trap_SendServerCommand( ent-g_entities, va("print \"ChangeMap\n\"") ); }
        //EMPOWER CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_EMPOWER))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Empower\n\"") ); }
        //RENAME CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_RENAME))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Rename\n\"") ); }
        //LOCKNAME CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_LOCKNAME))) {
            trap_SendServerCommand( ent-g_entities, va("print \"LockName\n\"") ); }
        //CSPRINT CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_CSPRINT))) {
            trap_SendServerCommand( ent-g_entities, va("print \"CSPrint\n\"") ); }
        //FORCETEAM CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_FORCETEAM))) {
            trap_SendServerCommand( ent-g_entities, va("print \"ForceTeam\n\"") ); }
        //MONK CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_HUMAN))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Monk\n\"") ); }
        //WEATHER CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_WEATHER))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Weather\n\"") ); }
        //PLACE CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_PLACE))) {
            trap_SendServerCommand( ent-g_entities, va("print \"AddEffect, ClearEffects, AddEffectTemp\n\"") ); }
        //PUNISH CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_PUNISH))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Punish\n\"") ); }
        //SLEEP CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_SLEEP))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Sleep\n\"") ); }
        //SLAP CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_SLAP))) {
            trap_SendServerCommand( ent-g_entities, va("print \"Slap\n\"") ); }
        //LOCKTEAM CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_LOCKTEAM))) {
            trap_SendServerCommand( ent-g_entities, va("print \"LockTeam\n\"") ); }
        //MODEL CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_MODEL))) {
            trap_SendServerCommand( ent-g_entities, va("print \"AddModel, ClearModels, AddModelTemp\n\"") ); }
        //WHOIP CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_WHOIP))) {
            trap_SendServerCommand( ent-g_entities, va("print \"WhoIP\n\"") ); }
        //AMVSTR CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_AMVSTR))) {
            trap_SendServerCommand( ent-g_entities, va("print \"AmVSTR\n\"") ); }
        //AMSTATUS CHECK
        if ((ent->r.svFlags & SVF_ADMIN) && (ent->client->pers.bitvalue & (1 << A_STATUS))) {
            trap_SendServerCommand( ent-g_entities, va("print \"AmStatus\n\"") ); }
    }
    //END
    else if ((Q_stricmp(cmd, "weather") == 0) || (Q_stricmp(cmd, "amweather") == 0) || (Q_stricmp(cmd, "setweather") == 0)){
        char	arg1[MAX_STRING_CHARS];
        char	line[256];
        char	savePath[MAX_QPATH];
        int num;
        vmCvar_t		mapname;
        fileHandle_t	f;
        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_WEATHER)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Weather is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        { 
            trap_SendServerCommand( ent-g_entities, "print \"^3Type in /amhelp weather for more information about this command.\n\"" ); 
            return; 
        }

        trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
        Com_sprintf(savePath, 1024*4, "mp_weather/%s.cfg", mapname.string);
        trap_FS_FOpenFile(savePath, &f, FS_WRITE);
        if (Q_stricmp(arg1, "snow") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*snow");
        }
        else if (Q_stricmp(arg1, "rain") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*rain 500");
        }
        else if (Q_stricmp(arg1, "sandstorm") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*wind");
            G_EffectIndex("*sand");
        }
        else if (Q_stricmp(arg1, "blizzard") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*constantwind (100 100 -100)");
            G_EffectIndex("*fog");
            G_EffectIndex("*snow");
        }
        else if (Q_stricmp(arg1, "fog") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*heavyrainfog");
        }
        else if (Q_stricmp(arg1, "spacedust") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*spacedust 4000");
        }
        else if (Q_stricmp(arg1, "acidrain") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            G_EffectIndex("*acidrain 500");
        }

        if (Q_stricmp(arg1, "clear") == 0){
            G_RemoveWeather();
            num = G_EffectIndex("*clear");
            trap_SetConfigstring( CS_EFFECTS + num, "");
            Com_sprintf( line, sizeof(line), "");
            G_LogPrintf("Weather admin command has been executed by %s. (Weather: Clear)\n", ent->client->pers.netname);
        }
        else {
            Com_sprintf( line, sizeof(line), "weather %s\n", arg1);
            G_LogPrintf("Weather admin command has been executed by %s. (Weather: %s)\n", ent->client->pers.netname, arg1);
        }

        trap_FS_Write( line, strlen(line), f);
        trap_FS_FCloseFile( f );
    }
    else if (Q_stricmp(cmd, "addeffect") == 0){
        char   arg1[MAX_STRING_CHARS];
        gentity_t *fx_runner = G_Spawn();
        char			savePath[MAX_QPATH];
        vmCvar_t		mapname;
        fileHandle_t	f;
        char			line[256];
        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_PLACE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AddEffect is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp addeffect\n\"" ); 
            return; 
        }
        if ((ent->r.svFlags & SVF_ADMIN)){
            AddSpawnField("fxFile", arg1);
#ifdef __linux__
            fx_runner->s.origin[2] = (int) ent->client->ps.origin[2];
#endif
#ifdef QAGAME
            fx_runner->s.origin[2] = (int) ent->client->ps.origin[2] - 15;
#endif
            fx_runner->s.origin[1] = (int) ent->client->ps.origin[1];
            fx_runner->s.origin[0] = (int) ent->client->ps.origin[0];
            SP_fx_runner(fx_runner);

            //cm - Dom
            //Effects are now written to a file sharing the name of the map we are on
            //This file is read at the start of each map load and the effects placed automatically
            trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
            Com_sprintf(savePath, 1024*4, "mp_effects/%s.cfg", mapname.string);

            trap_FS_FOpenFile(savePath, &f, FS_APPEND);

            if ( !f )
            {
                return;
            }

            //File saved in format: EffectName X Y Z
            G_LogPrintf("AddEffect admin command has been executed by %s. (%s) <%i %i %i>\n", ent->client->pers.netname, arg1, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2] - 5);
            Com_sprintf( line, sizeof(line), "addeffect %s %i %i %i\n", arg1, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2] - 5);
            trap_FS_Write( line, strlen(line), f);

            trap_FS_FCloseFile( f );
        }
    }

    else if (Q_stricmp(cmd, "cleareffects") == 0){
        char   arg1[MAX_STRING_CHARS];
        //gentity_t *fx_runner = G_Spawn();
        char			savePath[MAX_QPATH];
        vmCvar_t		mapname;
        fileHandle_t	f;
        char			line[256];
        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_PLACE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"ClearEffects is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }

        if ((ent->r.svFlags & SVF_ADMIN)){
            //cm - Dom
            //Effects are now written to a file sharing the name of the map we are on
            //This file is read at the start of each map load and the effects placed automatically
            trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
            Com_sprintf(savePath, 1024*4, "mp_effects/%s.cfg", mapname.string);

            trap_FS_FOpenFile(savePath, &f, FS_WRITE);

            if ( !f )
            {
                return;
            }

            //File cleared
            Com_sprintf( line, sizeof(line), " ");
            trap_FS_Write( line, strlen(line), f);

            trap_FS_FCloseFile( f );
        }
    }

    else if (Q_stricmp(cmd, "clearmodels") == 0){
        char   arg1[MAX_STRING_CHARS];
        char			savePath[MAX_QPATH];
        vmCvar_t		mapname;
        fileHandle_t	f;
        char			line[256];
        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_MODEL)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"ClearModels is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }

        if ((ent->r.svFlags & SVF_ADMIN)){
            //cm - Dom
            //Effects are now written to a file sharing the name of the map we are on
            //This file is read at the start of each map load and the effects placed automatically
            trap_SendServerCommand( ent-g_entities, va("print \"Models that been successfully cleared from this map.\n\"") );
            trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
            Com_sprintf(savePath, 1024*4, "mp_models/%s.cfg", mapname.string);

            trap_FS_FOpenFile(savePath, &f, FS_WRITE);

            if ( !f )
            {
                return;
            }

            //File cleared
            Com_sprintf( line, sizeof(line), " ");
            trap_FS_Write( line, strlen(line), f);

            trap_FS_FCloseFile( f );
        }
    }
    //cm - Dom
    //Places an effect down temporarily no file writing
    else if (Q_stricmp(cmd, "addeffecttemp") == 0){
        char   arg1[MAX_STRING_CHARS];
        gentity_t *fx_runner = G_Spawn();

        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_PLACE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AddEffect is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp addeffect\n\"" ); 
            return; 
        }
        if ((ent->r.svFlags & SVF_ADMIN)){
            AddSpawnField("fxFile", arg1);	
#ifdef __linux__
            fx_runner->s.origin[2] = (int) ent->client->ps.origin[2];
#endif
#ifdef QAGAME
            fx_runner->s.origin[2] = (int) ent->client->ps.origin[2] - 15;
#endif
            fx_runner->s.origin[1] = (int) ent->client->ps.origin[1];
            fx_runner->s.origin[0] = (int) ent->client->ps.origin[0];
            SP_fx_runner(fx_runner);
        }
    }

    else if (Q_stricmp(cmd, "addmodel") == 0){
        char   arg1[MAX_STRING_CHARS];
        vec3_t location;
        vec3_t angles;
        gentity_t *jakes_model = G_Spawn();
        char			savePath[MAX_QPATH];
        vmCvar_t		mapname;
        fileHandle_t	f;
        char			line[256];
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_MODEL)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AddModel is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp addmodel\n\"" ); 
            return; 
        }
        if ((ent->r.svFlags & SVF_ADMIN)){

            //Spawn model origin
#ifdef __linux__
            jakes_model->s.origin[2] = (int) ent->client->ps.origin[2] - 15;
#else
            jakes_model->s.origin[2] = (int) ent->client->ps.origin[2] - 25;
#endif
            jakes_model->s.origin[1] = (int) ent->client->ps.origin[1];
            jakes_model->s.origin[0] = (int) ent->client->ps.origin[0];
            //Spawn model yaw (view)
            ent->client->ps.viewangles[0] = 0.0f;
            ent->client->ps.viewangles[2] = 0.0f;
            VectorCopy(ent->client->ps.viewangles, angles);
            G_SetAngles(jakes_model, angles);
            //Teleport up cause solid block comes along with model
            location[2] = (int) ent->client->ps.origin[2] + 32; //Teleport up, above the non-walkthroughable box
            location[1] = (int) ent->client->ps.origin[1];
            location[0] = (int) ent->client->ps.origin[0];

            AddSpawnField("model", arg1);
            SP_jakes_model(jakes_model);
            TeleportPlayer(ent, location, ent->client->ps.viewangles);
            trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
            Com_sprintf(savePath, 1024*4, "mp_models/%s.cfg", mapname.string);

            trap_FS_FOpenFile(savePath, &f, FS_APPEND);

            if ( !f )
            {
                return;
            }

            //File saved in format: EffectName X Y Z
            G_LogPrintf("AddModel admin command has been executed by %s. (%s) <%i %i %i %i>\n", ent->client->pers.netname, arg1, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2], (int)ent->client->ps.viewangles[1]);
            Com_sprintf( line, sizeof(line), "addmodel %s %i %i %i %i\n", arg1, (int)ent->client->ps.origin[0], (int)ent->client->ps.origin[1], (int)ent->client->ps.origin[2], (int)ent->client->ps.viewangles[1]);
            trap_FS_Write( line, strlen(line), f);

            trap_FS_FCloseFile( f );
        }
    }

    //cm - Dom
    //Temporary model placement no file writing
    else if (Q_stricmp(cmd, "addmodeltemp") == 0){
        char   arg1[MAX_STRING_CHARS];
        vec3_t location;
        vec3_t angles;
        gentity_t *jakes_model = G_Spawn();
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_MODEL)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AddModel is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp addmodel\n\"" ); 
            return; 
        }
        if ((ent->r.svFlags & SVF_ADMIN)){

            //Spawn model origin
#ifdef __linux__
            jakes_model->s.origin[2] = (int) ent->client->ps.origin[2] - 15;
#else
            jakes_model->s.origin[2] = (int) ent->client->ps.origin[2] - 25;
#endif
            jakes_model->s.origin[1] = (int) ent->client->ps.origin[1];
            jakes_model->s.origin[0] = (int) ent->client->ps.origin[0];
            //Spawn model yaw (view)
            ent->client->ps.viewangles[0] = 0.0f;
            ent->client->ps.viewangles[2] = 0.0f;
            //ent->client->ps.viewangles[ROLL] = 0.0f;
            VectorCopy(ent->client->ps.viewangles, angles);
            G_SetAngles(jakes_model, angles);
            //Teleport up cause solid block comes along with model
            location[2] = (int) ent->client->ps.origin[2] + 32; //Teleport up, above the non-walkthroughable box
            location[1] = (int) ent->client->ps.origin[1];
            location[0] = (int) ent->client->ps.origin[0];

            AddSpawnField("model", arg1);
            SP_jakes_model(jakes_model);
            TeleportPlayer(ent, location, ent->client->ps.viewangles);
        }
    }
    else if ((Q_stricmp(cmd, "slay") == 0) || (Q_stricmp(cmd, "amslay") == 0))
    {
        int client_id = -1;
        char	arg1[MAX_STRING_CHARS];
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        client_id = G_ClientNumberFromArg( arg1 );

        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_SLAY)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Slay is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp slay if you need help with this command.\n\"" ); 
            return; 
        }
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return;
        }
        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
            return;
        }
        //cm - Dom
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->ps.duelInProgress == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
            return;
        }
        if (client_id >= 0 && client_id < MAX_GENTITIES)
        {
            gentity_t *kEnt = &g_entities[client_id];

            if (kEnt->inuse && kEnt->client)
            {
                g_entities[client_id].flags &= ~FL_GODMODE;
                g_entities[client_id].client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
                player_die (kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);

                G_LogPrintf("Slay admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
                if (roar_sayings_display.integer == 0){
                    trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_slay_saying.string ) );
                } else if (roar_sayings_display.integer == 1){
                    trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_slay_saying.string ) );
                } else if (roar_sayings_display.integer >= 2){
                    trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_slay_saying.string ) );
                    trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_slay_saying.string ) );
                }
            }
        }
    }
    else if ((Q_stricmp(cmd, "monk") == 0) || (Q_stricmp(cmd, "ammonk") == 0))
    {
        gentity_t * targetplayer;
        int	i;
        int f;
        int ft;
        int client_id = -1;
        char	arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_HUMAN)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Monk is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return;
        }
        if ( trap_Argc() > 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp monk if you need help with this command.\n\"" ); 
            return; 
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if ( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (!targetplayer->client->pers.amhuman && !targetplayer->client->pers.amfreeze && !targetplayer->client->pers.ampunish
                            && !targetplayer->client->pers.amsleep && !targetplayer->client->pers.amempower && !targetplayer->client->pers.amterminator
                            && !targetplayer->client->pers.amdemigod){
                        targetplayer->client->pers.amhuman = 1;
                        for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
                            targetplayer->client->ps.fd.forcePowerLevel[ft] = targetplayer->client->pers.forcePowerLevelSaved[ft];
                        }
                        targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
                        for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                            // Save
                            targetplayer->client->pers.forcePowerLevelSaved[f] = targetplayer->client->ps.fd.forcePowerLevel[f];
                            // Set new:
                            targetplayer->client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
                        }
                        // Save powers:
                        targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
                    }
                    targetplayer->client->ps.fd.forcePowersKnown &= ~(1 << FP_HEAL) & ~(1 << FP_SPEED) & ~(1 << FP_PUSH) & ~(1 << FP_PULL) 
                        & ~(1 << FP_TELEPATHY) & ~(1 << FP_GRIP) & ~(1 << FP_LIGHTNING) & ~(1 << FP_RAGE) 
                        & ~(1 << FP_PROTECT) & ~(1 << FP_ABSORB) & ~(1 << FP_DRAIN) & ~(1 << FP_SEE);
                    targetplayer->client->ps.fd.forcePowersKnown |= ( 1 << FP_LEVITATION);
                }
            }
            G_LogPrintf("Monk admin command executed by %s on ALL. (applying)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_monk_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_monk_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_monk_on_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_monk_on_ALL_saying.string ) );
            }
            return;
        }
        else if ( Q_stricmp( arg1, "-all" ) == 0 ){
            for ( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (targetplayer->client->pers.amhuman){
                        for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
                            targetplayer->client->ps.fd.forcePowerLevel[ft] = targetplayer->client->pers.forcePowerLevelSaved[ft];
                        }
                        targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
                        targetplayer->client->ps.persistant[PERS_MONK] = 0;
                        if (targetplayer->client->ps.duelInProgress == 0){
                            if (roar_starting_weapons.integer == 0)
                            {
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER);
                                targetplayer->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                                targetplayer->client->ps.weapon = WP_SABER;
                            }
                            else if (roar_starting_weapons.integer == 1)
                            {			
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                                targetplayer->client->ps.weapon = WP_MELEE;
                            }
                            else if (roar_starting_weapons.integer == 2)
                            {
                                targetplayer->client->ps.weapon = WP_SABER;
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER);
                            }
                            targetplayer->client->pers.amhuman = 0;
                            if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL)))
                            {
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
                            }
                        }
                    }
                }
            }
            G_LogPrintf("Monk admin command executed by %s on ALL. (removing)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_monk_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_monk_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_monk_off_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_monk_off_ALL_saying.string ) );
            }
            return;
        }
        client_id = G_ClientNumberFromArg( arg1 );
        if ( trap_Argc() < 2 ) 
        { 
            client_id = ent->client->ps.clientNum;
        }
        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
            return;
        }
        //cm - Dom
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.ampunish)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is currently being punished. Unpunish them to monk them.\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.amhuman)
        {
            if (g_entities[client_id].client->ps.duelInProgress == 0){
                if (roar_starting_weapons.integer == 0)
                {
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER);
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    g_entities[client_id].client->ps.weapon = WP_SABER;
                }
                else if (roar_starting_weapons.integer == 1)
                {			
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                    g_entities[client_id].client->ps.weapon = WP_MELEE;
                }
                else if (roar_starting_weapons.integer == 2)
                {
                    g_entities[client_id].client->ps.weapon = WP_SABER;
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER);
                }
                g_entities[client_id].client->pers.amhuman = 0;
                g_entities[client_id].client->ps.persistant[PERS_MONK] = 0;
                if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL)))
                {
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
                }
            }
            for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
                g_entities[client_id].client->ps.fd.forcePowerLevel[ft] = g_entities[client_id].client->pers.forcePowerLevelSaved[ft];
            }
            g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
            G_LogPrintf("Monk admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_human_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_human_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_human_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_human_off_saying.string ) );
            }
        }
        else {
            //save known force powers again...
            if (g_entities[client_id].client->pers.amempower){
                g_entities[client_id].client->pers.amempower = 0;
                g_entities[client_id].client->ps.eFlags &= ~EF_BODYPUSH;
                for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                    g_entities[client_id].client->ps.fd.forcePowerLevel[f] = g_entities[client_id].client->pers.forcePowerLevelSaved[f];
                }
                g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
            }
            if (g_entities[client_id].client->pers.amterminator){
                g_entities[client_id].client->pers.amterminator = 0;
                for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                    g_entities[client_id].client->ps.fd.forcePowerLevel[f] = g_entities[client_id].client->pers.forcePowerLevelSaved[f];
                }
                g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
            }
            for( f = 0; f < NUM_FORCE_POWERS; f ++ ) {
                g_entities[client_id].client->pers.forcePowerLevelSaved[f] = g_entities[client_id].client->ps.fd.forcePowerLevel[f];
                g_entities[client_id].client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
            }
            g_entities[client_id].client->pers.forcePowersKnownSaved = g_entities[client_id].client->ps.fd.forcePowersKnown;
            g_entities[client_id].client->pers.amhuman = 1;
            g_entities[client_id].client->ps.fd.forcePowersKnown &= ~(1 << FP_HEAL) & ~(1 << FP_SPEED) & ~(1 << FP_PUSH) & ~(1 << FP_PULL) 
                & ~(1 << FP_TELEPATHY) & ~(1 << FP_GRIP) & ~(1 << FP_LIGHTNING) & ~(1 << FP_RAGE) 
                & ~(1 << FP_PROTECT) & ~(1 << FP_ABSORB) & ~(1 << FP_DRAIN) & ~(1 << FP_SEE);
            g_entities[client_id].client->ps.fd.forcePowersKnown |= ( 1 << FP_LEVITATION);
            G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue); 
            G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav")); 
            G_LogPrintf("Monk admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_human_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_human_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_human_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_human_on_saying.string ) );
            }
        }
    } 
    else if (Q_stricmp(cmd, "grantadmin" ) == 0)
    {
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        char   password[MAX_STRING_CHARS];

        if (trap_Argc() != 3) 
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Usage: /grantadmin <client> <password>\n\"") );
            return;
        }

        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_GRANTADMIN)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"GrantAdmin is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }

        trap_Argv( 1,  arg1, sizeof( arg1 ) ); //client
        trap_Argv( 2, password, sizeof( password ) ); // password

        client_id = G_ClientNumberFromArg( arg1 ); 
        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID\n\"") );
            return;
        }
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID\n\"") );
            return;
        }
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID\n\"") );
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client is not active\n\"") );
            return; 
        }

        if ( g_entities[client_id].client->pers.iamanadmin != 0 ) {
            trap_SendServerCommand( ent-g_entities, va("print \"Client is already logged in.\n\"") );
            return; 
        }

        if ( g_entities[client_id].client->sess.adminbanned == 1 ) {
            trap_SendServerCommand( ent-g_entities, va("print \"Client is banned from administration.\n\"") );
            return; 
        }

        if (strcmp(password, cm_adminPassword1.string) == 0){
            g_entities[client_id].client->pers.bitvalue = cm_adminControl1.integer;
            strcpy(g_entities[client_id].client->pers.login, cm_adminlogin1_saying.string);
            strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout1_saying.string);
            g_entities[client_id].client->pers.iamanadmin = 1;
            g_entities[client_id].r.svFlags |= SVF_ADMIN;
            G_LogPrintf("%s was granted admin by %s\n", g_entities[client_id].client->pers.netname, ent->client->pers.netname);
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
        } else if (strcmp(password, cm_adminPassword2.string) == 0){
            g_entities[client_id].client->pers.bitvalue = cm_adminControl2.integer;
            strcpy(g_entities[client_id].client->pers.login, cm_adminlogin2_saying.string);
            strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout2_saying.string);
            g_entities[client_id].client->pers.iamanadmin = 2;
            g_entities[client_id].r.svFlags |= SVF_ADMIN;
            G_LogPrintf("%s was granted admin by %s\n", g_entities[client_id].client->pers.netname, ent->client->pers.netname);
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
        } else if (strcmp(password, cm_adminPassword3.string) == 0){
            g_entities[client_id].client->pers.bitvalue = cm_adminControl3.integer;
            strcpy(g_entities[client_id].client->pers.login, cm_adminlogin3_saying.string);
            strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout3_saying.string);
            g_entities[client_id].client->pers.iamanadmin = 3;
            g_entities[client_id].r.svFlags |= SVF_ADMIN;
            G_LogPrintf("%s was granted admin by %s\n", g_entities[client_id].client->pers.netname, ent->client->pers.netname);
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
        } else if (strcmp(password, cm_adminPassword4.string) == 0){
            g_entities[client_id].client->pers.bitvalue = cm_adminControl4.integer;
            strcpy(g_entities[client_id].client->pers.login, cm_adminlogin4_saying.string);
            strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout4_saying.string);
            g_entities[client_id].client->pers.iamanadmin = 4;
            g_entities[client_id].r.svFlags |= SVF_ADMIN;
            G_LogPrintf("%s was granted admin by %s\n", g_entities[client_id].client->pers.netname, ent->client->pers.netname);
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
        } else if (strcmp(password, cm_adminPassword5.string) == 0){
            g_entities[client_id].client->pers.bitvalue = cm_adminControl5.integer;
            strcpy(g_entities[client_id].client->pers.login, cm_adminlogin5_saying.string);
            strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout5_saying.string);
            g_entities[client_id].client->pers.iamanadmin = 5;
            g_entities[client_id].r.svFlags |= SVF_ADMIN;
            G_LogPrintf("%s was granted admin by %s\n", g_entities[client_id].client->pers.netname, ent->client->pers.netname);
            trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
        } else {
            trap_SendServerCommand( ent-g_entities, va("print \"Incorrect password.\n\"" ));
        }
    }
    else if (Q_stricmp(cmd, "amvstr") == 0)
    {
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_AMVSTR)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"AmVSTR is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp amvstr if you need help with this command.\n\"" ); 
            return; 
        }
        trap_Argv( 1,  arg1, sizeof( arg1 ) );
        G_LogPrintf("AmVSTR (%s) admin command executed by %s", arg1, ent->client->pers.netname);
        trap_SendConsoleCommand( EXEC_INSERT, va( "exec vstr.cfg; wait; wait; wait; vstr %s", arg1 ) );
    }
    else if ((Q_stricmp(cmd, "freeze") == 0) || (Q_stricmp(cmd, "amfreeze") == 0)) 
    { // freeze a player
        gentity_t * targetplayer;
        int i;
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS]; 

        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_FREEZE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Freeze is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp freeze if you need help with this command.\n\"" ); 
            return; 
        }
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        if( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (!targetplayer->client->pers.amfreeze){
                        targetplayer->client->pers.amfreeze = 1;
                    }
                }
            }
            G_LogPrintf("Freeze admin command is executed by %s on ALL. (applying)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_freeze_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_freeze_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_freeze_on_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_freeze_on_ALL_saying.string ) );
            }
            return;
        }
        else if( Q_stricmp( arg1, "-all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (targetplayer->client->pers.amfreeze){
                        targetplayer->client->ps.forceRestricted = qfalse;
                        targetplayer->client->pers.amfreeze = 0;
                    }
                }
            }
            G_LogPrintf("Freeze admin command is executed by %s on ALL. (removing)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_freeze_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_freeze_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_freeze_off_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_freeze_off_ALL_saying.string ) );
            }
            return;
        }
        client_id = G_ClientNumberFromArg(  arg1 );
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }

        if (g_entities[client_id].client->pers.ampunish){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Please unpunish them before freezing them.\nTo unpunish them, type in /punish again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amsleep){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently sleeping. Please wake them before freezing them.\nTo wake them, type in /sleep again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amdemigod == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently a demigod. Please undemigod them before freezing them.\nTo undemigod them, type in /demigod again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->ps.duelInProgress == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
            return;
        }

        if (g_entities[client_id].client->pers.amfreeze)
        {
            g_entities[client_id].client->ps.forceRestricted = qfalse;
            g_entities[client_id].client->pers.amfreeze = 0;
            G_LogPrintf("Freeze admin command is executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_off_saying.string ) );
            }
        }
        else{
            g_entities[client_id].client->pers.amfreeze = 1;
            G_LogPrintf("Freeze admin command is executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_freeze_on_saying.string ) );
            }
            G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue); 
            G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/ambience/thunder_close1"));
        }
    }
    else if ((Q_stricmp(cmd, "lockname") == 0) || (Q_stricmp(cmd, "amlockname") == 0))
    {
        int client_id = -1;
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_LOCKNAME)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"LockName is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp lockname if you need help with this command.\n\"" );
            return;
        }
        if ( trap_Argc() != 2)
        {
            trap_SendServerCommand( ent-g_entities, "print \"Usage: ^3/lockname (client)\n\"" ); 
            return; 
        }
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        client_id = G_ClientNumberFromArg(  arg1 );
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        }
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }// either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        {// check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.amlockname == 1){
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s\nmay change their name now!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( client_id, va("print \"Your name has been unlocked by an admin! You may change it!\n\"" ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s may change their name now!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( client_id, va("print \"Your name has been unlocked by an admin! You may change it!\n\"" ) ); 
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s\nmay change their name now!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( -1, va("print \"%s may change their name now!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( client_id, va("print \"Your name has been unlocked by an admin! You may change it!\n\"" ) ); 
            }
            g_entities[client_id].client->pers.amlockname = 0;
            G_LogPrintf("LockName admin command is executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
        }
        else {
            g_entities[client_id].client->pers.amlockname = 1; 
            G_LogPrintf("LockName admin command is executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s\nis not allowed to change their name!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( client_id, va("print \"Your name has been locked by an admin! You may not change it!\n\"" ) ); 
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s is not allowed to change their name!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( client_id, va("print \"Your name has been locked by an admin! You may not change it!\n\"" ) ); 
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s\nis not allowed to change their name!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( -1, va("print \"%s is not allowed to change their name!\n\"", g_entities[client_id].client->pers.netname ) );
                trap_SendServerCommand( client_id, va("print \"Your name has been locked by an admin! You may not change it!\n\"" ) ); 
            }
        }
    }
    else if (( Q_stricmp( cmd, "forceteam" ) == 0 ) || (Q_stricmp( cmd, "amforceteam" ) == 0)) {
        char arg1[MAX_STRING_CHARS]; 
        char teamname[MAX_STRING_CHARS];
        int clientid;

        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_FORCETEAM)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"ForceTeam is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 3 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp forceteam if you need help with this command.\n\"" ); 
            return; 
        }
        else if ( g_gametype.integer >= GT_TEAM || g_gametype.integer == GT_FFA ) {	
            trap_Argv( 1, arg1, sizeof( arg1 ) );
            clientid = G_ClientNumberFromArg( arg1 );
            if (clientid == -1) 
            { 
                trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
                return; 
            } 
            if (clientid == -2) 
            { 
                trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
                return; 
            }
            if (!g_entities[clientid].inuse) 
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
                return; 
            }
            trap_Argv( 2, teamname, sizeof( teamname ) );
            if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
                SetTeam(&g_entities[clientid], "red" );
                G_LogPrintf("ForceTeam [RED] admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
                if (roar_sayings_display.integer == 0){
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to the ^1Red ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer == 1){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^1Red ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer >= 2){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^1Red ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to the ^1Red ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                }
            }
            else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
                SetTeam(&g_entities[clientid], "blue" );
                G_LogPrintf("ForceTeam [BLUE] admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
                if (roar_sayings_display.integer == 0){
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to the ^4blue ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer == 1){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^4blue ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer >= 2){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^4blue ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to the ^4blue ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                }
            }
            else if ( !Q_stricmp( teamname, "spectate" ) || !Q_stricmp( teamname, "spectator" )  || !Q_stricmp( teamname, "spec" ) || !Q_stricmp( teamname, "s" )) {
                SetTeam(&g_entities[clientid], "spectator" );
                G_LogPrintf("ForceTeam [SPECTATOR] admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
                if (roar_sayings_display.integer == 0){
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to ^3Spectator ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer == 1){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^3Spectator ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer >= 2){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^3Spectator ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to ^3Spectator ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                }
            }
            else if ( !Q_stricmp( teamname, "enter" ) || !Q_stricmp( teamname, "free" ) || !Q_stricmp( teamname, "join" ) || !Q_stricmp( teamname, "j" )
                    || !Q_stricmp( teamname, "f" )) {
                SetTeam(&g_entities[clientid], "free" );
                G_LogPrintf("ForceTeam [JOIN] admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[clientid].client->pers.netname);
                if (roar_sayings_display.integer == 0){
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to the ^2Join ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer == 1){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^2Join ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                } else if (roar_sayings_display.integer >= 2){
                    trap_SendServerCommand( -1, va("print \"%s ^7has been forced to the ^2Join ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                    trap_SendServerCommand( -1, va("cp \"%s ^7has been forced to the ^2Join ^7team.\n\"", g_entities[clientid].client->pers.netname) );
                }
            }
            else {
                trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp forceteam if you need help with this command.\n\"" ); 
            }

            if (clientid == -1) 
            { 
                trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
                return; 
            }
            if (clientid == -2) 
            { 
                trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
                return; 
            }  
            if (!g_entities[clientid].inuse) 
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
                return; 
            }
            /*if (g_gametype.integer >= GT_TEAM && g_entities[clientid].r.svFlags & SVF_BOT){
              trap_SendServerCommand( ent-g_entities, va("print \"Can't force bots to a team.\n\"" ) ); 
              return; 
              }*/
        }
        else
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Wrong gametype! Must be FFA, TEAMFFA, SIEGE, or CTF\n\"") );
            return;
        }
    }
    else if ((Q_stricmp(cmd, "slap") == 0) || (Q_stricmp(cmd, "amslap") == 0)){
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_SLAP)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Slap is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Usage: ^3/slap (client)\n\"" );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        client_id = G_ClientNumberFromArg(  arg1 );
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        if (!g_entities[client_id].inuse) 
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if(g_entities[client_id].client->ps.duelInProgress){
            trap_SendServerCommand( ent-g_entities, va("print \"You cannot slap someone who is currently dueling.\n\"") ); 
            return;
        }
        g_entities[client_id].client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
        g_entities[client_id].client->ps.forceHandExtendTime = level.time + 3000;
        g_entities[client_id].client->ps.velocity[2] += 500;
        g_entities[client_id].client->ps.forceDodgeAnim = 0;
        g_entities[client_id].client->ps.quickerGetup = qfalse;
        G_LogPrintf("Slap admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
        if (roar_sayings_display.integer == 0){
            trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
        } else if (roar_sayings_display.integer == 1){
            trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
        } else if (roar_sayings_display.integer >= 2){
            trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
            trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
        }
    }
    else if ((Q_stricmp(cmd, "sleep") == 0) || (Q_stricmp(cmd, "amsleep") == 0)){
        gentity_t * targetplayer;
        int	i;
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_SLEEP)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Sleep is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Usage: ^3/sleep (client)\n\"" );
            return;
        }
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE ||
                g_gametype.integer == GT_JEDIMASTER){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    targetplayer->client->pers.amsleep = 1;
                    targetplayer->client->pers.tzone = 0;
                }
            }
            G_LogPrintf("Sleep admin command executed by %s on ALL. (applying)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
            }
            return;
        }
        if( Q_stricmp( arg1, "-all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    targetplayer->client->pers.amsleep = 0;
                    targetplayer->takedamage = qtrue;
                    targetplayer->flags &= ~FL_GODMODE;
                    targetplayer->client->ps.forceHandExtendTime = 0;
                }
            }
            G_LogPrintf("Sleep admin command executed by %s on ALL. (removing)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
            }
            return;
        }
        else {
            client_id = G_ClientNumberFromArg(  arg1 );
        }
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.ampunish == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Please unpunish them before sleeping them.\nTo unpunish them, type in /punish again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amfreeze == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently frozen. Please unfreeze them before sleeping them.\nTo unfreeze them, type in /freeze again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amdemigod == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently a demigod. Please undemigod them before sleeping them.\nTo undemigod them, type in /demigod again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amsplat){
            trap_SendServerCommand( ent-g_entities, va("print \"Please wait for client to die first.\n\"", arg1 ) ); 
            return;
        }
        if (g_entities[client_id].client->pers.amsleep == 1)
        {
            G_LogPrintf("Sleep admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_off_saying.string ) );
            }
            g_entities[client_id].client->pers.amsleep = 0;
            if (g_entities[client_id].client->pers.tzone == 0){
                g_entities[client_id].takedamage = qtrue;
            }
            g_entities[client_id].flags &= ~FL_GODMODE;
            g_entities[client_id].client->ps.forceHandExtendTime = 0;
        }
        else
        {
            G_LogPrintf("Sleep admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
            }
            g_entities[client_id].client->pers.amsleep = 1;
            g_entities[client_id].client->pers.tzone = 0;
        }
    }
    else if ((Q_stricmp(cmd, "punish") == 0) || (Q_stricmp(cmd, "ampunish") == 0)){
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_PUNISH)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Punish is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Usage: ^3/punish (client)\n\"" );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        client_id = G_ClientNumberFromArg( arg1 );
        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
            return;
        }
        if (client_id == -2)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
            return;
        }
        // either we have the client id or the string did not match
        if (!g_entities[client_id].inuse)
        { // check to make sure client slot is in use
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
            return;
        }
        if (g_entities[client_id].client->pers.amsleep == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently sleeping. Please unsleep them before punishing them.\nTo wake them, type in /sleep again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amfreeze == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently frozen. Please unfreeze them before punishing them.\nTo unfreeze them, type in /freeze again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amdemigod == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently a demigod. Please undemigod them before punishing them.\nTo undemigod them, type in /demigod again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->ps.duelInProgress == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
            return;
        }
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.ampunish == 1) //Take away punish
        {
            g_entities[client_id].client->ps.weaponTime = 0; //You may use your lightsaber now
            g_entities[client_id].client->pers.ampunish = 0; //Remove persistant flag (for respawn purposes)
            g_entities[client_id].flags &= ~FL_GODMODE; //You are no longer immortal
            G_LogPrintf("Punish admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_punish_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_punish_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_punish_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_punish_off_saying.string ) );
            }
            if (!g_entities[client_id].client->pers.amterminator && !g_entities[client_id].client->pers.amhuman)
            {
                g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
                g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
                    & ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
                    & ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL) & ~(1 << WP_SABER) & ~(1 << WP_MELEE);
                if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL))) //restore weapons
                {
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
                }
                if (roar_starting_weapons.integer == 0)
                {
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER);
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    g_entities[client_id].client->ps.weapon = WP_SABER;
                }
                else if (roar_starting_weapons.integer == 1)
                {			
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                    g_entities[client_id].client->ps.weapon = WP_MELEE;
                }
                else if (roar_starting_weapons.integer == 2)
                {
                    g_entities[client_id].client->ps.weapon = WP_SABER;
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) + (1 << WP_SABER);
                }
            }
            g_entities[client_id].client->ps.eFlags &= ~EF_SEEKERDRONE; //Take away weapons cause u might try to annoy us with sounds/effects.
        }
        else { //Give punish
            g_entities[client_id].client->pers.ampunish = 1;
            G_LogPrintf("Punish admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
            }
        }
    }
    else if ((Q_stricmp(cmd, "silence") == 0) || (Q_stricmp(cmd, "amsilence") == 0))
    { // silence a player
        gentity_t * targetplayer;
        int i;
        int	client_id = -1; 
        char	arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_SILENCE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Silence is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp silence if you need help with this command.\n\"" );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (!targetplayer->client->pers.silent && !targetplayer->client->pers.silent2){
                        targetplayer->client->pers.silent = qtrue;
                    }
                }
            }
            G_LogPrintf("Silence admin command executed by %s on ALL. (applying)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_silence_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_silence_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_silence_on_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_silence_on_ALL_saying.string ) );
            }
            return;
        }
        else if( Q_stricmp( arg1, "-all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (targetplayer->client->pers.silent && !targetplayer->client->pers.silent2){
                        targetplayer->client->pers.silent = qfalse;
                    }
                }
            }
            G_LogPrintf("Silence admin command executed by %s on ALL. (removing)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_silence_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_silence_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_silence_off_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_silence_off_ALL_saying.string ) );
            }
            return;
        }
        else {
            client_id = G_ClientNumberFromArg( arg1 );
        }

        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
            return;
        }
        if (client_id == -2)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
            return;
        }
        // either we have the client id or the string did not match
        if (!g_entities[client_id].inuse)
        { // check to make sure client slot is in use
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
            return;
        }
        if (g_entities[client_id].client->pers.ampunish){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Please unpunish them before silencing them.\nTo unpunish them, type in /punish again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.silent)
        {
            g_entities[client_id].client->pers.silent = qfalse;
            G_LogPrintf("Silence admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_silence_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_silence_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_silence_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_silence_off_saying.string ) );
            }
        }
        else{
            g_entities[client_id].client->pers.silent = qtrue;
            G_LogPrintf("Silence admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_silence_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_silence_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_silence_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_silence_on_saying.string ) );
            }
            G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav")); 
        }
    }
    else if ((Q_stricmp(cmd, "insultsilence") == 0) || (Q_stricmp(cmd, "aminsultsilence") == 0))
    { // silence a player
        int	client_id = -1; 
        char	arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_INSULTSILENCE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"InsultSilence is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp insultsilence if you need help with this command.\n\"" );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        client_id = G_ClientNumberFromArg( arg1 );

        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
            return;
        }
        if (client_id == -2)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
            return;
        }
        // either we have the client id or the string did not match
        if (!g_entities[client_id].inuse)
        { // check to make sure client slot is in use
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
            return;
        }
        if (g_entities[client_id].client->pers.ampunish){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Please unpunish them before insult-silencing them.\nTo unpunish them, type in /punish again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.silent2)
        {
            g_entities[client_id].client->pers.silent2 = qfalse;
            G_LogPrintf("InsultSilence admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_off_saying.string ) );
            }
        }
        else{
            g_entities[client_id].client->pers.silent2 = qtrue;
            G_LogPrintf("InsultSilence admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_insult_silence_on_saying.string ) );
            }
            G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav")); 
        }
    }
    else if ((Q_stricmp(cmd, "teleport") == 0) || (Q_stricmp(cmd, "tele") == 0) || (Q_stricmp(cmd, "admintele") == 0) || (Q_stricmp(cmd, "amtele") == 0))
    { // teleport to specific location
        vec3_t location;
        vec3_t forward;
        vec3_t origin;
        vec3_t yaw;
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_ADMINTELE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Teleport is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if( g_gametype.integer == GT_CTF || g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE )
        {
            G_Printf( "Cannot use teleport during flag or seige game modes.\n" );
            return;
        }
        /*if ( trap_Argc() < 2 ) {
          trap_SendServerCommand( ent-g_entities, va("print \"^3Type in ^5/amhelp teleport ^3if you need help with this command.\n\"") );
          return;
          }*/
        if ( trap_Argc() == 1 ) {
            //cm NOTE: This is where you teleport to a the telemark.
            if (ent->client->pers.amtelemark1 == 0 && ent->client->pers.amtelemark2 == 0 && 
                    ent->client->pers.amtelemark3 == 0 && ent->client->pers.amtelemarkyaw == 0 &&
                    ent->client->pers.amtelemarkset == qfalse) {
                trap_SendServerCommand( ent-g_entities, va("print \"You do not have a telemark set.\nType in /telemark or /amtelemark to establish a telemark.\n\"") );
                return;
            } else {
                origin[0] = ent->client->pers.amtelemark1;
                origin[1] = ent->client->pers.amtelemark2;
                origin[2] = ent->client->pers.amtelemark3;
                yaw[0] = 0.0f;
                yaw[1] = ent->client->pers.amtelemarkyaw;
                yaw[2] = 0.0f;
                TeleportPlayer( ent, origin, yaw );
            }
        }
        //cm - Dom
        //Teleport to player
        if ( trap_Argc() == 2 )
        {
            int	client_id = -1;
            char	arg1[MAX_STRING_CHARS];
            trap_Argv( 1, arg1, sizeof( arg1 ) );
            client_id = G_ClientNumberFromArg( arg1 );

            if (client_id == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
                return;
            }
            if (client_id == -2)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
                return;
            }
            if (client_id >= MAX_CLIENTS || client_id < 0) 
            { 
                G_Printf ( "Bad client ID for %s\n", arg1 ); 
                return;
            }
            // either we have the client id or the string did not match
            if (!g_entities[client_id].inuse)
            { // check to make sure client slot is in use
                trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
                return;
            }
            if (g_entities[client_id].health <= 0)
            {
                return;
            }
            if ( client_id == ent->client->ps.clientNum )
            {
                trap_SendServerCommand( ent-g_entities, va("print \"You cant teleport yourself.\n\""));
                return;
            }
            //Copy their location
            VectorCopy(g_entities[client_id].client->ps.origin, location);
            AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
            // set location out in front of your view
            forward[2] = 0; //no elevation change
            VectorNormalize(forward);
            VectorMA(g_entities[client_id].client->ps.origin, 50, forward, location);
            location[2] += 5; //add just a bit of height???
            //Teleport you to them
            TeleportPlayer(ent, location, g_entities[client_id].client->ps.viewangles);
            G_LogPrintf("Teleport admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            //trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, roar_teleport_saying.string ) );  
        }
        //Teleport player to player
        if ( trap_Argc() == 3 )
        {
            int	client_id = -1;
            int	client_id2 = -1;
            char	arg1[MAX_STRING_CHARS];
            char	arg2[MAX_STRING_CHARS];
            trap_Argv( 1, arg1, sizeof( arg1 ) );
            trap_Argv( 2, arg2, sizeof( arg2 ) );
            client_id = G_ClientNumberFromArg( arg1 );
            client_id2 = G_ClientNumberFromArg( arg2 );

            if (client_id == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
                return;
            }
            if (client_id == -2)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
                return;
            }

            if (client_id2 == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg2 ) );
                return;
            }
            if (client_id2 == -2)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg2 ) );
                return;
            }
            if (client_id >= MAX_CLIENTS || client_id < 0) 
            { 
                G_Printf ( "Bad client ID for %s\n", arg1 ); 
                return;
            }
            if (client_id2 >= MAX_CLIENTS || client_id2 < 0) 
            { 
                G_Printf ( "Bad client ID for %s\n", arg1 ); 
                return;
            }

            // either we have the client id or the string did not match
            if (!g_entities[client_id].inuse)
            { // check to make sure client slot is in use
                trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
                return;
            }
            if (g_entities[client_id].health <= 0)
            {
                return;
            }

            // either we have the client id or the string did not match
            if (!g_entities[client_id2].inuse)
            { // check to make sure client slot is in use
                trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg2 ) );
                return;
            }
            if (g_entities[client_id2].health <= 0)
            {
                return;
            }

            if ( client_id == client_id2 )
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Cant teleport client to same client.\n\""));
                return;
            }

            //Copy client 2 origin
            VectorCopy(g_entities[client_id2].client->ps.origin, location);
            AngleVectors(g_entities[client_id2].client->ps.viewangles, forward, NULL, NULL);
            // set location out in front of your view
            forward[2] = 0; //no elevation change
            VectorNormalize(forward);
            VectorMA(g_entities[client_id2].client->ps.origin, 50, forward, location);
            location[2] += 5; //add just a bit of height???
            //Teleport you to them
            TeleportPlayer(&g_entities[client_id], location, g_entities[client_id2].client->ps.viewangles);
            G_LogPrintf("Teleport admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            //trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_teleport_saying.string ) );
            //trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_teleport_saying.string ) );
        }
        //Using manual coordinates
        if ( trap_Argc() == 4 )
        {
            Admin_Teleport(ent);
        }
        //cm - Dom
        //Teleport player to manual coordinates
        if ( trap_Argc() == 5 )
        {
            int	client_id = -1;			
            char	arg1[MAX_STRING_CHARS];
            vec3_t		origin;
            char		buffer[MAX_TOKEN_CHARS];	

            trap_Argv( 1, arg1, sizeof( arg1 ) );

            client_id = G_ClientNumberFromArg( arg1 );


            if (client_id == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
                return;
            }
            if (client_id == -2)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
                return;
            }
            if (client_id >= MAX_CLIENTS || client_id < 0) 
            { 
                G_Printf ( "Bad client ID for %s\n", arg1 ); 
                return;
            }

            // either we have the client id or the string did not match
            if (!g_entities[client_id].inuse)
            { // check to make sure client slot is in use
                trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
                return;
            }
            if (g_entities[client_id].health <= 0)
            {
                return;
            }

            //Taken from Admin_Teleport() with some mods			
            trap_Argv(2, buffer, sizeof( buffer ) );
            origin[0] = atof(buffer);
            trap_Argv(3, buffer, sizeof( buffer ) );
            origin[1] = atof(buffer);
            trap_Argv(4, buffer, sizeof( buffer ) );
            origin[2] = atof(buffer);			

            TeleportPlayer( &g_entities[client_id], origin, g_entities[client_id].client->ps.viewangles );
            G_LogPrintf("Teleport admin command is executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            //trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_teleport_saying.string ) );  
            //trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_teleport_saying.string ) );  
        }
    }
    else if ((Q_stricmp(cmd, "terminator") == 0) || (Q_stricmp(cmd, "amterminator") == 0) || (Q_stricmp(cmd, "ammerc") == 0))
    {
        gentity_t * targetplayer;
        int	i, f, a, ft;
        int	client_id = -1; 
        char	arg1[MAX_STRING_CHARS]; 
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_TERMINATOR)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Terminator is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return;
        }
        if ( trap_Argc() > 2 )
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp terminator if you need help with this command.\n\"" ); 
            return; 
        }
        trap_Argv( 1,  arg1, sizeof(  arg1 ) ); 
        if( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (!targetplayer->client->pers.amhuman && !targetplayer->client->pers.amfreeze && !targetplayer->client->pers.ampunish
                            && !targetplayer->client->pers.amsleep && !targetplayer->client->pers.amempower && !targetplayer->client->pers.amterminator
                            && !targetplayer->client->pers.amdemigod){
                        // Restore forcepowers:
                        //ITEMS
                        targetplayer->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
                        //WEAPONS
                        targetplayer->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
                        targetplayer->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
                            | (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
                            | (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_TRIP_MINE) | (1 << WP_BRYAR_PISTOL);
                        if (targetplayer->client->pers.amempower == 1 || targetplayer->client->pers.amhuman == 1){
                            targetplayer->client->pers.amempower = 0;
                            targetplayer->client->ps.eFlags &= ~EF_BODYPUSH;
                            for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                                targetplayer->client->ps.fd.forcePowerLevel[f] = targetplayer->client->pers.forcePowerLevelSaved[f];
                            }
                            targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
                        }
                        for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ) {
                            targetplayer->client->pers.forcePowerLevelSaved[ft] = targetplayer->client->ps.fd.forcePowerLevel[ft];
                            targetplayer->client->ps.fd.forcePowerLevel[ft] = FORCE_LEVEL_3;
                        }
                        targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
                        //we must then remove unneeded force powers
                        targetplayer->client->ps.fd.forcePowersKnown &= ~(1 << FP_HEAL) & ~(1 << FP_SPEED) & ~(1 << FP_PUSH) & ~(1 << FP_PULL) 
                            & ~(1 << FP_TELEPATHY) & ~(1 << FP_GRIP) & ~(1 << FP_LIGHTNING) & ~(1 << FP_RAGE) 
                            & ~(1 << FP_PROTECT) & ~(1 << FP_ABSORB) & ~(1 << FP_DRAIN) & ~(1 << FP_SEE);
                        targetplayer->client->ps.fd.forcePowersKnown |= ( 1 << FP_LEVITATION);
                        targetplayer->client->pers.amterminator = 1;
                        targetplayer->client->pers.amhuman = 0;
                        if (targetplayer->client->ps.duelInProgress == 0){
                            targetplayer->client->ps.weapon = WP_MELEE;
                        }
                        if (cm_terminator_infammo.integer == 0){
                            for ( a = 0 ; a < MAX_WEAPONS ; a++ ) {
                                targetplayer->client->ps.ammo[a] = 999;
                            }
                        }
                    }
                }
            }
            G_LogPrintf("Terminator admin command executed by %s on ALL clients. (applying)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_terminator_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_terminator_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_terminator_on_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_terminator_on_ALL_saying.string ) );
            }
            return;
        }
        else if( Q_stricmp( arg1, "-all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (targetplayer->client->pers.amterminator){
                        for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                            targetplayer->client->ps.fd.forcePowerLevel[f] = targetplayer->client->pers.forcePowerLevelSaved[f];
                        }
                        targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
                        targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
                        targetplayer->client->ps.persistant[PERS_MONK] = 0;
                        targetplayer->client->ps.eFlags &= ~EF_SEEKERDRONE;
                        targetplayer->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
                        targetplayer->client->pers.amterminator = 0;
                        targetplayer->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
                            & ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
                            & ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
                        if (targetplayer->client->ps.duelInProgress == 0){
                            if (roar_starting_weapons.integer == 0)
                            {
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER);
                                targetplayer->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                                targetplayer->client->ps.weapon = WP_SABER;
                            }
                            else if (roar_starting_weapons.integer == 1)
                            {			
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                                targetplayer->client->ps.weapon = WP_MELEE;
                            }
                            else if (roar_starting_weapons.integer == 2)
                            {
                                targetplayer->client->ps.weapon = WP_SABER;
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER);
                            }
                            if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL)))
                            {
                                targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
                            }
                        }
                    }
                }
            }
            G_LogPrintf("Terminator admin command executed by %s on ALL clients. (removing)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_terminator_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_terminator_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_terminator_off_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_terminator_off_ALL_saying.string ) );
            }
            return;
        }
        //else {
        client_id = G_ClientNumberFromArg(  arg1 );
        //}
        if ( trap_Argc() < 2 ) 
        { 
            client_id = ent->client->ps.clientNum;
        }
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.ampunish)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is currently being punished. Unpunish them to terminator them.\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.amterminator) //Remove terminator from 1 client
        {
            int	num = 0;
            for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                g_entities[client_id].client->ps.ammo[i] = num;
            }
            g_entities[client_id].client->ps.eFlags &= ~EF_SEEKERDRONE;
            g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
            g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
                & ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
                & ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
            g_entities[client_id].client->ps.persistant[PERS_MONK] = 0;
            g_entities[client_id].client->pers.amterminator = 0;
            if (g_entities[client_id].client->ps.duelInProgress == 0){
                if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL)))
                {
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
                }
                if (roar_starting_weapons.integer == 0)
                {
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER);
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    g_entities[client_id].client->ps.weapon = WP_SABER;
                }
                else if (roar_starting_weapons.integer == 1)
                {			
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                    g_entities[client_id].client->ps.weapon = WP_MELEE;
                }
                else if (roar_starting_weapons.integer == 2)
                {
                    g_entities[client_id].client->ps.weapon = WP_SABER;
                    g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) + (1 << WP_SABER);
                    //g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER);
                }
            }
            //we have to restore the powers we took away to give jump only...
            for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                g_entities[client_id].client->ps.fd.forcePowerLevel[f] = g_entities[client_id].client->pers.forcePowerLevelSaved[f];
            }
            g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
            G_LogPrintf("Terminator admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_off_saying.string ) );
            }
        }
        else {
            //Empowerment takeaway
            // Restore forcepowers:
            //ITEMS
            g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
            //WEAPONS
            g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
            g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
                | (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
                | (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_TRIP_MINE) | (1 << WP_BRYAR_PISTOL);
            if (g_entities[client_id].client->pers.amempower || g_entities[client_id].client->pers.amhuman){
                g_entities[client_id].client->pers.amempower = 0;
                g_entities[client_id].client->ps.eFlags &= ~EF_BODYPUSH;
                for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
                    g_entities[client_id].client->ps.fd.forcePowerLevel[f] = g_entities[client_id].client->pers.forcePowerLevelSaved[f];
                }
                g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
            }
            if (cm_terminator_infammo.integer == 0){
                int	i;
                for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                    g_entities[client_id].client->ps.ammo[i] = 999;
                }
            }
            //save known force powers again...
            for( f = 0; f < NUM_FORCE_POWERS; f ++ ) {
                g_entities[client_id].client->pers.forcePowerLevelSaved[f] = g_entities[client_id].client->ps.fd.forcePowerLevel[f];
                g_entities[client_id].client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
            }
            g_entities[client_id].client->pers.forcePowersKnownSaved = g_entities[client_id].client->ps.fd.forcePowersKnown;
            g_entities[client_id].client->ps.fd.forcePowersKnown &= ~(1 << FP_HEAL) & ~(1 << FP_SPEED) & ~(1 << FP_PUSH) & ~(1 << FP_PULL) 
                & ~(1 << FP_TELEPATHY) & ~(1 << FP_GRIP) & ~(1 << FP_LIGHTNING) & ~(1 << FP_RAGE) 
                & ~(1 << FP_PROTECT) & ~(1 << FP_ABSORB) & ~(1 << FP_DRAIN) & ~(1 << FP_SEE);
            g_entities[client_id].client->ps.fd.forcePowersKnown |= ( 1 << FP_LEVITATION);
            g_entities[client_id].client->pers.amterminator = 1;
            g_entities[client_id].client->pers.amhuman = 0;
            if (g_entities[client_id].client->ps.duelInProgress == 0){
                g_entities[client_id].client->ps.weapon = WP_MELEE;
            }
            G_LogPrintf("Terminator admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_terminator_on_saying.string ) );
            }
            G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue); 
            G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav")); 
        }
    }
    else if ((Q_stricmp(cmd, "CSPrint") == 0) || (Q_stricmp(cmd, "amCSPrint") == 0) || (Q_stricmp(cmd, "ampsay") == 0))
    { 
        int client_id = -1;
        char   arg[MAX_STRING_CHARS];
        //RoAR mod NOTE: Blµb's code begins here		
        int pos = 0;

        char real_msg[MAX_STRING_CHARS];
        char *msg = ConcatArgs(2); 
        while(*msg) { 
            if(msg[0] == '\\' && msg[1] == 'n') { 
                msg++;           // \n is 2 chars, so increase by one here. (one, cuz it's increased down there again...) 
                real_msg[pos++] = '\n';  // put in a real \n 
            } else { 
                real_msg[pos++] = *msg;  // otherwise just copy 
            } 
            msg++;                         // increase the msg pointer 
        }
        real_msg[pos] = 0;
        //RoAR mod NOTE: Blµb's code ends here
        trap_Argv(1, arg, sizeof(arg)); 
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_CSPRINT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"CSPrint is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() < 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp csprint if you need help with this command.\n\"" ); 
            return; 
        }
        if(Q_stricmp(arg, "all") == 0)
        {
            trap_SendServerCommand( -1, va("cp \"%s\"", real_msg) );
            return;
        }

        client_id = G_ClientNumberFromArg( arg );

        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg ) ); 
            return; 
        }
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg ) ); 
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg ) ); 
            return; 
        }

        trap_SendServerCommand(client_id, va("cp \"%s\"", real_msg) );
    }
    else if ((Q_stricmp(cmd, "amclip") == 0) || (Q_stricmp(cmd, "demigod") == 0) || (Q_stricmp(cmd, "amdemigod") == 0))
    {
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS]; 
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_DEMIGOD)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"DemiGod is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() > 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Usage: ^3/demigod (client)\n\"" ); 
            return; 
        } 
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
                g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
                g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
            trap_SendServerCommand( ent-g_entities, va("print \"Cannot use this admin command in this gametype.\n\"" ) );
            return;
        }
        trap_Argv( 1,  arg1, sizeof(  arg1 ) ); 
        client_id = G_ClientNumberFromArg(  arg1 ); 
        if ( trap_Argc() < 2 ) 
        { 
            client_id = ent->client->ps.clientNum;
        }
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.amsleep == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently sleeping. Please unsleep them before demigoding them.\nTo wake them, type in /sleep again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amfreeze == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently frozen. Please unfreeze them before demigoding them.\nTo unfreeze them, type in /freeze again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.ampunish == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Please unpunish them before demigoding them.\nTo unpunish them, type in /punish again on the client.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.amsplat == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being splatted. Please wait for them to die.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->ps.eFlags2 & EF2_HELD_BY_MONSTER){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being eaten. Please wait for them to die.\n\"" ) );
            return;
        }
        if(g_entities[client_id].client->pers.amdemigod)
        {
            G_LogPrintf("DemiGod admin command is executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_off_saying.string ) );
            }
            g_entities[client_id].client->noclip = 0;
            g_entities[client_id].client->pers.amdemigod = 0;
            g_entities[client_id].client->ps.saberCanThrow = qtrue;
            g_entities[client_id].client->ps.forceRestricted = qfalse;
            g_entities[client_id].takedamage = qtrue;

        }
        else
        {
            g_entities[client_id].client->pers.amdemigod = 1;
            G_LogPrintf("DemiGod admin command is executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_demigod_on_saying.string ) );
            }

            G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue); 
            G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav"));
        }
    }
    else if ((Q_stricmp(cmd, "protect") == 0) || (Q_stricmp(cmd, "amprotect") == 0))
    {
        gentity_t * targetplayer;
        int i;
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_PROTECT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Protect is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() > 2 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp protect if you need help with this command.\n\"" );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if( Q_stricmp( arg1, "+all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (!targetplayer->client->pers.tzone){
                        targetplayer->client->pers.tzone = 1;
                        targetplayer->client->pers.autopro = 0;
                    }
                }
            }
            G_LogPrintf("Protect admin command executed by %s on ALL. (applying)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_protect_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_protect_on_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_protect_on_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_protect_on_ALL_saying.string ) );
            }
            return;
        }
        else if( Q_stricmp( arg1, "-all" ) == 0 ){
            for( i = 0; i < level.maxclients; i++ )
            {
                targetplayer = &g_entities[i];

                if( targetplayer->client && targetplayer->client->pers.connected ){
                    if (targetplayer->client->pers.tzone){
                        targetplayer->client->pers.tzone = 0;
                        targetplayer->client->pers.autopro = 0;
                        targetplayer->takedamage = qtrue;
                        targetplayer->client->ps.eFlags &= ~EF_INVULNERABLE;
                    }
                }
            }
            G_LogPrintf("Protect admin command executed by %s on ALL. (removing)\n", ent->client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_protect_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_protect_off_ALL_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_protect_off_ALL_saying.string ) );
                trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_protect_off_ALL_saying.string ) );
            }
            return;
        }
        //else {
        client_id = G_ClientNumberFromArg( arg1 );
        //}
        if ( trap_Argc() < 2 ) 
        { 
            client_id = ent->client->ps.clientNum;
        }
        if (client_id == -1)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) );
            return;
        }
        if (client_id == -2)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) );
            return;
        }
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
            return; 
        }
        if (g_entities[client_id].client->ps.eFlags2 == EF2_HELD_BY_MONSTER)
        {
            return;
        }
        if (g_entities[client_id].client->pers.amsleep == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently sleeping. Wake him to protect him.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.ampunish == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently being punished. Unpunish him to protect him.\n\"" ) );
            return;
        }
        if (g_entities[client_id].health <= 0)
        {
            return;
        }
        // either we have the client id or the string did not match
        if (!g_entities[client_id].inuse)
        { // check to make sure client slot is in use
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) );
            return;
        }
        if (g_entities[client_id].client->ps.duelInProgress == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->pers.tzone == 1)
        {
            g_entities[client_id].client->pers.tzone = 0;
            g_entities[client_id].client->pers.autopro = 0;
            g_entities[client_id].takedamage = qtrue;
            g_entities[client_id].client->ps.eFlags &= ~EF_INVULNERABLE;
            G_LogPrintf("Protect admin command executed by %s on %s. (removing)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_protect_off_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_protect_off_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_protect_off_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_protect_off_saying.string ) );
            } 
        }
        else {
            g_entities[client_id].client->pers.tzone = 1;
            g_entities[client_id].client->pers.autopro = 0;
            G_LogPrintf("Protect admin command executed by %s on %s. (applying)\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_protect_on_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_protect_on_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_protect_on_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_protect_on_saying.string ) );
            }
        }
    }
    else if (( Q_stricmp (cmd, "changemode") == 0 ) || ( Q_stricmp (cmd, "amchangemode") == 0 )){
        char arg1[MAX_TEAMNAME];
        int i;
        gentity_t * targetplayer;
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_CHANGEMODE)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"ChangeMode is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 ){
            trap_SendServerCommand( ent-g_entities, va("print \"Type in /amhelp changemode if you need help with this command.\n\""));
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        if ( !Q_stricmp( arg1, "clanmatch" ) || !Q_stricmp( arg1, "match" ) ) {
            if ((level.modeClanMatch == qfalse) && (level.modeMeeting == qfalse)){
                if ( g_gametype.integer != GT_TEAM ) {
                    trap_SendServerCommand( ent-g_entities, "print \"ClanMatch mode can only be used during Team FFA gametype.\n\"");
                    return;
                }
                level.modeClanMatch = qtrue;
                //cm_mode.integer = 1;
                G_LogPrintf("ChangeMode admin command executed by %s. (START: ClanMatch)\n", ent->client->pers.netname);
                trap_SendServerCommand( -1, va("cp \"^7A ^1clan match^7 has begun!\n\""));
                trap_SendServerCommand( -1, va("print \"^7A ^1clan match^7 has begun!\n\""));
            }
            else {
                trap_SendServerCommand( ent-g_entities, "print \"A mode is already enabled\n\"" );
                return;
            }
        }
        else if ( !Q_stricmp( arg1, "clanmeeting" ) || !Q_stricmp( arg1, "meeting" ) ) {
            if ( g_gametype.integer != GT_FFA ) {
                trap_SendServerCommand( ent-g_entities, "print \"ClanMeeting mode can only be used during FFA gametype.\n\"");
                return;
            }
            if ((level.modeClanMatch == qfalse) && (level.modeMeeting == qfalse)){
                level.modeMeeting = qtrue;
                //cm_mode.integer = 2;
                G_LogPrintf("ChangeMode admin command executed by %s. (START: ClanMeeting)\n", ent->client->pers.netname);
                trap_SendServerCommand( -1, va("cp \"^7A ^3clan meeting^7 has begun!\n\""));
                trap_SendServerCommand( -1, va("print \"^7A ^3clan meeting^7 has begun!\n\""));
            }
            else {
                trap_SendServerCommand( ent-g_entities, "print \"A mode is already enabled\n\"" );
                return;
            }
        }
        else if ( !Q_stricmp( arg1, "clear" )) {
            if (level.modeClanMatch == qtrue){
                G_LogPrintf("ChangeMode admin command executed by %s. (STOP: ClanMatch)\n", ent->client->pers.netname);
                trap_SendServerCommand( ent-g_entities, "print \"Clan Match mode has stopped.\n\"" );
                level.modeClanMatch = qfalse;
                //cm_mode.integer = 0;
            }
            else if (level.modeMeeting == qtrue){
                for ( i = 0; i < level.numConnectedClients; i ++ )
                {
                    targetplayer = &g_entities[i];
                    if( targetplayer->client && targetplayer->client->pers.connected ){
                        g_entities[i].client->ps.userInt3 = 0;
                        if (roar_starting_weapons.integer == 0)
                        {
                            targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );
                            targetplayer->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                            targetplayer->client->ps.weapon = WP_SABER;
                        }
                        else if (roar_starting_weapons.integer == 1)
                        {
                            targetplayer->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                            targetplayer->client->ps.weapon = WP_MELEE;
                        }
                        else if (roar_starting_weapons.integer == 2)
                        {
                            targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER ) | (1 << WP_MELEE);
                            targetplayer->client->ps.weapon = WP_SABER;
                        }
                    }
                }
                G_LogPrintf("ChangeMode admin command executed by %s. (STOP: ClanMeeting)\n", ent->client->pers.netname);
                trap_SendServerCommand( ent-g_entities, "print \"Clan Meeting mode has stopped.\n\"" );
                level.modeMeeting = qfalse;
            }
        }
    }

    else if (( Q_stricmp (cmd, "lockteam") == 0 ) || ( Q_stricmp (cmd, "amlockteam") == 0 )){
        char teamname[MAX_TEAMNAME];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_LOCKTEAM)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"LockTeam is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( g_gametype.integer >= GT_TEAM || g_gametype.integer == GT_FFA ) {

            if ( trap_Argc() != 2 ){
                trap_SendServerCommand( ent-g_entities, va("print \"Usage: /lockteam (team)\n^3TEAMS = Spectator, Blue, Red, Free\n\""));
                return;
            }
            trap_Argv( 1, teamname, sizeof( teamname ) );

            if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
                if (level.isLockedred == qfalse){
                    level.isLockedred = qtrue;
                    G_LogPrintf("LockTeam admin command executed by %s on Red Team. (locking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^1Red ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^1Red ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^1Red ^7team is now ^1Locked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^1Red ^7team is now ^1Locked^7.\n\""));
                    }
                }
                else {
                    level.isLockedred = qfalse;
                    G_LogPrintf("LockTeam admin command executed by %s on Red Team. (unlocking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^1Red ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^1Red ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^1Red ^7team is now ^2unLocked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^1Red ^7team is now ^2unLocked^7.\n\""));
                    }
                }
            }
            else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
                if (level.isLockedblue == qfalse){
                    level.isLockedblue = qtrue;
                    G_LogPrintf("LockTeam admin command executed by %s on Blue Team. (locking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^4Blue ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^4Blue ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^4Blue ^7team is now ^1Locked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^4Blue ^7team is now ^1Locked^7.\n\""));
                    }
                }
                else {
                    level.isLockedblue = qfalse;
                    G_LogPrintf("LockTeam admin command executed by %s on Blue Team. (unlocking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^4Blue ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^4Blue ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^4Blue ^7team is now ^2unLocked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^4Blue ^7team is now ^2unLocked^7.\n\""));
                    }
                }
            }
            else if( !Q_stricmp( teamname, "spectator" ) || !Q_stricmp( teamname, "s" ) || !Q_stricmp( teamname, "spec" ) || !Q_stricmp( teamname, "spectate" ) ) {
                if (level.isLockedspec == qfalse){
                    level.isLockedspec = qtrue;
                    G_LogPrintf("LockTeam admin command executed by %s on Spectator Team. (locking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^3Spectator ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^3Spectator ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^3Spectator ^7team is now ^1Locked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^3Spectator ^7team is now ^1Locked^7.\n\""));
                    }
                }
                else {
                    level.isLockedspec = qfalse;
                    G_LogPrintf("LockTeam admin command executed by %s on Spectator Team. (unlocking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^3Spectator ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^3Spectator ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^3Spectator ^7team is now ^2unLocked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^3Spectator ^7team is now ^2unLocked^7.\n\""));
                    }
                }
            }
            else if( !Q_stricmp( teamname, "join" ) || !Q_stricmp( teamname, "free" ) || !Q_stricmp( teamname, "enter" )
                    || !Q_stricmp( teamname, "f" ) || !Q_stricmp( teamname, "j" )) {
                if (level.isLockedjoin == qfalse){
                    level.isLockedjoin = qtrue;
                    G_LogPrintf("LockTeam admin command executed by %s on Join Team. (locking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^2Join ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^2Join ^7team is now ^1Locked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^2Join ^7team is now ^1Locked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^2Join ^7team is now ^1Locked^7.\n\""));
                    }
                }
                else {
                    level.isLockedjoin = qfalse;
                    G_LogPrintf("LockTeam admin command executed by %s on Join Team. (unlocking)\n", ent->client->pers.netname);
                    if (roar_sayings_display.integer == 0){
                        trap_SendServerCommand( -1, va("cp \"^7The ^2Join ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer == 1){
                        trap_SendServerCommand( -1, va("print \"^7The ^2Join ^7team is now ^2unLocked^7.\n\""));
                    } else if (roar_sayings_display.integer >= 2){
                        trap_SendServerCommand( -1, va("cp \"^7The ^2Join ^7team is now ^2unLocked^7.\n\""));
                        trap_SendServerCommand( -1, va("print \"^7The ^2Join ^7team is now ^2unLocked^7.\n\""));
                    }
                }
            }
            else {
                trap_SendServerCommand( ent-g_entities, va("print \"Usage: /lockteam (team)\n^3TEAMS = Spectator, Blue, Red, Join\n\""));
                return;
            }
        }
        else
        {
            G_Printf("^1Warning^7: You cannot Lock the teams in this gameplay\n");
            return;
        }
    }
    else if ((Q_stricmp(cmd, "changemap") == 0) || (Q_stricmp(cmd, "amchangemap") == 0) || (Q_stricmp(cmd, "ammap") == 0))

    {
        char   arg1[MAX_STRING_CHARS]; 
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_CHANGEMAP)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"ChangeMap is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 3 )
        {
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp changemap if you need help with this command.\n\"" );
            return;
        }
        trap_Argv( 1, arg1, sizeof( arg1 ) );
        trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype %s\n", arg1));
        G_LogPrintf("ChangeMap admin command executed by %s to GAMETYPE:%s", ent->client->pers.netname, arg1);
        trap_Argv( 2, arg1, sizeof( arg1 ) );
        trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", arg1));
        G_LogPrintf(" MAP:%s.\n", arg1);


    }
    else if ((Q_stricmp(cmd, "splat") == 0) || (Q_stricmp(cmd, "amsplat") == 0))
    { // splat a player 
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS];
        if (ent->r.svFlags & SVF_ADMIN)
        {
            if (!(ent->client->pers.bitvalue & (1 << A_SPLAT)))
            {
                trap_SendServerCommand( ent-g_entities, va("print \"Splat is not allowed at this administration rank.\n\"") );
                return;
            }
        }
        if (!(ent->r.svFlags & SVF_ADMIN)){
            trap_SendServerCommand( ent-g_entities, "print \"Must login with /adminlogin (password)\n\"" );
            return;
        }
        if ( trap_Argc() != 2 ) 
        { 
            trap_SendServerCommand( ent-g_entities, "print \"Type in /amhelp splat if you need help with this command.\n\"" ); 
            return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) ); 
        client_id = G_ClientNumberFromArg(  arg1 ); 
        if (client_id == -1) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Can't find client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id == -2) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1 ) ); 
            return; 
        } 
        if (client_id >= MAX_CLIENTS || client_id < 0) 
        { 
            trap_SendServerCommand( ent-g_entities, va("print \"Bad client ID for %s\n\"", arg1 ) ); 
            return;
        }
        // either we have the client id or the string did not match 
        if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
            trap_SendServerCommand( ent-g_entities, va("print \"Client %s is not active\n\"", arg1 ) ); 
            return; 
        }
        if (g_entities[client_id].client->pers.tzone == 1 || g_entities[client_id].client->pers.autopro == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Can't splat a protected client.\n\"", arg1 ) );
            return;
        }
        if (g_entities[client_id].client->pers.amdemigod){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently a demigod. Please un-demigod them.\nTo un-demigod them, type in /demigod on them again.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->ps.duelInProgress == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"Client is currently dueling. Please wait for them to finish.\n\"" ) );
            return;
        }
        if (g_entities[client_id].client->emote_freeze==1)
        {
            g_entities[client_id].client->emote_freeze=0;
        }
        if (g_entities[client_id].health > 0)
        {
            G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue);
            G_LogPrintf("Splat admin command executed by %s on %s.\n", ent->client->pers.netname, g_entities[client_id].client->pers.netname);
            if (roar_sayings_display.integer == 0){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_splat_saying.string ) );
            } else if (roar_sayings_display.integer == 1){
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_splat_saying.string ) );
            } else if (roar_sayings_display.integer >= 2){
                trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_splat_saying.string ) );
                trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_splat_saying.string ) );
            }
            g_entities[client_id].client->ps.velocity[2] += 1000;
            g_entities[client_id].client->pers.amsplat = 1;
            g_entities[client_id].client->ps.forceDodgeAnim = 0;
            g_entities[client_id].client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
            g_entities[client_id].client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
            g_entities[client_id].client->ps.quickerGetup = qfalse;
        }
        else
        {
            return;
        }
    }
    else if ((Q_stricmp(cmd, "whoip") == 0) || (Q_stricmp(cmd, "amwhoip") == 0))
      {
      int client_id = -1;
      char   arg1[MAX_STRING_CHARS];
      if (ent->r.svFlags & SVF_ADMIN)
      {
          if (!(ent->client->pers.bitvalue & (1 << A_WHOIP)))
          {
              trap_SendServerCommand(ent - g_entities, va("print \"WhoIP is not allowed at this administration rank.\n\""));
              return;
          }
      }
      if (!(ent->r.svFlags & SVF_ADMIN)) {
          trap_SendServerCommand(ent - g_entities, "print \"Must login with /adminlogin (password)\n\"");
          return;
      }
      if (trap_Argc() != 2)
      {
          trap_SendServerCommand(ent - g_entities, "print \"Type in /help whoip if you need help with this command.\n\"");
          return;
      }
      trap_Argv(1, arg1, sizeof(arg1));
      client_id = G_ClientNumberFromArg(arg1);
      if (client_id == -1)
      {
          trap_SendServerCommand(ent - g_entities, va("print \"Can't find client ID for %s\n\"", arg1));
          return;
      }
      if (client_id == -2)
      {
          trap_SendServerCommand(ent - g_entities, va("print \"Ambiguous client ID for %s\n\"", arg1));
          return;
      }
      if (client_id >= MAX_CLIENTS || client_id < 0)
      {
          trap_SendServerCommand(ent - g_entities, va("print \"Bad client ID for %s\n\"", arg1));
          return;
      }
      // either we have the client id or the string did not match 
      if (!g_entities[client_id].inuse)
      { // check to make sure client slot is in use 
          trap_SendServerCommand(ent - g_entities, va("print \"Client %s is not active\n\"", arg1));
          return;
      }
      //Admins can see the IP address of each client
      if (g_entities[client_id].r.svFlags & SVF_BOT) {
          trap_SendServerCommand(ent - g_entities, va("print \"%s ^7does not have an IP address.\"", g_entities[client_id].client->pers.netname));
          return;
      }
      trap_SendServerCommand(ent - g_entities, va("print \"%s's^7 IP is %s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->sess.myip));
      }
    else if (Q_stricmp(cmd, "who") == 0) 
    {
        int i;
        trap_SendServerCommand(ent-g_entities, va("print \"\n^1Current clients connected & client status\n\n^3===================================\n\""));
        for(i = 0; i < level.maxclients; i++) { 
            if(g_entities[i].client->pers.connected == CON_CONNECTED) { 
                trap_SendServerCommand(ent-g_entities, va("print \"(%i) %s^7\"", i, g_entities[i].client->pers.netname));
                if (!g_entities[i].client->pers.amfreeze && !g_entities[i].client->pers.iamclan && !g_entities[i].client->pers.iamanadmin && !g_entities[i].client->pers.silent && !g_entities[i].client->pers.silent2 && !g_entities[i].client->pers.amterminator && !g_entities[i].client->pers.amempower && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                    trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                }
                if (g_entities[i].client->pers.amfreeze){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^5(frozen)\""));
                    if (!g_entities[i].client->pers.iamclan && !g_entities[i].client->pers.iamanadmin && !g_entities[i].client->pers.silent && !g_entities[i].client->pers.silent2 && !g_entities[i].client->pers.amterminator && !g_entities[i].client->pers.amempower && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.iamclan){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^1(clan)\""));
                    if (!g_entities[i].client->pers.iamanadmin && !g_entities[i].client->pers.silent && !g_entities[i].client->pers.silent2 && !g_entities[i].client->pers.amterminator && !g_entities[i].client->pers.amempower && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.silent){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^6(silenced)\""));
                    if (!g_entities[i].client->pers.iamanadmin && !g_entities[i].client->pers.silent2 && !g_entities[i].client->pers.amterminator && !g_entities[i].client->pers.amempower && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.silent2){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^6(in-silence)\""));
                    if (!g_entities[i].client->pers.iamanadmin && !g_entities[i].client->pers.amterminator && !g_entities[i].client->pers.amempower && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.amterminator){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^4(terminator)\""));
                    if (!g_entities[i].client->pers.amempower && !g_entities[i].client->pers.iamanadmin && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.amempower){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^4(empowered)\""));
                    if (!g_entities[i].client->pers.iamanadmin && !(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.iamanadmin){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^3(admin)\""));
                    if (!(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.ampunish && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.ampunish){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^1(punished)\""));
                    if (!(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.amsleep && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                if (g_entities[i].client->pers.amsleep){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^5(sleeping)\""));
                    if (!(g_entities[i].r.svFlags & SVF_BOT) && !g_entities[i].client->pers.plugindetect){
                        trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    }
                }
                //Bots n Plugins
                if (g_entities[i].r.svFlags & SVF_BOT){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^7(bot)\""));
                    trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                    //NOTE: End of the road for the bot. Mark it off with \n
                }
                if (g_entities[i].client->pers.plugindetect && !(g_entities[i].r.svFlags & SVF_BOT)){
                    trap_SendServerCommand(ent-g_entities, va("print \" ^2(^3plugin^2)\""));
                    trap_SendServerCommand(ent-g_entities, va("print \"\n\""));
                }
            }
        }
        trap_SendServerCommand(ent-g_entities, va("print \"^3===================================\n\n\""));
    }
    else
    {
        trap_SendServerCommand( ent-g_entities, va("print \"^7Unknown command: %s^7!\n\"", cmd ) );
        return;
    }
}
