// Copyright (C) 1999-2000 Id Software, Inc.
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "g_adminshared.h"
#include "timestamp.h"
#include <stdio.h>

//threads
#ifdef _WIN32
#include "windows.h"
#endif
#ifdef __linux__
#include <pthread.h>
#endif

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

typedef struct ipFilter2_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter2_t;

// VVFIXME - We don't need this at all, but this is the quick way.
#ifdef _XBOX
#define	MAX_IPFILTERS	1
#else
#define	MAX_IPFILTERS	1024
#endif

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static ipFilter2_t	ipAdminFilters[MAX_IPFILTERS];
static int			numIPFilters;
static int			numIPAdminFilters;

void *parse_server_output(char *cmd);
char * replace_str(const char *string, const char *substr, const char *replacement);
char * mysqlGetLeaders(char * stmt);

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

char *replace_one(char *str, char *orig, char *rep) //replaces only the first occurance of orig
{
	static char buffer[4096];
	char *p;

	if (!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
		return str;

	strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
	buffer[p - str] = '\0';

	sprintf(buffer + (p - str), "%s%s", rep, p + strlen(orig));

	return buffer;
}

#ifdef _WIN32
DWORD WINAPI ThreadFunc2(__in LPVOID sntCmd) {
	FILE *fp;
	char buf[2048];
	char * s = "";
	char cmd[2048] = "";
	int * type = 0;

	DWORD dwNumSeconds = (DWORD)sntCmd;

	sprintf(cmd, "%s", dwNumSeconds);

	if (strstr(cmd, "mysqlGetLeaders") != NULL) {
		type = 1;
		strcpy(cmd, replace_str(cmd, "mysqlGetLeaders ", ""));
	}
	else if (strstr(cmd, "mysqlRegisterUser")) {
		type = 2;
		strcpy(cmd, replace_str(cmd, "mysqlRegisterUser ", ""));
	}
	else if (strstr(cmd, "mysqlFindUser")) {
		type = 3;
		strcpy(cmd, replace_str(cmd, "mysqlFindUser ", ""));
	}
	else if (strstr(cmd, "mysqlUserStats")) {
		type = 4;
		strcpy(cmd, replace_str(cmd, "mysqlUserStats ", ""));
	}
	else if (strstr(cmd, "mysqlStats2")) {
		type = 5;
		strcpy(cmd, replace_str(cmd, "mysqlStats2 ", ""));
	}

	if ((fp = _popen(cmd, "r")) == NULL) {
		return "Error opening pipe!\n";
	}

	while (fgets(buf, 1024, fp) != NULL) {
		s = malloc(snprintf(NULL, 0, "%s", buf) + 1);
		sprintf(s, "%s", buf);
	}

	if (type == 1)
		printf("%s\n", mysqlGetLeaders(s));
	if (type == 2) {
		if (strstr(s, "success"))
			printf("User now registered.");
	}
	if (type == 3) {
		int * userID = atoi(s);
		if (userID > 0)
			printf("USER FOUND - ID: %i\n", userID);
		else
			printf("USER NOT FOUND");
	}
	if (type == 4) {
		int * getID = atoi(s);

		if (getID > 0) {
			printf("USER FOUND - ID: %i\n", getID);
			parse_server_output(va("mysqlStats2 curl --data \"key=%s&p=stats&g=jedi_academy&id=%i\" %s", cm_mysql_secret.string, getID, cm_mysql_url.string));
		}
		else
			printf("USER NOT FOUND");
	}
	if (type == 5) {
		printf("%s\n", s);
		printf("kills,deaths,duel_wins,duel_loses,flag_captures,ffa_wins,ffa_loses,tdm_wins,tdm_loses,siege_wins,siege_loses,ctf_wins,ctf_loses\n");
	}

	if (_pclose(fp)) {
		return "Command not found or exited with error status\n";
	}

	return 0;
}
#endif
#ifdef __linux__

void *linuxThread2(void *sntCmd)
{
	FILE *fp;
	char buf[2048];
	char * s = "";
	char cmd[2048] = "";
	int * type = 0;

	sprintf(cmd, "%s", sntCmd);

	if (strstr(cmd, "mysqlGetLeaders") != NULL) {
		type = 1;
		strcpy(cmd, replace_str(cmd, "mysqlGetLeaders ", ""));
	}
	else if (strstr(cmd, "mysqlRegisterUser")) {
		type = 2;
		strcpy(cmd, replace_str(cmd, "mysqlRegisterUser ", ""));
	}
	else if (strstr(cmd, "mysqlFindUser")) {
		type = 3;
		strcpy(cmd, replace_str(cmd, "mysqlFindUser ", ""));
	}
	else if (strstr(cmd, "mysqlUserStats")) {
		type = 4;
		strcpy(cmd, replace_str(cmd, "mysqlUserStats ", ""));
	}
	else if (strstr(cmd, "mysqlStats2")) {
		type = 5;
		strcpy(cmd, replace_str(cmd, "mysqlStats2 ", ""));
	}

	if ((fp = popen(cmd, "r")) == NULL) {
		printf( "Error opening pipe!\n");
	}

	while (fgets(buf, 1024, fp) != NULL) {
		s = malloc(snprintf(NULL, 0, "%s", buf) + 1);
		sprintf(s, "%s", buf);
	}

	if (pclose(fp)) {
		printf ("Command not found or exited with error status\n");
	}

	if (type == 1)
		printf("%s\n", mysqlGetLeaders(s));
	if (type == 2) {
		if (strstr(s, "success"))
			printf("User now registered.\n");
	}
	if (type == 3) {
		int * userID = atoi(s);
		if (userID > 0)
			printf("USER FOUND - ID: %i\n", userID);
		else
			printf("USER NOT FOUND");
	}
	if (type == 4) {
		int * getID = atoi(s);

		if (getID > 0) {
			printf("USER FOUND - ID: %i\n", getID);
			return parse_server_output(va("mysqlStats2 curl --data \"key=%s&p=stats&g=jedi_academy&id=%i\" %s", cm_mysql_secret.string, getID, cm_mysql_url.string));
		}
		else
			printf("USER NOT FOUND");
	}
	if (type == 5) {
		printf("%s\n", s);
		printf("kills,deaths,duel_wins,duel_loses,flag_captures,ffa_wins,ffa_loses,tdm_wins,tdm_loses,siege_wins,siege_loses,ctf_wins,ctf_loses\n");
	}

	return NULL;
}
#endif

void *parse_server_output(char *cmd) {
	char s[2048] = "";

#ifdef _WIN32
	DWORD   threadID;
	HANDLE thread = CreateThread(NULL, 0, ThreadFunc2, (LPVOID)cmd, 0, &threadID);
	//WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread); //detach
	if (thread == NULL) {
		G_Printf("ERROR: Thread Handle is null!");
	}
	if (threadID == NULL) {
		G_Printf("ERROR: Thread ID is null!");
	}
#endif

#ifdef __linux__
	pthread_t tid;
	pthread_create(&tid, NULL, linuxThread2, (void*)cmd);
	//if ((pthread_kill(tid, 0)) == 0)
	//	pthread_join(tid, NULL);
	pthread_detach(&tid);
#endif

	return NULL;
}

static qboolean StringToAdminFilter (char *s, ipFilter2_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	ipstr[MAX_INFO_STRING];
	fileHandle_t	f;

	trap_FS_FOpenFile( "banIP.txt", &f, FS_WRITE );
	if ( !f )
	{
		trap_Printf( va("BAN Error: file cannot be opened for writing: %s\n", "banIP.txt") );
		return;
	}
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( ipstr, sizeof(ipstr), "%i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
		trap_FS_Write( ipstr, strlen(ipstr), f);
	}
	trap_FS_FCloseFile( f );

}

static void UpdateIPAdminBans (void)
{
	byte	b[4];
	int		i;
	char	ipstr[MAX_INFO_STRING];
	fileHandle_t	f;

	trap_FS_FOpenFile( "adminIP.txt", &f, FS_WRITE );
	if ( !f )
	{
		trap_Printf( va("BAN Error: file cannot be opened for writing: %s\n", "adminIP.txt") );
		return;
	}
	for (i = 0 ; i < numIPAdminFilters ; i++)
	{
		if (ipAdminFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipAdminFilters[i].compare;
		Com_sprintf( ipstr, sizeof(ipstr), "%i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
		trap_FS_Write( ipstr, strlen(ipstr), f);
	}
	trap_FS_FCloseFile( f );

}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	byte			m[4];// = {'\0','\0','\0','\0'};
	int				i = 0;
	unsigned int	in;
	char			*p;

	while (i < 4)
	{
		m[i] = 0;
		i++;
	}

	i = 0;
	p = from;
	while (*p && i < 4) {
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned int *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

qboolean G_FilterAdminPacket (char *from)
{
	byte			m[4];// = {'\0','\0','\0','\0'};
	int				i = 0;
	unsigned int	in;
	char			*p;

	while (i < 4)
	{
		m[i] = 0;
		i++;
	}

	i = 0;
	p = from;
	while (*p && i < 4) {
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned int *)m;

	for (i=0 ; i<numIPAdminFilters ; i++)
		if ( (in & ipAdminFilters[i].mask) == ipAdminFilters[i].compare)
			return g_filterAdminBan.integer != 0;

	return g_filterAdminBan.integer == 0;
}

qboolean G_CheckMaxConnections( char *from )
{ // returns qfalse when # of players on this ip is <= sv_maxConnections or dont care to check
	int i=0,n=0,count=1;
	int idnum;
	char from2[22];
	gentity_t *ent;

	if ( sv_maxConnections.integer == 0 ) { // max connections check is disabled //RoAR mod NOTE: Changed.
		return qfalse;
	}

	/*if ( g_dedicated.integer == 1 ) { //RoAR mod NOTE: Hosting a local server?
		return qfalse;
	}*/

	Q_strncpyz(from2, from, sizeof(from2));
	n=0;
	while(++n<strlen(from2))if(from2[n]==':')from2[n]=0;// stip port off of "from"

	/*if ( !Q_stricmp(from2, "localhost" ) ) { // localhost doesnt matter
		return qfalse;
	}*/

	if ( !Q_stricmp(from2, "" ) || from2[0] == 0 ) { // bots dont matter either
		return qfalse;
	}

	for(i =0 ; i < g_maxclients.integer; i++) {
		idnum = level.sortedClients[i];
		ent = g_entities + idnum;
		
		if ( !ent || !ent->client )
			continue;
			
		if ( ent->client->pers.connected == CON_DISCONNECTED )
			continue;

		if ( Q_stricmp(from2, ent->client->sess.myip ) )	// ips are not same so dont count
			continue;

		count++;
	}

	if ( count > sv_maxConnections.integer ) {
		return qtrue;
	}

	return qfalse;
}

/*
=================
AddIP
=================
*/
void AddIP( char *str )
{
	int		i;

	if (!Q_stricmp(str, "0.0.0.0")) {
		G_Printf ("Illegal AddIP 0.0.0.0\n");
		return;
	}

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

void AddAdminIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPAdminFilters ; i++)
		if (ipAdminFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPAdminFilters)
	{
		if (numIPAdminFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPAdminFilters++;
	}
	
	if (!StringToAdminFilter (str, &ipAdminFilters[i]))
		ipAdminFilters[i].compare = 0xffffffffu;

	UpdateIPAdminBans();
}

static void InitialAddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;
}

static void InitialAddAdminIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPAdminFilters ; i++)
		if (ipAdminFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPAdminFilters)
	{
		if (numIPAdminFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPAdminFilters++;
	}
	
	if (!StringToAdminFilter (str, &ipAdminFilters[i]))
		ipAdminFilters[i].compare = 0xffffffffu;
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	int len;
	fileHandle_t	f;
	char *buf;

	numIPFilters = 0;

	len = trap_FS_FOpenFile( "banIP.txt", &f, FS_READ );
	if ( !f )
	{
		trap_Printf( va( "BAN Warning: file cannot be opened for reading: %s\n", "banIP.txt") );
		return;
	}
	if ( !len )
	{ //empty file
		trap_FS_FCloseFile( f );
		return;
	}

	if ( (buf = BG_TempAlloc(len+1)) == 0 )
	{//alloc memory for buffer
		return;
	}
	trap_FS_Read( buf, len, f );
	trap_FS_FCloseFile( f );

	for (t = s = buf; *t; /* */ ) {
		s = strchr(s, '\n');
		if (!s)
			break;
		while (*s == '\n')
			*s++ = 0;
		if (*t)
			InitialAddIP( t );
		t = s;
	}
	BG_TempFree(len+1);
}

void G_ProcessAdminIPBans(void) 
{
	char *s, *t;
	int len;
	fileHandle_t	f;
	char *buf;

	numIPAdminFilters = 0;

	len = trap_FS_FOpenFile( "adminIP.txt", &f, FS_READ );
	if ( !f )
	{
		trap_Printf( va( "BAN Warning: file cannot be opened for reading: %s\n", "adminIP.txt") );
		return;
	}
	if ( !len )
	{ //empty file
		trap_FS_FCloseFile( f );
		return;
	}

	if ( (buf = BG_TempAlloc(len+1)) == 0 )
	{//alloc memory for buffer
		return;
	}
	trap_FS_Read( buf, len, f );
	trap_FS_FCloseFile( f );

	for (t = s = buf; *t; /* */ ) {
		s = strchr(s, '\n');
		if (!s)
			break;
		while (*s == '\n')
			*s++ = 0;
		if (*t)
			InitialAddAdminIP( t );
		t = s;
	}
	BG_TempFree(len+1);
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

void Svcmd_AddAdminIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  addadminip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddAdminIP( str );

}

//RoAR mod BEGIN
extern int G_ClientNumberFromArg( char *str);

void Svcmd_Ban_f (void)
{
	char		str[MAX_TOKEN_CHARS];
	int			client_id=-1;
	
	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  ban <playername>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	client_id = G_ClientNumberFromArg( str );
	if (client_id == -1)
	{
		G_Printf("Can't find client ID for %s\n", str);
		return;
	}
	// either we have the client id or the string did not match
	if (!g_entities[client_id].inuse)
	{ // check to make sure client slot is in use
		G_Printf("Client %s is not active\n", str);
		return;
	}

	G_LogPrintf("amBan admin server command executed on %s. (IP: %s)\n", g_entities[client_id].client->pers.netname, g_entities[client_id].client->sess.myip);
	trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %s", g_entities[client_id].client->sess.myip ) );
	trap_SendConsoleCommand( EXEC_APPEND, va("clientkick %d\n", client_id) );
}
//RoAR mod END

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s\n", str );
}

void Svcmd_RemoveAdminIP_f (void)
{
	ipFilter2_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  removeadminip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToAdminFilter (str, &f))
		return;

	for (i=0 ; i<numIPAdminFilters ; i++) {
		if (ipAdminFilters[i].mask == f.mask	&&
			ipAdminFilters[i].compare == f.compare) {
			ipAdminFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPAdminBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_NPC:
			G_Printf("ET_NPC              ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

extern void AddSpawnField(char *field, char *value);
extern void SP_fx_runner( gentity_t *ent );
void	Svcmd_AddEffect( void ) {
gentity_t *fx_runner = G_Spawn();
char   effect[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];

trap_Argv( 1, effect, sizeof( effect ) );
AddSpawnField("fxFile", effect);
trap_Argv( 2, origin_number, 1024 );
fx_runner->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
fx_runner->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
fx_runner->s.origin[2] = atoi(origin_number3) - 5;
SP_fx_runner(fx_runner);
}

extern void G_RemoveWeather( void );
void Svcmd_Weather( void ) {
	char   arg1[MAX_STRING_CHARS];
	int num;
	trap_Argv( 1,  arg1, sizeof( arg1 ) );
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
			}
}

extern void AddSpawnField(char *field, char *value);
extern void SP_jakes_model( gentity_t *ent );
void	Svcmd_AddModel( void ) {
gentity_t *jakes_model = G_Spawn();
char   model[MAX_STRING_CHARS];
char origin_number[MAX_STRING_CHARS];
char origin_number2[MAX_STRING_CHARS];
char origin_number3[MAX_STRING_CHARS];
char yaw_number[MAX_STRING_CHARS];

if ( trap_Argc() != 6 ) {
	G_Printf("Usage: /addmodel (model) (x) (y) (z) (yaw)");
		return;
}

trap_Argv( 1, model, sizeof( model ) );
AddSpawnField("model", model);
trap_Argv( 2, origin_number, 1024 );
jakes_model->s.origin[0] = atoi(origin_number);
trap_Argv( 3, origin_number2, 1024 );
jakes_model->s.origin[1] = atoi(origin_number2);
trap_Argv( 4, origin_number3, 1024 );
//#ifdef __linux__
//jakes_model->s.origin[2] = atoi(origin_number3) - 55;
//#else 
jakes_model->s.origin[2] = atoi(origin_number3) - 60;
//#endif
trap_Argv( 5, yaw_number, 1024 );
jakes_model->s.angles[1] = atoi(yaw_number);
SP_jakes_model(jakes_model);
G_Printf("Added model: %s at <%s %s %s %s>", model, origin_number, origin_number2, origin_number3, yaw_number);
}

void Svcmd_Sleep (void) {
gentity_t * targetplayer;
			int	i;
			int client_id = -1; 
			char   arg1[MAX_STRING_CHARS];
			if ( trap_Argc() != 2 )
			{
				G_Printf ( "Usage: /sleep (client)\n" );
				return;
			}
			if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
			 g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
			 g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
				 G_Printf ( "Cannot use this admin command in this gametype.\n" );
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
			targetplayer->flags |= FL_GODMODE;
			targetplayer->client->ps.forceHandExtendTime = Q3_INFINITE;
			//targetplayer->client->ps.forceDodgeAnim = 0;
			}
		}
		G_LogPrintf("Sleep admin command executed by SERVER on ALL clients. (applying)\n");
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_on_ALL_saying.string ) );
		}
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
		G_LogPrintf("Sleep admin command executed by SERVER on ALL clients. (removing)\n");
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_sleep_off_ALL_saying.string ) );
		}
		 }
		 else {
			client_id = G_ClientNumberFromArg(  arg1 );
			}
			if (client_id == -1) 
         { 
            G_Printf ( "print \"Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
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
            G_Printf ( "print \"Client %s is not active\n", arg1 ); 
            return; 
         }
		 if (g_entities[client_id].client->pers.ampunish == 1){
			G_Printf ( "Client is currently being punished. Please unpunish them before sleeping them.\nTo unpunish them, type in /punish again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amfreeze == 1){
			G_Printf ( "Client is currently frozen. Please unfreeze them before sleeping them.\nTo unfreeze them, type in /freeze again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amdemigod == 1){
			G_Printf ( "Client is currently a demigod. Please undemigod them before sleeping them.\nTo undemigod them, type in /demigod again on the client.\n" );
			 return;
		 }
			if (g_entities[client_id].client->pers.amsleep == 1)
			{
				G_LogPrintf("Sleep admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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
				G_LogPrintf("Sleep admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
				if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_sleep_on_saying.string ) );
		}
			g_entities[client_id].client->pers.amsleep = 1;
			g_entities[client_id].client->ps.forceHandExtendTime = Q3_INFINITE;
			//g_entities[client_id].client->ps.forceDodgeAnim = 0;
			}
}

void	Svcmd_Terminator( void ) {
 gentity_t * targetplayer;
		  int	i, f, a, ft;
         int	client_id = -1; 
         char	arg1[MAX_STRING_CHARS]; 
			if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
			 g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
			 g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
				 G_Printf ( "Cannot use Terminator in this gametype.\n" );
				 return;
			 }
         if ( trap_Argc() != 2 )
         { 
			G_Printf ( "Usage: /terminator (client)\n" );
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
						targetplayer->client->ps.ammo[i] = 999;
					}
		}
				}
			}
		}
		G_LogPrintf("Terminator admin command executed by SERVER on ALL clients. (applying)\n");
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
		G_LogPrintf("Terminator admin command executed by SERVER on ALL clients. (removing)\n");
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
         client_id = G_ClientNumberFromArg( arg1 );
         if (client_id == -1) 
         { 
			G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
			G_Printf ( "Ambiguous client ID for %s\n", arg1 );
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
            G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }
		 if (g_entities[client_id].client->pers.ampunish)
		{
			G_Printf( "print \"Client %s is currently being punished. Unpunish them to terminator them.\n", arg1 ); 
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
		G_LogPrintf("Terminator admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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
		 //ITEMS
			g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_BINOCULARS) | (1 << HI_SEEKER) | (1 << HI_CLOAK) | (1 << HI_EWEB) | (1 << HI_SENTRY_GUN);
			//WEAPONS
			g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
			g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_BLASTER) | (1 << WP_DISRUPTOR) | (1 << WP_BOWCASTER)
			| (1 << WP_REPEATER) | (1 << WP_DEMP2) | (1 << WP_FLECHETTE) | (1 << WP_ROCKET_LAUNCHER) | (1 << WP_THERMAL) | (1 << WP_DET_PACK)
			| (1 << WP_BRYAR_OLD) | (1 << WP_CONCUSSION) | (1 << WP_TRIP_MINE) | (1 << WP_BRYAR_PISTOL);
		// Restore forcepowers:
		if (g_entities[client_id].client->pers.amempower){
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
		 G_LogPrintf("Terminator admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
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

		 void Svcmd_InsultSilence ( void ) {
		 int	client_id = -1; 
			char	arg1[MAX_STRING_CHARS];
			if ( trap_Argc() < 2 )
			{
				G_Printf ( "Type in /help insultsilence if you need help with this command.\n" );
				return;
			}
			trap_Argv( 1, arg1, sizeof( arg1 ) );
			client_id = G_ClientNumberFromArg( arg1 );

			if (client_id == -1)
			{
				G_Printf ( "Can't find client ID for %s\n", arg1 );
				return;
			}
			if (client_id == -2)
			{
				G_Printf ( "Ambiguous client ID for %s\n", arg1 );
				return;
			}
			// either we have the client id or the string did not match
			if (!g_entities[client_id].inuse)
			{ // check to make sure client slot is in use
				G_Printf ( "Client %s is not active\n", arg1 );
				return;
			}
			if (g_entities[client_id].client->pers.ampunish){
			G_Printf ( "Client is currently being punished. Please unpunish them before insult-silencing them. To unpunish them, type in /punish again on the client.\n" );
			 return;
		 }
			if (g_entities[client_id].client->pers.tzone){
			G_Printf ( "Client is currently being silenced. Please unsilence them before insult-silencing them. To unsilence them, type in /silence again on the client.\n" );
			 return;
		 }
			if (g_entities[client_id].client->pers.silent2)
			{
				g_entities[client_id].client->pers.silent2 = qfalse;
				G_LogPrintf("InsultSilence admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
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
			G_LogPrintf("InsultSilence admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
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

		 void Svcmd_Silence ( void ) {
			gentity_t * targetplayer;
			int i;
			int	client_id = -1; 
			char	arg1[MAX_STRING_CHARS];
			if ( trap_Argc() < 2 )
			{
				G_Printf ( "Type in /help silence if you need help with this command.\n" );
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
		G_LogPrintf("Silence admin command executed by SERVER on ALL. (applying)\n");
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
		G_LogPrintf("Silence admin command executed by SERVER on ALL. (removing)\n");
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
				G_Printf ( "Can't find client ID for %s\n", arg1 );
				return;
			}
			if (client_id == -2)
			{
				G_Printf ( "Ambiguous client ID for %s\n", arg1 );
				return;
			}
			// either we have the client id or the string did not match
			if (!g_entities[client_id].inuse)
			{ // check to make sure client slot is in use
				G_Printf ( "Client %s is not active\n", arg1 );
				return;
			}
			if (g_entities[client_id].client->pers.ampunish){
			G_Printf ( "Client is currently being punished. Please unpunish them before silencing them.\nTo unpunish them, type in /punish again on the client.\n" );
			 return;
		 }
			if (g_entities[client_id].client->pers.silent)
			{
				G_LogPrintf("Silence admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
				g_entities[client_id].client->pers.silent = qfalse;
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
			G_LogPrintf("Silence admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
			g_entities[client_id].client->pers.silent = qtrue;
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

void Svcmd_AdminBan ( void ) {
		int client_id = -1;
        char   arg1[MAX_STRING_CHARS];
        if ( trap_Argc() < 2 ) 
        { 
			G_Printf ( "Usage: /adminban <client>\n" );
           return; 
        } 
        trap_Argv( 1,  arg1, sizeof(  arg1 ) );
        client_id = G_ClientNumberFromArg(  arg1 ); 
        if (client_id == -1) 
        { 
			G_Printf ( "Can't find client ID for %s\n", arg1 );
           return; 
        } 
        if (client_id == -2) 
        { 
			G_Printf ( "Ambiguous client ID for %s\n", arg1 );
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
			G_Printf ( "Client %s is not active\n", arg1 );
           return; 
        }
		G_LogPrintf("adminBan admin command executed by SERVER on %s. (IP: %s)\n", g_entities[client_id].client->pers.netname, g_entities[client_id].client->sess.myip);
		if (g_entities[client_id].r.svFlags & SVF_ADMIN){
				 g_entities[client_id].r.svFlags &= ~SVF_ADMIN;
				 g_entities[client_id].client->pers.iamanadmin = 0;
				 g_entities[client_id].client->sess.adminbanned = 1;
				 g_entities[client_id].client->pers.bitvalue = 0;
				 trap_SendConsoleCommand( EXEC_APPEND, va( "AddAdminIP %s", g_entities[client_id].client->sess.myip ) );
				 trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.logout ));
			 }  else {
				 G_Printf ( "%s is not an administrator.\n", arg1 );
			 }
}

void Svcmd_Monk ( void ) {
		gentity_t * targetplayer;
		int	i;
		int f;
		int ft;
		int client_id = -1;
		char	arg1[MAX_STRING_CHARS];
			if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
			 g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
			 g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
				 G_Printf ( "Cannot use Monk in this gametype.\n" );
				 return;
			 }
			if ( trap_Argc() != 2 ) 
         { 
			G_Printf ( "Usage: /monk (client)\n" );
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
		G_LogPrintf("Monk admin command executed by SERVER on ALL. (applying)\n");
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
				targetplayer->client->ps.persistant[PERS_MONK] = 0;
				for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
					targetplayer->client->ps.fd.forcePowerLevel[ft] = targetplayer->client->pers.forcePowerLevelSaved[ft];
				}
				targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
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
		G_LogPrintf("Monk admin command executed by SERVER on ALL. (removing)\n");
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
			if (client_id == -1)
			{
				G_Printf ( "Can't find client ID for %s\n", arg1 );
				return;
			}
			//cm - Dom
		if (client_id == -2) 
         { 
			G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
            return;
         }
		if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
		 if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
			G_Printf ( "Client %s is not active\n", arg1 );
           return; 
        }
		if (g_entities[client_id].client->pers.ampunish)
		{
			G_Printf( "print \"Client %s is currently being punished. Unpunish them to monk them.\n", arg1 ); 
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
			G_LogPrintf("Monk admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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
		 G_LogPrintf("Monk admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
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

void Svcmd_AmVSTR ( void ) {
	char   arg1[MAX_STRING_CHARS];
		trap_Argv( 1,  arg1, sizeof( arg1 ) );
		if ( trap_Argc() != 2 ) 
        { 
			G_Printf ( "Usage: /amvstr (variable)\n" );
			return; 
        }
		G_LogPrintf("AmVSTR (%s) admin command executed by SERVER", arg1);
		trap_SendConsoleCommand( EXEC_INSERT, va( "exec vstr.cfg; wait; wait; wait; vstr %s", arg1 ) );
}
void Svcmd_GrantAdmin ( void ) {
		int client_id = -1; 
        char   arg1[MAX_STRING_CHARS]; 
		char   password[MAX_STRING_CHARS];

		if (trap_Argc() != 3) 
        {
			G_Printf ( "Usage: /grantadmin <client> <password>\n" );
			return;
        }

		trap_Argv( 1, arg1, sizeof( arg1 ) ); // username
		trap_Argv( 2, password, sizeof( password ) ); // password

		client_id = G_ClientNumberFromArg( arg1 ); 
		 if (client_id == -1)
			{
				G_Printf ( "Can't find client ID\n" );
				return;
			}
		if (client_id == -2) 
         { 
			G_Printf ( "Ambiguous client ID\n" ); 
            return;
         }
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID\n" ); 
            return;
         }
		 if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
			G_Printf ( "Client is not active\n" );
           return; 
        }

		if ( g_entities[client_id].client->pers.iamanadmin != 0 ) {
			G_Printf ( "Client is already logged in.\n" );
            return; 
        }

		if ( g_entities[client_id].client->sess.adminbanned == 1 ) {
			G_Printf ("Client is banned from administration.\n");
            return; 
        }
		
		if (strcmp(password, cm_adminPassword1.string) == 0){
		g_entities[client_id].client->pers.bitvalue = cm_adminControl1.integer;
		strcpy(g_entities[client_id].client->pers.login, cm_adminlogin1_saying.string);
		strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout1_saying.string);
		g_entities[client_id].client->pers.iamanadmin = 1;
		g_entities[client_id].r.svFlags |= SVF_ADMIN;
		G_LogPrintf("%s was granted admin by SERVER\n", g_entities[client_id].client->pers.netname);
		trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
		} else if (strcmp(password, cm_adminPassword2.string) == 0){
		g_entities[client_id].client->pers.bitvalue = cm_adminControl2.integer;
		strcpy(g_entities[client_id].client->pers.login, cm_adminlogin2_saying.string);
		strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout2_saying.string);
		g_entities[client_id].client->pers.iamanadmin = 2;
		g_entities[client_id].r.svFlags |= SVF_ADMIN;
		G_LogPrintf("%s was granted admin by SERVER\n", g_entities[client_id].client->pers.netname);
		trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
		} else if (strcmp(password, cm_adminPassword3.string) == 0){
		g_entities[client_id].client->pers.bitvalue = cm_adminControl3.integer;
		strcpy(g_entities[client_id].client->pers.login, cm_adminlogin3_saying.string);
		strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout3_saying.string);
		g_entities[client_id].client->pers.iamanadmin = 3;
		g_entities[client_id].r.svFlags |= SVF_ADMIN;
		G_LogPrintf("%s was granted admin by SERVER\n", g_entities[client_id].client->pers.netname);
		trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
		} else if (strcmp(password, cm_adminPassword4.string) == 0){
		g_entities[client_id].client->pers.bitvalue = cm_adminControl4.integer;
		strcpy(g_entities[client_id].client->pers.login, cm_adminlogin4_saying.string);
		strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout4_saying.string);
		g_entities[client_id].client->pers.iamanadmin = 4;
		g_entities[client_id].r.svFlags |= SVF_ADMIN;
		G_LogPrintf("%s was granted admin by SERVER\n", g_entities[client_id].client->pers.netname);
		trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
		} else if (strcmp(password, cm_adminPassword5.string) == 0){
		g_entities[client_id].client->pers.bitvalue = cm_adminControl5.integer;
		strcpy(g_entities[client_id].client->pers.login, cm_adminlogin5_saying.string);
		strcpy(g_entities[client_id].client->pers.logout, cm_adminlogout5_saying.string);
		g_entities[client_id].client->pers.iamanadmin = 5;
		g_entities[client_id].r.svFlags |= SVF_ADMIN;
		G_LogPrintf("%s was granted admin by SERVER\n", g_entities[client_id].client->pers.netname);
		trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", g_entities[client_id].client->pers.netname, g_entities[client_id].client->pers.login ));
		} else {
			G_Printf ( "Incorrect password.\n" );
		}
}

void	Svcmd_Slay( void ) {
int client_id = -1;
		char	arg1[MAX_STRING_CHARS];
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		client_id = G_ClientNumberFromArg( arg1 );

		if ( trap_Argc() != 2 ) 
         { 
			 G_Printf ( "Usage: /slay (client)\n" ); 
            return; 
         }
		 if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
			 g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
			 g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
				 G_Printf ( "Cannot use this admin command in this gametype.\n" );
				 return;
			 }
			if (client_id == -1)
			{
				G_Printf ( "Can't find client ID for %s\n", arg1 );
				return;
			}

			//cm - Dom
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
            return;
         }
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
		 if (!g_entities[client_id].inuse) 
        { // check to make sure client slot is in use 
			G_Printf ( "Client %s is not active\n", arg1 );
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
					//DismembermentTest(client_id);
					G_LogPrintf("Slay admin command executed by SERVER on %s.\n", g_entities[client_id].client->pers.netname);
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

void Svcmd_Protect( void ) {
			gentity_t * targetplayer;
			int i;
			int client_id = -1; 
			char   arg1[MAX_STRING_CHARS];
			if ( trap_Argc() != 2 )
			{
				G_Printf ( "Usage: /protect (client)\n" );
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
		G_LogPrintf("Protect admin command executed by SERVER on ALL. (applying)\n");
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
		G_LogPrintf("Protect admin command executed by SERVER on ALL. (removing)\n");
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
			if (client_id == -1)
			{
				G_Printf ( "Can't find client ID for %s\n", arg1 );
				return;
			}
			if (client_id == -2)
			{
				G_Printf ( "Ambiguous client ID for %s\n", arg1 );
				return;
			}
			if (g_entities[client_id].client->ps.eFlags2 == EF2_HELD_BY_MONSTER)
			{
				return;
			}
			if (g_entities[client_id].client->pers.amsleep == 1){
				G_Printf ( "Client is currently sleeping. Wake him to protect him.\n" );
				return;
			}
			if (g_entities[client_id].client->pers.ampunish == 1){
				G_Printf ( "Client is currently being punished. Unpunish him to protect him.\n" );
				return;
			}
	if (g_entities[client_id].health <= 0)
		 {
			return;
		 }
			// either we have the client id or the string did not match
			if (!g_entities[client_id].inuse)
			{ // check to make sure client slot is in use
				G_Printf ( "Client %s is not active\n", arg1 );
				return;
			}
			if (g_entities[client_id].client->pers.tzone == 1)
		 {
		 g_entities[client_id].client->pers.tzone = 0;
		 g_entities[client_id].client->pers.autopro = 0;
		g_entities[client_id].takedamage = qtrue;
		g_entities[client_id].client->ps.eFlags &= ~EF_INVULNERABLE;
		G_LogPrintf("Protect admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
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
		G_LogPrintf("Protect admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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

void	Svcmd_DemiGod( void ) {
int client_id = -1; 
         char   arg1[MAX_STRING_CHARS]; 
         if ( trap_Argc() != 2 ) 
         { 
            G_Printf ( "Usage: /demigod (client)\n" ); 
            return; 
         } 
         trap_Argv( 1,  arg1, sizeof(  arg1 ) ); 
         client_id = G_ClientNumberFromArg(  arg1 ); 
         if (client_id == -1) 
         { 
            G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
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
            G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }
		 if (g_entities[client_id].client->pers.amsleep == 1){
			G_Printf ( "Client is currently sleeping. Please unsleep them before demigoding them.\nTo wake them, type in /sleep again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amfreeze == 1){
			G_Printf ( "Client is currently frozen. Please unfreeze them before demigoding them.\nTo unfreeze them, type in /freeze again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.ampunish == 1){
			G_Printf ( "Client is currently being punished. Please unpunish them before demigoding them.\nTo unpunish them, type in /punish again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amsplat == 1){
			G_Printf ( "Client is currently being splatted. Please wait for them to die.\n" );
			 return;
		 }
		 if(g_entities[client_id].client->pers.amdemigod)
		 {
			 G_LogPrintf("DemiGod admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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
			
				}
		 else
		 {
			G_LogPrintf("DemiGod admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
			g_entities[client_id].client->pers.amdemigod = 1;
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

void	Svcmd_Ban( void ) {
	int client_id = -1; 
	char   arg1[MAX_STRING_CHARS];
	char *time;
	time = timestring ( );
	if ( trap_Argc() != 2 ) 
         { 
			G_Printf ( "Usage: /ban (client).\n" ); 
            return; 
         } 
         trap_Argv( 1,  arg1, sizeof(  arg1 ) );
         client_id = G_ClientNumberFromArg(  arg1 ); 
         if (client_id == -1) 
         { 
            G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
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
           G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }
		 if(!(g_entities[client_id].r.svFlags & SVF_BOT)){
			trap_SendConsoleCommand( EXEC_APPEND, va( "AddIP %s", g_entities[client_id].client->sess.myip ) );
		}
		 if ( g_entities[client_id].client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			g_entities[client_id].client->sess.spectatorState = SPECTATOR_FREE;
		}
		G_LogPrintf("Ban admin command executed by SERVER on %s.\n", g_entities[client_id].client->pers.netname);
		trap_DropClient(client_id, "was Banned");
	  }

void	Svcmd_Slap( void ) {
int client_id = -1; 
			char   arg1[MAX_STRING_CHARS];
			if ( trap_Argc() != 2 )
			{
				G_Printf ( "Usage: ^3/slap (client)\n" );
				return;
			}
			trap_Argv( 1, arg1, sizeof( arg1 ) );
			client_id = G_ClientNumberFromArg(  arg1 );
			if (client_id == -1) 
         { 
            G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
            return; 
         } 
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
         if (!g_entities[client_id].inuse) 
         {
            G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }
		 if(g_entities[client_id].client->ps.duelInProgress){
			G_Printf ( "You cannot slap someone who is currently dueling.\n" ); 
			return;
		 }
		 G_LogPrintf("Slap admin command executed by SERVER on %s.\n", g_entities[client_id].client->pers.netname);
		g_entities[client_id].client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
		g_entities[client_id].client->ps.forceHandExtendTime = level.time + 3000;
		g_entities[client_id].client->ps.velocity[2] += 500;
		g_entities[client_id].client->ps.forceDodgeAnim = 0;
		g_entities[client_id].client->ps.quickerGetup = qfalse;
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_slap_saying.string ) );
		}
		}

void	Svcmd_LockTeam( void ) {
char teamname[MAX_TEAMNAME];
	if ( g_gametype.integer >= GT_TEAM || g_gametype.integer == GT_FFA ) {
		
		if ( trap_Argc() != 2 ){
			G_Printf ( "Usage: /lockteam (team)\nTEAMS = Spectator, Blue, Red, Free\n");
			return;
		}
		trap_Argv( 1, teamname, sizeof( teamname ) );
				
		if ( !Q_stricmp( teamname, "red" ) || !Q_stricmp( teamname, "r" ) ) {
			if (level.isLockedred == qfalse){
			level.isLockedred = qtrue;
			trap_SendServerCommand( -1, va("cp \"^7The ^1Red ^7team is now ^1Locked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^1Red ^7team is now ^1Locked^7.\n\""));
			}
			else {
			level.isLockedred = qfalse;
			trap_SendServerCommand( -1, va("cp \"^7The ^1Red ^7team is now ^2unLocked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^1Red ^7team is now ^2unLocked^7.\n\""));
			}
		}
		else if ( !Q_stricmp( teamname, "blue" ) || !Q_stricmp( teamname, "b" ) ) {
			if (level.isLockedblue == qfalse){
			level.isLockedblue = qtrue;
			trap_SendServerCommand( -1, va("cp \"^7The ^4Blue ^7team is now ^1Locked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^4Blue ^7team is now ^1Locked^7.\n\""));
			}
			else {
			level.isLockedblue = qfalse;
			trap_SendServerCommand( -1, va("cp \"^7The ^4Blue ^7team is now ^2unLocked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^4Blue ^7team is now ^2unLocked^7.\n\""));
			}
		}
		else if( !Q_stricmp( teamname, "spectator" ) || !Q_stricmp( teamname, "s" ) || !Q_stricmp( teamname, "spec" ) || !Q_stricmp( teamname, "spectate" ) ) {
			if (level.isLockedspec == qfalse){
			level.isLockedspec = qtrue;
			trap_SendServerCommand( -1, va("cp \"^7The ^3Spectator ^7team is now ^1Locked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^3Spectator ^7team is now ^1Locked^7.\n\""));
			}
			else {
			level.isLockedspec = qfalse;
			trap_SendServerCommand( -1, va("cp \"^7The ^3Spectator ^7team is now ^2unLocked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^3Spectator ^7team is now ^2unLocked^7.\n\""));
			}
		}
		else if( !Q_stricmp( teamname, "join" ) || !Q_stricmp( teamname, "free" ) || !Q_stricmp( teamname, "enter" )
			 || !Q_stricmp( teamname, "f" ) || !Q_stricmp( teamname, "j" )) {
			if (level.isLockedjoin == qfalse){
			level.isLockedjoin = qtrue;
			trap_SendServerCommand( -1, va("cp \"^7The ^2Join ^7team is now ^1Locked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^2Join ^7team is now ^1Locked^7.\n\""));
			}
			else {
			level.isLockedjoin = qfalse;
			trap_SendServerCommand( -1, va("cp \"^7The ^2Join ^7team is now ^2unLocked^7.\n\""));
			trap_SendServerCommand( -1, va("print \"^7The ^2Join ^7team is now ^2unLocked^7.\n\""));
			}
		}
		else {
			G_Printf ( "Usage: /lockteam (team)\nTEAMS = Spectator, Blue, Red, Join\n");
			return;
		}
	}
	else
	{
		G_Printf("Warning: You cannot Lock the teams in this gameplay\n");
		return;
	}
	}

void	Svcmd_ChangeMap( void ) {
char   arg1[MAX_STRING_CHARS]; 
trap_Argv( 1, arg1, sizeof( arg1 ) );
trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype %s\n", arg1));
G_LogPrintf("ChangeMap admin command executed by SERVER to GAMETYPE:%s", arg1);
trap_Argv( 2, arg1, sizeof( arg1 ) );
trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", arg1));
G_LogPrintf(" MAP:%s.\n", arg1);
}

void	Svcmd_Freeze( void ) {
gentity_t * targetplayer;
		int i;
        int client_id = -1; 
        char   arg1[MAX_STRING_CHARS]; 
         if ( trap_Argc() != 2 ) 
         { 
            G_Printf("Usage: /freeze (client)\n" ); 
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
		G_LogPrintf("Freeze admin command executed by SERVER on ALL. (applying)\n");
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
		G_LogPrintf("Freeze admin command executed by SERVER on ALL. (removing)\n");
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
            G_Printf("Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf("Ambiguous client ID for %s\n", arg1 ); 
            return; 
         } 
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
         if (!g_entities[client_id].inuse) 
         { // check to make sure client slot is in use 
            G_Printf("Client %s is not active\n", arg1 ); 
            return; 
         }

		 if (g_entities[client_id].client->pers.ampunish){
			G_Printf("Client is currently being punished. Please unpunish them before freezing them.\nTo unpunish them, type in /punish again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amsleep){
			G_Printf("Client is currently sleeping. Please wake them before freezing them.\nTo wake them, type in /sleep again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amdemigod == 1){
			G_Printf("Client is currently a demigod. Please undemigod them before freezing them.\nTo undemigod them, type in /demigod again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->ps.duelInProgress == 1){
			G_Printf("Client is currently dueling. Please wait for them to finish.\n" );
			return;
		}

		 if (g_entities[client_id].client->pers.amfreeze)
		 {
		 g_entities[client_id].client->pers.amfreeze = 0; 
		 g_entities[client_id].client->ps.forceRestricted = qfalse;
		 G_LogPrintf("Freeze admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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
		 G_LogPrintf("Freeze admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
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

void	Svcmd_Kick( void ) {
int client_id = -1; 
         char   arg1[MAX_STRING_CHARS]; 
         if ( trap_Argc() != 2 ) 
         { 
            G_Printf("Type in /help amkick if you need help with this command.\n" ); 
            return; 
         }
         trap_Argv( 1,  arg1, sizeof( arg1 ) ); 
         client_id = G_ClientNumberFromArg( arg1 ); 
         if (client_id == -1) 
         { 
            G_Printf("Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf("Ambiguous client ID for %s\n", arg1 ); 
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
            G_Printf("Client %s is not active\n", arg1 ); 
            return; 
         }
		 if ( g_entities[client_id].client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			g_entities[client_id].client->sess.spectatorState = SPECTATOR_FREE;
		}
		 trap_DropClient(client_id, "was Kicked");
	  }
char	*ConcatArgs( int start );
void Svcmd_CSPrint( void ) {
		int client_id = -1;
		char   arg[MAX_STRING_CHARS]; 	
		int pos = 0;
		char real_msg[MAX_STRING_CHARS];
		char *msg = ConcatArgs(2);
		while(*msg) { 
    if(msg[0] == '\\' && msg[1] == 'n') { 
          msg++;
          real_msg[pos++] = '\n';
    } else { 
          real_msg[pos++] = *msg;
    } 
    msg++;
}
		 real_msg[pos] = 0;
		 trap_Argv(1, arg, sizeof(arg)); 
         if ( trap_Argc() != 3 ) 
         { 
			G_Printf("Usage: /csprint (client or all) (message).\n" ); 
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
            G_Printf("Can't find client ID for %s\n", arg ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf("Ambiguous client ID for %s\n", arg ); 
            return;
         } 
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg ); 
            return;
         }
         if (!g_entities[client_id].inuse) 
         { // check to make sure client slot is in use 
            G_Printf("Client %s is not active\n", arg ); 
            return; 
         }
			 trap_SendServerCommand(client_id, va("cp \"%s\"", real_msg) );
	  }

//extern void uw5Rename(gentity_t *player, const char *newname);
//extern void uw3Rename(gentity_t *player, const char *newname);
extern void uwRename(gentity_t *player, const char *newname);
void	Svcmd_Rename( void ) {
int client_id = -1; 
   char   arg1[MAX_STRING_CHARS];
   char   arg2[MAX_STRING_CHARS];
   //char   arg3[MAX_STRING_CHARS];

   if ( trap_Argc() != 3) 
   { 
	  G_Printf ( "Usage: /rename <client> <new name>\n");
      return;
   }

   trap_Argv( 1, arg1, sizeof( arg1 ) );
   //trap_Argv( 2,  arg2, sizeof(  arg2 ) );
   client_id = G_ClientNumberFromArg( arg1 );
   if (client_id == -1) 
         { 
            G_Printf("Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf("Ambiguous client ID for %s\n", arg1 ); 
            return; 
         } 
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
         if (!g_entities[client_id].inuse) 
         { // check to make sure client slot is in use 
            G_Printf("Client %s is not active\n", arg1 ); 
            return; 
         }
   trap_Argv( 2, arg2, sizeof( arg2 ) );
	   trap_SendServerCommand( -1, va("print \"%s^7%s\n\"", g_entities[client_id].client->pers.netname, roar_rename_saying.string ) );
		trap_SendServerCommand(client_id, va("cvar name %s", arg2));
		uwRename(&g_entities[client_id], arg2);
   }

void	Svcmd_Splat( void ) {
		int client_id = -1; 
         char   arg1[MAX_STRING_CHARS];
         if ( trap_Argc() != 2 ) 
         { 
			G_Printf("Usage: /splat (client)\n" ); 
            return; 
         } 
         trap_Argv( 1,  arg1, sizeof(  arg1 ) ); 
         client_id = G_ClientNumberFromArg(  arg1 ); 
         if (client_id == -1) 
         { 
            G_Printf("Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf("Ambiguous client ID for %s\n", arg1 ); 
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
            G_Printf("Client %s is not active\n", arg1 ); 
            return; 
         }
		 if (g_entities[client_id].client->pers.tzone == 1 || g_entities[client_id].client->pers.autopro == 1){
			 G_Printf("Can't splat a protected client.\n", arg1 );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amdemigod){
			G_Printf("Client is currently a demigod. Please un-demigod them.\nTo un-demigod them, type in /demigod on them again.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->emote_freeze==1)
				{
					g_entities[client_id].client->emote_freeze=0;
				}
		 if (g_entities[client_id].health > 0)
		 {
		G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue);
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

void	Svcmd_Empower( void ) {
gentity_t * targetplayer;
		int i;
		int f;
		int ft;
	   int client_id = -1; 
       char   arg1[MAX_STRING_CHARS]; 
			if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
			 g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
			 g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
				 G_Printf ( "Cannot use Empower in this gametype.\n" );
				 return;
			 }
			 if ( trap_Argc() != 2 ) 
         { 
			G_Printf ( "Usage: /empower (client)\n" );
            return; 
         }
		 trap_Argv( 1,  arg1, sizeof( arg1 ) );
		 //NOTE: EVERYONE HAS BECOME EMPOWERED!
		 if( Q_stricmp( arg1, "+all" ) == 0 ){
			for( i = 0; i < level.maxclients; i++ )
		{
			targetplayer = &g_entities[i];

			if( targetplayer->client && targetplayer->client->pers.connected ){
				if (!targetplayer->client->pers.amhuman && !targetplayer->client->pers.amfreeze && !targetplayer->client->pers.ampunish
					 && !targetplayer->client->pers.amsleep && !targetplayer->client->pers.amempower && !targetplayer->client->pers.amterminator
					  && !targetplayer->client->pers.amdemigod){
		//restore old force powers so we can save them again...
	if (targetplayer->client->pers.amterminator == 1 || targetplayer->client->pers.amhuman == 1) {
		for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
			targetplayer->client->ps.fd.forcePowerLevel[ft] = targetplayer->client->pers.forcePowerLevelSaved[ft];
		}
		targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
	}
	if (targetplayer->client->pers.amterminator == 0 && targetplayer->client->pers.amhuman == 0) { 
		//MJN's code here
		for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
					// Save
					targetplayer->client->pers.forcePowerLevelSaved[f] = targetplayer->client->ps.fd.forcePowerLevel[f];
					// Set new:
					targetplayer->client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
		}
				// Save and set known powers:
				targetplayer->client->pers.forcePowersKnownSaved = targetplayer->client->ps.fd.forcePowersKnown;
	}
	targetplayer->client->pers.amterminator = 0;
		targetplayer->client->pers.amhuman = 0;
		targetplayer->client->pers.amempower = 1;
		targetplayer->client->ps.eFlags &= ~EF_SEEKERDRONE;
				targetplayer->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
		targetplayer->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
		targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );
		targetplayer->client->ps.weapon = WP_SABER;
				}
			}
		}
		G_LogPrintf("Empower admin command executed by SERVER on ALL. (applying)\n");
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_empower_on_ALL_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_empower_on_ALL_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_empower_on_ALL_saying.string ) );
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_empower_on_ALL_saying.string ) );
		}
		return;
				}
		 //NOTE: EVERYONE IS NOW UNEMPOWERED!
		else if( Q_stricmp( arg1, "-all" ) == 0 ){
			for( i = 0; i < level.maxclients; i++ )
		{
			targetplayer = &g_entities[i];

			if( targetplayer->client && targetplayer->client->pers.connected ){
				if (targetplayer->client->pers.amempower){
				targetplayer->client->ps.persistant[PERS_MONK] = 0;
				targetplayer->client->pers.amempower = 0;
				targetplayer->client->ps.eFlags &= ~EF_BODYPUSH;
				//MJN's code here
				// Restore forcepowers:
		for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
			// Save
			targetplayer->client->ps.fd.forcePowerLevel[f] = targetplayer->client->pers.forcePowerLevelSaved[f];
		}

		// Save and set known powers:
		targetplayer->client->ps.fd.forcePowersKnown = targetplayer->client->pers.forcePowersKnownSaved;
		if (targetplayer->client->ps.duelInProgress == 0){
			if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL)))
			{
				targetplayer->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
			}
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
			}
		}
		G_LogPrintf("Empower admin command executed by SERVER on ALL. (removing)\n");
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_empower_off_ALL_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_empower_off_ALL_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"^7%s\n\"", roar_empower_off_ALL_saying.string ) );
			trap_SendServerCommand( -1, va("print \"^7%s\n\"", roar_empower_off_ALL_saying.string ) );
		}
		return;
		 }
         client_id = G_ClientNumberFromArg( arg1 );
         if (client_id == -1) 
         { 
           G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return;
         }
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
            return;
         }
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
         if (!g_entities[client_id].inuse) 
         {
            G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }
		 if (g_entities[client_id].client->pers.ampunish)
		{
			G_Printf( "print \"Client %s is currently being punished. Unpunish them to empower them.\n", arg1 ); 
            return; 
		}
		 if (g_entities[client_id].client->pers.amempower == 1)
		 {
		//MJN's code here
		// Restore forcepowers:
		g_entities[client_id].client->pers.amempower = 0;
		g_entities[client_id].client->ps.persistant[PERS_MONK] = 0;
		for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
			g_entities[client_id].client->ps.fd.forcePowerLevel[f] = g_entities[client_id].client->pers.forcePowerLevelSaved[f];
		}
		g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
		//
		g_entities[client_id].client->ps.eFlags &= ~EF_BODYPUSH;
		g_entities[client_id].client->ps.powerups[PW_FORCE_ENLIGHTENED_LIGHT] = 0;
		g_entities[client_id].client->ps.powerups[PW_FORCE_ENLIGHTENED_DARK] = 0;
		if (g_entities[client_id].client->ps.duelInProgress == 0){
		if (!(g_weaponDisable.integer & (1 << WP_BRYAR_PISTOL)))
			{
				g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BRYAR_PISTOL );
			}
			if (roar_starting_weapons.integer == 0)
				{
				g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );
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
				g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER ) | (1 << WP_MELEE);
				g_entities[client_id].client->ps.weapon = WP_SABER;
				}
		}
		G_LogPrintf("Empower admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_empower_off_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_empower_off_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_empower_off_saying.string ) );
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_empower_off_saying.string ) );
		}
		 }
		 else {
		//Terminator takeaway
		g_entities[client_id].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SABER );
		g_entities[client_id].client->ps.weapon = WP_SABER;
		g_entities[client_id].client->ps.eFlags &= ~EF_SEEKERDRONE;
		g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
		g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
		if (g_entities[client_id].client->pers.amterminator == 1 || g_entities[client_id].client->pers.amhuman == 1){
		for( ft = 0; ft < NUM_FORCE_POWERS; ft ++ ){
			g_entities[client_id].client->ps.fd.forcePowerLevel[ft] = g_entities[client_id].client->pers.forcePowerLevelSaved[ft];
		}
		g_entities[client_id].client->ps.fd.forcePowersKnown = g_entities[client_id].client->pers.forcePowersKnownSaved;
		}
		//MJN's code here
		for( f = 0; f < NUM_FORCE_POWERS; f ++ ){
					// Save
					g_entities[client_id].client->pers.forcePowerLevelSaved[f] = g_entities[client_id].client->ps.fd.forcePowerLevel[f];
					// Set new:
					g_entities[client_id].client->ps.fd.forcePowerLevel[f] = FORCE_LEVEL_3;
				}
				// Save powers:
				g_entities[client_id].client->pers.forcePowersKnownSaved = g_entities[client_id].client->ps.fd.forcePowersKnown;
		//END MJN's code here.
		g_entities[client_id].client->pers.amterminator = 0;
		G_LogPrintf("Empower admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
		if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, cm_empower_on_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, cm_empower_on_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, cm_empower_on_saying.string ) );
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, cm_empower_on_saying.string ) );
		}
		g_entities[client_id].client->pers.amempower = 1;
		g_entities[client_id].client->pers.amhuman = 0;
		G_ScreenShake(g_entities[client_id].client->ps.origin, &g_entities[client_id],  3.0f, 2000, qtrue); 
         G_Sound(&g_entities[client_id], CHAN_AUTO, G_SoundIndex("sound/effects/electric_beam_lp.wav"));
		 }
		 }

void Svcmd_Punish( void ) {
		 int client_id = -1; 
			char   arg1[MAX_STRING_CHARS];
			if ( trap_Argc() != 2 )
			{
				G_Printf ( "Usage: /punish (client)\n" );
				return;
			}
			trap_Argv( 1, arg1, sizeof( arg1 ) );
			client_id = G_ClientNumberFromArg(  arg1 );
		 if (client_id == -1) 
         { 
            G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return; 
         } 
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
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
            G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }
		 if (g_entities[client_id].client->pers.amsleep == 1){
			G_Printf ( "Client is currently sleeping. Please unsleep them before punishing them.\nTo wake them, type in /sleep again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amfreeze == 1){
			G_Printf ( "Client is currently frozen. Please unfreeze them before punishing them.\nTo unfreeze them, type in /freeze again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->pers.amdemigod == 1){
			G_Printf ( "Client is currently a demigod. Please undemigod them before punishing them.\nTo undemigod them, type in /demigod again on the client.\n" );
			 return;
		 }
		 if (g_entities[client_id].client->ps.duelInProgress == 1){
			G_Printf ( "Client is currently dueling. Please wait for them to finish.\n" );
			return;
		}
		 if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL ||
			 g_gametype.integer == GT_TEAM || g_gametype.integer == GT_CTF ||
			 g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE){
				 G_Printf ( "Cannot use this admin command in this gametype.\n" );
				 return;
			 }
			if (g_entities[client_id].client->pers.ampunish == 1) //Take away punish
			{
			g_entities[client_id].client->ps.weaponTime = 0; //You may use your lightsaber now
			g_entities[client_id].client->pers.ampunish = 0; //Remove persistant flag (for respawn purposes)
			g_entities[client_id].flags &= ~FL_GODMODE; //You are no longer immortal
			G_LogPrintf("Punish admin command executed by SERVER on %s. (removing)\n", g_entities[client_id].client->pers.netname);
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
			g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
			g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL) & ~(1 << WP_SABER) & ~(1 << WP_MELEE);
		}
		g_entities[client_id].client->ps.eFlags &= ~EF_SEEKERDRONE; //Take away weapons cause u might try to annoy us with sounds/effects.
			}
			else { //Give punish
			g_entities[client_id].client->pers.ampunish = 1;
			G_LogPrintf("Punish admin command executed by SERVER on %s. (applying)\n", g_entities[client_id].client->pers.netname);
			if (roar_sayings_display.integer == 0){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
		} else if (roar_sayings_display.integer == 1){
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
		} else if (roar_sayings_display.integer >= 2){
			trap_SendServerCommand( -1, va("cp \"%s^7\n%s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
			trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", g_entities[client_id].client->pers.netname, roar_punish_on_saying.string ) );
		}
			g_entities[client_id].client->ps.eFlags &= ~EF_SEEKERDRONE; //Take away weapons cause u might try to annoy us with sounds/effects.
			g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_BINOCULARS) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_EWEB) & ~(1 << HI_CLOAK);
			g_entities[client_id].client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
			& ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
			& ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL) & ~(1 << WP_SABER) & ~(1 << WP_MELEE);
			}
		}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	//gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];
	int client_id = -1;
	char   arg1[MAX_STRING_CHARS]; 

	if ( trap_Argc() != 3 ){
		G_Printf ( "Usage: /forceteam (client) (team)\nTEAMS = Spectator, Blue, Red, Free\n");
		return;
	}
	// find the player
	//cm - Jake BASEJKA was here.
	/*trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}*/
	trap_Argv( 1,  arg1, sizeof( arg1 ) );
	client_id = G_ClientNumberFromArg( arg1 );
         if (client_id == -1) 
         { 
           G_Printf ( "Can't find client ID for %s\n", arg1 ); 
            return;
         }
         if (client_id == -2) 
         { 
            G_Printf ( "Ambiguous client ID for %s\n", arg1 ); 
            return;
         }
		 if (client_id >= MAX_CLIENTS || client_id < 0) 
         { 
			G_Printf ( "Bad client ID for %s\n", arg1 ); 
            return;
         }
         if (!g_entities[client_id].inuse) 
         {
            G_Printf ( "Client %s is not active\n", arg1 ); 
            return; 
         }

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	//SetTeam( &g_entities[cl - level.clients], str );
	SetTeam(&g_entities[client_id], str );
}

char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

/*	if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}
*/
	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addadminip") == 0) {
		Svcmd_AddAdminIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeadminip") == 0) {
		Svcmd_RemoveAdminIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		int		i;
		byte	b[4];

		G_Printf("%d IP slots used.\n", numIPFilters);
		for(i = 0; i < numIPFilters; i++) {
		//G_Printf("%d: ", i); // purposely no new line here as it is added after ip data is

		if (ipFilters[i].compare == 0xffffffff) {
			G_Printf("unused\n");
		} else {
			*(unsigned *)b = ipFilters[i].compare;
			G_Printf("%i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
		}
		}
	}
	
	if (Q_stricmp (cmd, "listadminip") == 0) {
		int		i;
		byte	b[4];

		G_Printf("%d IP slots used.\n", numIPAdminFilters);
		for(i = 0; i < numIPAdminFilters; i++) {
		//G_Printf("%d: ", i); // purposely no new line here as it is added after ip data is

		if (ipAdminFilters[i].compare == 0xffffffff) {
			G_Printf("unused\n");
		} else {
			*(unsigned *)b = ipAdminFilters[i].compare;
			G_Printf("%i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
		}
		}
	}

	//cm - Jake
	//NOTE: This must be BEFORE "say" otherwhise, server will skip this.
	//BEGIN
	if ((Q_stricmp (cmd, "punish") == 0) || (Q_stricmp (cmd, "ampunish") == 0)) {
		Svcmd_Punish();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "sleep") == 0) || (Q_stricmp (cmd, "amsleep") == 0)) {
		Svcmd_Sleep();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addeffect") == 0) {
		Svcmd_AddEffect();
		return qtrue;
	}

	if (Q_stricmp (cmd, "weather") == 0) {
		Svcmd_Weather();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addmodel") == 0) {
		Svcmd_AddModel();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "silence") == 0) || (Q_stricmp (cmd, "amsilence") == 0)) {
		Svcmd_Silence();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "insultsilence") == 0) || (Q_stricmp (cmd, "aminsultsilence") == 0)) {
		Svcmd_InsultSilence();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "grantadmin") == 0) || (Q_stricmp (cmd, "amgrantadmin") == 0)) {
		Svcmd_GrantAdmin();
		return qtrue;
	}

	if (Q_stricmp(cmd, "cmregister") == 0){
		char *user[MAX_STRING_CHARS];
		char *pass[MAX_STRING_CHARS];

		trap_Argv(1, user, sizeof(user));
		trap_Argv(2, pass, sizeof(pass));

		if ((trap_Argc() < 2) || (trap_Argc() > 3))
		{
			G_Printf("Usage: /cmregister <playername> <password>\n");
			return;
		}

		if (cm_database.integer == 1) {
			sqliteRegisterUser("INSERT INTO users (user, pass, ipaddress) VALUES ('%s', '%s', '127.0.0.1')", user, pass);
			G_Printf("User %s now registered.\n", user);
		}
		else if (cm_database.integer == 2) {
			parse_server_output(va("mysqlRegisterUser curl --data \"key=%s&p=register&user=%s&pass=%s&ipaddress=%s\" %s", cm_mysql_secret.string, user, pass, "127.0.0.1", cm_mysql_url.string));
		}
	}
	if (Q_stricmp(cmd, "cmfinduser") == 0){
		char *user[MAX_STRING_CHARS];

		trap_Argv(1, user, sizeof(user));

		if ((trap_Argc() < 1) || (trap_Argc() > 2))
		{
			G_Printf("Usage: /cmfinduser <playername>");
			return;
		}

		int userID = 0;

		if (cm_database.integer == 1) {
			userID = sqliteSelectUserID("SELECT * FROM users WHERE user = '%s'", user);
			if (userID > 0)
				G_Printf("USER FOUND - ID: %i\n", userID);
			else {
				G_Printf("USER NOT FOUND");
			}
		}
		else if (cm_database.integer == 2)
			parse_server_output(va("mysqlFindUser curl --data \"key=%s&p=find&user=%s\" %s", cm_mysql_secret.string, user, cm_mysql_url.string));
	}
	if (Q_stricmp(cmd, "shapass") == 0) {
		char   arg1[MAX_STRING_CHARS];
		char test[40] = "";
		if (trap_Argc() < 2)
		{
			G_Printf("Usage: /shapass <password>\n");
			return;
		}
		trap_Argv(1, arg1, sizeof(arg1));
		G_Printf(SHA1ThisPass(arg1));
	}
	if (Q_stricmp(cmd, "cmstats") == 0) {
		char *user[MAX_STRING_CHARS];
		int getID = 0;

		trap_Argv(1, user, sizeof(user));
		G_Printf("Getting stats for %s...\n", user);

		if (cm_database.integer == 1) {
			getID = sqliteSelectUserID("SELECT * FROM users WHERE user = '%s'", user);
			if (getID > 0) {
				G_Printf("USER FOUND - ID: %i\n", getID);
				G_Printf("%s\n", sqliteGetStats("SELECT * FROM stats WHERE user_id = '%i'", getID));
			}
			else
				G_Printf("USER NOT FOUND");
		}
		else if (cm_database.integer == 2)
			parse_server_output(va("mysqlUserStats curl --data \"key=%s&p=find&user=%s\" %s", cm_mysql_secret.string, user, cm_mysql_url.string));
	}
	if (Q_stricmp(cmd, "cmleaderboard") == 0 || Q_stricmp(cmd, "cmleaders") == 0) {

		if (cm_database.integer <= 0)
			G_Printf("Database commands have been disabled on this server.\n");

		char rows[255];
		char column[60];
		char query[MAX_STRING_CHARS];

		if (g_gametype.integer == GT_FFA || g_gametype.integer == GT_HOLOCRON || g_gametype.integer == GT_JEDIMASTER) {
			Q_strncpyz(rows, "user_id,kills,deaths", sizeof(rows));
			Q_strncpyz(column, "kills", sizeof(column));
		}
		else if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) {
			Q_strncpyz(rows, "user_id,duel_wins,duel_loses", sizeof(rows));
			Q_strncpyz(column, "duel_wins", sizeof(column));
		}
		else if (g_gametype.integer == GT_TEAM) {
			Q_strncpyz(rows, "user_id,tdm_wins,tdm_loses", sizeof(rows));
			Q_strncpyz(column, "tdm_wins", sizeof(column));
		}
		else if (g_gametype.integer == GT_SIEGE) {
			Q_strncpyz(rows, "user_id,siege_wins,siege_loses", sizeof(rows));
			Q_strncpyz(column, "siege_wins", sizeof(column));
		}
		else if (g_gametype.integer == GT_CTF || g_gametype.integer == GT_CTY) {
			Q_strncpyz(rows, "user_id,ctf_wins,ctf_loses", sizeof(rows));
			Q_strncpyz(column, "ctf_wins", sizeof(column));
		}

		if (cm_database.integer == 1) {
			Q_strcat(query, sizeof(query), sqliteGetLeaders("SELECT %s FROM stats ORDER BY %s DESC LIMIT 5", rows, column));
			G_Printf(query);
		}
		else if (cm_database.integer == 2)
			parse_server_output(va("mysqlGetLeaders curl --data \"key=%s&p=leaders&g=jedi_academy&r=%s&o=%s\" %s", cm_mysql_secret.string, rows, column, cm_mysql_url.string));
			//sprintf(query, "%s", mysqlGetLeaders(parse_output(va("curl --data \"key=%s&p=leaders&g=jedi_academy&r=%s&o=%s\" %s", cm_mysql_secret.string, rows, column, cm_mysql_url.string))));
			//Q_strcat(query, sizeof(query), mysqlGetLeaders(parse_output(va("curl --data \"key=%s&p=leaders&g=jedi_academy&r=%s&o=%s\" %s", cm_mysql_secret.string, rows, column, cm_mysql_url.string))));
	}

	if (Q_stricmp(cmd, "pipetest") == 0) {
		char   arg1[255];
		char   arg2[1024];
		if (trap_Argc() < 3)
		{
			G_Printf("Usage: /pipetest <function> <value>\n");
			return;
		}
		trap_Argv(1, arg1, sizeof(arg1));
		trap_Argv(2, arg2, sizeof(arg2));
		G_Printf("[DEBUG] SENDING %s TO PIPE %s\n", arg2, arg1);
	}

	if (Q_stricmp (cmd, "amvstr") == 0) {
		Svcmd_AmVSTR();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "empower") == 0) || (Q_stricmp (cmd, "amempower") == 0)) {
		Svcmd_Empower();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "splat") == 0) || (Q_stricmp (cmd, "amsplat") == 0)) {
		Svcmd_Splat();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "rename") == 0) || (Q_stricmp (cmd, "amrename") == 0)) {
		Svcmd_Rename();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "csprint") == 0) || (Q_stricmp (cmd, "amcsprint") == 0)) {
		Svcmd_CSPrint();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "kick") == 0) || (Q_stricmp (cmd, "amkick") == 0)) {
		Svcmd_Kick();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "freeze") == 0) || (Q_stricmp (cmd, "amfreeze") == 0)) {
		Svcmd_Freeze();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "changemap") == 0) || (Q_stricmp (cmd, "amchangemap") == 0)) {
		Svcmd_ChangeMap();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "lockteam") == 0) || (Q_stricmp (cmd, "amlockteam") == 0)) {
		Svcmd_LockTeam();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "slap") == 0) || (Q_stricmp (cmd, "amslap") == 0)) {
		Svcmd_Slap();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "ban") == 0) || (Q_stricmp (cmd, "amban") == 0)) {
		Svcmd_Ban();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "protect") == 0) || (Q_stricmp (cmd, "amprotect") == 0)) {
		Svcmd_Protect();
		return qtrue;
	}

	if ((Q_stricmp(cmd,"amclip") == 0) || (Q_stricmp (cmd, "demigod") == 0) || (Q_stricmp (cmd, "amdemigod") == 0)) {
		Svcmd_DemiGod();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "slay") == 0) || (Q_stricmp (cmd, "amslay") == 0)) {
		Svcmd_Slay();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "terminator") == 0) || (Q_stricmp (cmd, "amterminator") == 0)) {
		Svcmd_Terminator();
		return qtrue;
	}

	if ((Q_stricmp (cmd, "monk") == 0) || (Q_stricmp (cmd, "ammonk") == 0)) {
		Svcmd_Monk();
		return qtrue;
	}

	if (Q_stricmp (cmd, "adminban") == 0) {
		Svcmd_AdminBan();
		return qtrue;
	}
	//END

	//RoAR mod NOTE: This is where the server speaks!
	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			//trap_SendServerCommand( -1, va("print \"^0*^1SERVER^0*^7: ^2%s\n\"", ConcatArgs(1) ) );
			//G_LogPrintf( "^0*^1SERVER^0*^7: %s\n", ConcatArgs(1) );
			trap_SendServerCommand( -1, va("%s \"^0*^1SERVER^0*^7 > %s\"", "chat", ConcatArgs(1) ));
			return qtrue;
		}
		// everything else will also be printed as a say command
		//trap_SendServerCommand( -1, va("print \"^0*^1SERVER^0*^7: ^2%s\n\"", ConcatArgs(0) ) );
		//G_LogPrintf( "^0*^1SERVER^0*^7: %s\n", ConcatArgs(0) );
		trap_SendServerCommand( -1, va("%s \"^0*^1SERVER^0*^7 > %s\"", "chat", ConcatArgs(0) ));
		return qtrue;
	}

	return qfalse;
}

