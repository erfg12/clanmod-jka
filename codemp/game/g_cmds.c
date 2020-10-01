// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "bg_saga.h"
#include "g_adminshared.h"
#include "timestamp.h"
#include "../sqlite3/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include "sha1.h"
#ifdef _WIN32
#include "windows.h"
#endif
#ifdef __linux__
#include <pthread.h>
#endif

#include "../../ui/menudef.h"			// for the voice chats

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

#include "../namespace_begin.h"
void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );
#include "../namespace_end.h"
#include "g_cmds.h"

// Required for holocron edits.
//[HolocronFiles]
extern vmCvar_t bot_wp_edit;
//[/HolocronFiles]
//
void CG_CenterPrint( const char *str, int y, int charWidth );
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);
extern int G_ClientNumberFromArg( char *str);
void ChangeWeapon( gentity_t *ent, int newWeapon );

char *mysqlGetStats(gentity_t *ent, char *stmt) {
    char *data = malloc(1024);
    sprintf(data, "");

    char * pch;
    int i = 0;
    const char *a[20];
    //printf("[DEBUG] Splitting string \"%s\" into tokens:\n", stmt);
    pch = strtok(stmt, ",");
    while (pch != NULL)
    {
        a[i] = pch;
        i++;
        pch = strtok(NULL, ",");
    }

    char * kills = a[0] + ent->client->pers.sql_kills;
    char * deaths = a[1] + ent->client->pers.sql_deaths;
    char * duel_wins = a[2] + ent->client->pers.sql_duelwins;
    char * duel_loses = a[3] + ent->client->pers.sql_duelloses;
    char * flag_captures = a[4] + ent->client->pers.sql_flagcaps;
    char * ffa_wins = a[5] + ent->client->pers.sql_ffawins;
    char * ffa_loses = a[6] + ent->client->pers.sql_ffaloses;
    char * tdm_wins = a[7] + ent->client->pers.sql_tdmwins;
    char * tdm_loses = a[8] + ent->client->pers.sql_tdmloses;
    char * siege_wins = a[9] + ent->client->pers.sql_siegewins;
    char * siege_loses = a[10] + ent->client->pers.sql_siegeloses;
    char * ctf_wins = a[11] + ent->client->pers.sql_ctfwins;
    char * ctf_loses = a[12] + ent->client->pers.sql_ctfloses;

    sprintf(data, "Kills/Deaths: %s/%s \n", kills, deaths);
    sprintf(data + strlen(data), "Duel Wins/Loses: %s/%s \n", duel_wins, duel_loses);
    sprintf(data + strlen(data), "Flag Captures: %s \n", flag_captures);
    sprintf(data + strlen(data), "FFA Wins/Loses: %s/%s \n", ffa_wins, ffa_loses);
    sprintf(data + strlen(data), "TDM Wins/Loses: %s/%s \n", tdm_wins, tdm_loses);
    sprintf(data + strlen(data), "CTF Wins/Loses: %s/%s \n", ctf_wins, ctf_loses);
    sprintf(data + strlen(data), "Siege Wins/Loses: %s/%s \n", siege_wins, siege_loses);
    return data;
}

#ifdef _WIN32
typedef struct ARGS {
    gentity_t *ent;
    char *cmd;
} args;
#endif

#ifdef __linux__
struct thread_info {
    gentity_t *ent;
    char *sntCmd;
} tinfo;
#endif

#ifdef _WIN32
DWORD WINAPI ThreadFunc(LPVOID sntCmd) {
    FILE *fp;
    char buf[2048];
    char * s = "";
    char cmd[2048] = "";
    int * type = 0;

    args *twoCmds = (args*)sntCmd;

    printf("cmd: %s\n", twoCmds->cmd);

    gentity_t *ent = twoCmds->ent;

    sprintf(cmd, "%s", twoCmds->cmd);

    if (strstr(cmd, "mysqlGetLeaders") != NULL) {
        type = 1;
        strcpy(cmd, replace_str(cmd, "mysqlGetLeaders ", ""));
    }
    if (strstr(cmd, "mysqlRegisterUser")) {
        type = 2;
        strcpy(cmd, replace_str(cmd, "mysqlRegisterUser ", ""));
    }
    if (strstr(cmd, "mysqlUserLogin")) {
        type = 3;
        strcpy(cmd, replace_str(cmd, "mysqlUserLogin ", ""));
    }
    if (strstr(cmd, "mysqlUserStats")) {
        type = 4;
        strcpy(cmd, replace_str(cmd, "mysqlUserStats ", ""));
    }
    if (strstr(cmd, "mysqlUpdateStats")) {
        type = 5;
        strcpy(cmd, replace_str(cmd, "mysqlUpdateStats ", ""));
    }

    if ((fp = _popen(cmd, "r")) == NULL) {
        return "Error opening pipe!\n";
    }

    while (fgets(buf, 1024, fp) != NULL) {
        s = malloc(snprintf(NULL, 0, "%s", buf) + 1);
        sprintf(s, "%s", buf);
    }

    if (type == 1) {
        trap_SendServerCommand(ent-g_entities, va("print \"^3===^1GAMETYPE LEADERBOARD^3===\n\n%s\n\"", mysqlGetLeaders(s)));
        strcpy(ent->client->csMessage, G_NewString(va("^3===^1GAMETYPE LEADERBOARD^3===\n\n%s\n\"", mysqlGetLeaders(s))));
    }
    if (type == 2) {
        if (strstr(s, "success"))
            trap_SendServerCommand(ent-g_entities, va("print \"User now registered.\n\""));
    }
    if (type == 3) {
        int * userID = atoi(s);
        if (userID > 0) {
            ent->client->pers.userID = userID;
            trap_SendServerCommand(ent-g_entities, va("print \"^3You are now logged in.\n\""));
        }
        else
            trap_SendServerCommand(ent-g_entities, "USER NOT FOUND");
    }
    if (type == 4) {
        char * mysqlMsg = mysqlGetStats(ent,s);
        trap_SendServerCommand(ent-g_entities, va("print \"^3===^1YOUR PLAYER STATUS^3===\n\n%s\n\"", mysqlMsg));
        strcpy(ent->client->csMessage, G_NewString(va("^3===^1YOUR PLAYER STATUS^3===\n\n%s\n\"", mysqlMsg)));
    }
    if (type == 5) {
        trap_SendServerCommand(ent-g_entities, va("print \"^1[DEBUG] Increased in DB.\n\""));
    }

    if (_pclose(fp)) {
        printf("Command not found or exited with error status\n");
    }

    return 0;
}
#endif
#ifdef __linux__

void *linuxThread(void *args)
{
    FILE *fp;
    char buf[2048];
    char * s = "";
    char cmd[2048] = "";
    int * type = 0;

    G_Printf("cmd: %s\n", tinfo.sntCmd);

    gentity_t *ent = tinfo.ent;

    sprintf(cmd, "%s", tinfo.sntCmd);

    if (strstr(cmd, "mysqlGetLeaders") != NULL) {
        type = 1;
        strcpy(cmd, replace_str(cmd, "mysqlGetLeaders ", ""));
    }
    if (strstr(cmd, "mysqlRegisterUser")) {
        type = 2;
        strcpy(cmd, replace_str(cmd, "mysqlRegisterUser ", ""));
    }
    if (strstr(cmd, "mysqlUserLogin")) {
        type = 3;
        strcpy(cmd, replace_str(cmd, "mysqlUserLogin ", ""));
    }
    if (strstr(cmd, "mysqlUserStats")) {
        type = 4;
        strcpy(cmd, replace_str(cmd, "mysqlUserStats ", ""));
    }

    if ((fp = popen(cmd, "r")) == NULL) {
        return "Error opening pipe!\n";
    }

    while (fgets(buf, 1024, fp) != NULL) {
        s = malloc(snprintf(NULL, 0, "%s", buf) + 1);
        sprintf(s, "%s", buf);
    }

    if (type == 1) {
        trap_SendServerCommand(ent-g_entities, va("print \"^3===^1GAMETYPE LEADERBOARD^3===\n\n%s\n\"", mysqlGetLeaders(s)));
        strcpy(ent->client->csMessage, G_NewString(va("^3===^1GAMETYPE LEADERBOARD^3===\n\n%s\n\"", mysqlGetLeaders(s))));
    } if (type == 2) {
        if (strstr(s, "success"))
            trap_SendServerCommand(ent-g_entities, va("print \"User now registered.\n\""));
    }
    if (type == 3) {
        int * userID = atoi(s);
        if (userID > 0) {
            ent->client->pers.userID = userID;
            trap_SendServerCommand(ent-g_entities, va("print \"^3You are now logged in.\n\""));
        }
        else
            trap_SendServerCommand(ent-g_entities, "USER NOT FOUND");
    }
    if (type == 4) {
        char * mysqlMsg = mysqlGetStats(ent,s);
        trap_SendServerCommand(ent-g_entities, va("print \"^3===^1YOUR PLAYER STATUS^3===\n\n%s\n\"", mysqlMsg));
        strcpy(ent->client->csMessage, G_NewString(va("^3===^1YOUR PLAYER STATUS^3===\n\n%s\n\"", mysqlMsg)));
    }

    if (pclose(fp)) {
        printf("Command not found or exited with error status\n");
    }

    return s;
}
#endif

void parse_output(gentity_t *ent, char *cmd) {
#ifdef _WIN32
    DWORD   threadID;
    G_Printf("ent: %i cmd: %s\n", ent-g_entities, cmd);
    args argys = { ent, cmd };
    HANDLE thread = 0;
    thread = CreateThread(NULL, 0, ThreadFunc, &argys, 0, &threadID);

    if (thread == NULL) {
        G_Printf("ERROR: Thread Handle is null!");
    }
    if (threadID == NULL) {
        G_Printf("ERROR: Thread ID is null!");
    }
    if (thread) {
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread); //detach
    }
#endif

#ifdef __linux__
    pthread_t tid;
    G_Printf("ent: %i cmd: %s\n", ent-g_entities, cmd);
    
    tinfo.sntCmd = cmd;
    tinfo.ent = ent;

    pthread_create(&tid, NULL, &linuxThread, &tinfo);
    pthread_detach(&tid);
#endif
}

/*
   ===================
   SQLite 3 database stuff

   ===================
 */
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for (i = 0; i < argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void sqliteRegisterUser(char *SQLStmnt, ...) {
    va_list		argptr;
    char		text[1024];

    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open("clanmod.db", &db);
    if (rc != SQLITE_OK) //error connecting
    {
        sqlite3_close(db);
        return;
    }

    va_start(argptr, SQLStmnt);
    vsprintf(text, SQLStmnt, argptr);
    va_end(argptr);
    rc = sqlite3_exec(db, text, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        G_Printf( "Database error" );
    }
    else {
        int lastid = sqlite3_last_insert_rowid(db);
        char sql[1024];
        sprintf(sql, "INSERT INTO stats (user_id) VALUES ('%i')", lastid);
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            G_Printf("Database error\n");
        }
        else {
            sqlite3_close(db);
            G_Printf("Database insert success\n");
        }
    }
}

void sqliteUpdateStats(char *SQLStmnt, ...) {
    va_list		argptr;
    char		text[1024];

    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open("clanmod.db", &db);
    if (rc) { //error connecting
        sqlite3_close(db);
        G_Printf("ERROR: Can't connecting to clanmod.db is the file in your GameData folder?\n");
    }

    va_start(argptr, SQLStmnt);
    vsprintf(text, SQLStmnt, argptr);
    va_end(argptr);
    rc = sqlite3_exec(db, text, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        G_Printf("Database error\n");
    }
    else {
        sqlite3_close(db);
        G_Printf("Database insert success\n");
    }
}

int sqliteSelectUserID(char *SQLStmnt, ...) {
    sqlite3_stmt *stmt;
    sqlite3 *db;
    char *zErrMsg = 0;
    va_list		argptr;
    char		text[1024];

    va_start(argptr, SQLStmnt);
    vsprintf(text, SQLStmnt, argptr);
    va_end(argptr);

    if (sqlite3_open("clanmod.db", &db) == SQLITE_OK)
    {
        //G_Printf(text);
        int rc = sqlite3_prepare_v2(db, text, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
            fprintf(stderr, "SQL error: %s\n", zErrMsg);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            G_Printf("user ID not found\n");
            return 0;
        }

        if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            G_Printf("user ID not found\n");
            return 0;
        }

        int id = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return id;
    }
    else
        return 0;
}

char *sqliteGetName(char *SQLStmnt, ...) {
    sqlite3_stmt *stmt;
    sqlite3 *db;
    char *zErrMsg = 0;
    va_list		argptr;
    char		text[1024];

    va_start(argptr, SQLStmnt);
    vsprintf(text, SQLStmnt, argptr);
    va_end(argptr);

    if (sqlite3_open("clanmod.db", &db) == SQLITE_OK)
    {
        //G_Printf(text);
        int rc = sqlite3_prepare_v2(db, text, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
            fprintf(stderr, "SQL error: %s\n", zErrMsg);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            G_Printf("user name not found\n");
            return 0;
        }

        if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            G_Printf("user name not found\n");
            return 0;
        }

        char *data = malloc(1024);
        sprintf(data, sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);
        return data;
    }
    else
        return "ERROR CANT OPEN DB FILE";
}

char *SHA1ThisPass(char *myPass) {
    SHA1Context sha;
    int i;
    char *balance[5];

    SHA1Reset(&sha);
    SHA1Input(&sha, myPass, strlen(myPass));

    if (!SHA1Result(&sha))
        fprintf(stderr, "ERROR-- could not compute message digest\n");
    else
    {
        for (i = 0; i < 5; i++)
        {
            balance[i] = va("%X", sha.Message_Digest[i]);
        }
    }
    char * s = malloc(snprintf(NULL, 0, "%s%s%s%s%s", balance[0], balance[1], balance[2], balance[3], balance[4]) + 1);
    sprintf(s, "%s%s%s%s%s", balance[0], balance[1], balance[2], balance[3], balance[4]);

    return s;
}

void updateStats(gentity_t *ent) {
    if (cm_database.integer <= 0)
        return;
    
    if (cm_database.integer == 1)
        sqliteUpdateStats("UPDATE stats SET kills=kills+%i, deaths=deaths+%i, duel_wins=duel_wins+%i, duel_loses=duel_loses+%i, flag_captures=flag_captures+%i, ffa_wins=ffa_wins+%i, ffa_loses=ffa_loses+%i, tdm_wins=tdm_wins+%i, tdm_loses=tdm_loses+%i, siege_wins=siege_wins+%i, siege_loses=siege_loses+%i, ctf_wins=ctf_wins+%i, ctf_loses=ctf_loses+%i WHERE user_id = '%i'", ent->client->pers.sql_kills, ent->client->pers.sql_deaths, ent->client->pers.sql_duelwins, ent->client->pers.sql_duelloses, ent->client->pers.sql_flagcaps, ent->client->pers.sql_ffawins, ent->client->pers.sql_ffaloses, ent->client->pers.sql_tdmwins, ent->client->pers.sql_tdmloses, ent->client->pers.sql_siegewins, ent->client->pers.sql_siegeloses, ent->client->pers.sql_ctfwins, ent->client->pers.sql_ctfloses, ent->client->pers.userID);
    else if (cm_database.integer == 2)
        parse_output(ent, va("curl --data \"key=%s&p=increase&g=jedi_academy&query=%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i&id=%i\" %s", cm_mysql_secret.string, ent->client->pers.sql_kills, ent->client->pers.sql_deaths, ent->client->pers.sql_duelwins, ent->client->pers.sql_duelloses, ent->client->pers.sql_flagcaps, ent->client->pers.sql_ffawins, ent->client->pers.sql_ffaloses, ent->client->pers.sql_tdmwins, ent->client->pers.sql_tdmloses, ent->client->pers.sql_siegewins, ent->client->pers.sql_siegeloses, ent->client->pers.sql_ctfwins, ent->client->pers.sql_ctfloses, ent->client->pers.userID, cm_mysql_url.string));
    G_LogPrintf("Stat updated for user ID:%i - DATA[%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i]", ent->client->pers.userID, ent->client->pers.sql_kills, ent->client->pers.sql_deaths, ent->client->pers.sql_duelwins, ent->client->pers.sql_duelloses, ent->client->pers.sql_flagcaps, ent->client->pers.sql_ffawins, ent->client->pers.sql_ffaloses, ent->client->pers.sql_tdmwins, ent->client->pers.sql_tdmloses, ent->client->pers.sql_siegewins, ent->client->pers.sql_siegeloses, ent->client->pers.sql_ctfwins, ent->client->pers.sql_ctfloses);
}

char * replace_str(const char *string, const char *substr, const char *replacement) {
    char *tok = NULL;
    char *newstr = NULL;
    char *oldstr = NULL;
    char *head = NULL;

    /* if either substr or replacement is NULL, duplicate string a let caller handle it */
    if (substr == NULL || replacement == NULL) return strdup(string);
    newstr = strdup(string);
    head = newstr;
    while ((tok = strstr(head, substr))) {
        oldstr = newstr;
        newstr = malloc(strlen(oldstr) - strlen(substr) + strlen(replacement) + 1);
        /*failed to alloc mem, free old string and return NULL */
        if (newstr == NULL) {
            free(oldstr);
            return NULL;
        }
        memcpy(newstr, oldstr, tok - oldstr);
        memcpy(newstr + (tok - oldstr), replacement, strlen(replacement));
        memcpy(newstr + (tok - oldstr) + strlen(replacement), tok + strlen(substr), strlen(oldstr) - strlen(substr) - (tok - oldstr));
        memset(newstr + strlen(oldstr) - strlen(substr) + strlen(replacement), 0, 1);
        /* move back head right after the last replacement */
        head = newstr + (tok - oldstr) + strlen(replacement);
        free(oldstr);
    }
    return newstr;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result = 0;
    size_t count = 0;
    char* tmp = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}



char *sqliteGetStats(char *SQLStmnt, ...) {
    sqlite3_stmt *stmt;
    sqlite3 *db;
    char *zErrMsg = 0;
    va_list		argptr;
    char		text[1024];

    va_start(argptr, SQLStmnt);
    vsprintf(text, SQLStmnt, argptr);
    va_end(argptr);

    if (sqlite3_open("clanmod.db", &db) == SQLITE_OK)
    {
        int rc = sqlite3_prepare_v2(db, text, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
            fprintf(stderr, "SQL error: %s\n", zErrMsg);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            G_Printf("user ID not found\n");
            return;
        }

        if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            G_Printf("user ID not found\n");
            return;
        }
        int id = sqlite3_column_int(stmt, 0);
        int kills = sqlite3_column_int(stmt, 1);
        int deaths = sqlite3_column_int(stmt, 2);
        int duel_wins = sqlite3_column_int(stmt, 3);
        int duel_loses = sqlite3_column_int(stmt, 4);
        int flag_captures = sqlite3_column_int(stmt, 5);
        int ffa_wins = sqlite3_column_int(stmt, 6);
        int ffa_loses = sqlite3_column_int(stmt, 7);
        int tdm_wins = sqlite3_column_int(stmt, 8);
        int tdm_loses = sqlite3_column_int(stmt, 9);
        int siege_wins = sqlite3_column_int(stmt, 10);
        int siege_loses = sqlite3_column_int(stmt, 11);
        int ctf_wins = sqlite3_column_int(stmt, 12);
        int ctf_loses = sqlite3_column_int(stmt, 13);
        sqlite3_finalize(stmt);

        char *data = malloc(1024);
        sprintf(data, "Kills/Deaths: %i/%i \n", kills, deaths);
        sprintf(data + strlen(data), "Duel Wins/Loses: %i/%i \n", duel_wins, duel_loses);
        sprintf(data + strlen(data), "Flag Captures: %i \n", flag_captures);
        sprintf(data + strlen(data), "FFA Wins/Loses: %i/%i \n", ffa_wins, ffa_loses);
        sprintf(data + strlen(data), "TDM Wins/Loses: %i/%i \n", tdm_wins, tdm_loses);
        sprintf(data + strlen(data), "CTF Wins/Loses: %i/%i \n", ctf_wins, ctf_loses);
        sprintf(data + strlen(data), "Siege Wins/Loses: %i/%i \n", siege_wins, siege_loses);
        return data;
    }
    else
        return "ERROR CANT OPEN DB FILE";
}

char * mysqlGetLeaders(char * stmt) {
    char newStmt[512];
    strcpy(newStmt, stmt);

    char *data = malloc(1024);
    sprintf(data, "CURRENT GAMETYPE LEADERBOARD \n");

    char * pch;
    int i = 0;
    //printf("[DEBUG] Splitting string \"%s\" into tokens:\n", stmt);
    pch = strtok(newStmt, ",");
    while (pch != NULL)
    {
        sprintf(data + strlen(data), "%s", pch);
        if (i == 0)
            sprintf(data + strlen(data), ": ");
        if (i == 1)
            sprintf(data + strlen(data), "/");
        //printf("%s\n", pch);
        i++;
        if (i == 2)
            i = 0;
        pch = strtok(NULL, ",");
    }
    strcpy(data, replace_str(data, ";", "\n"));
    data[strlen(data) - 2] = 0;
    return data;
}

char *sqliteGetLeaders(char *SQLStmnt, ...) {
    sqlite3_stmt *stmt;
    sqlite3		*db;
    char		*zErrMsg = 0;

    if (sqlite3_open("clanmod.db", &db) == SQLITE_OK)
    {
        int rc = sqlite3_prepare_v2(db, SQLStmnt, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
            fprintf(stderr, "SQL error: %s\n", zErrMsg);

        char *data = malloc(1024);
        sprintf(data, "CURRENT GAMETYPE LEADERBOARD \n");
        do {
            rc = sqlite3_step(stmt);

            if (rc == SQLITE_ROW) { /* can read data */
                sprintf(data + strlen(data), "%s: %i/%i \n", sqliteGetName("SELECT * FROM users WHERE id = '%i'",sqlite3_column_int(stmt, 0)), sqlite3_column_int(stmt, 1), sqlite3_column_int(stmt, 2));
            }
        } while (rc == SQLITE_ROW);

        sqlite3_finalize(stmt);

        return data;
    }
    else
        return "ERROR CANT OPEN DB FILE";
}

/*
   ==================
   DeathmatchScoreboardMessage

   ==================
 */
void DeathmatchScoreboardMessage( gentity_t *ent ) {
    char		entry[1024];
    char		string[1400];
    int			stringlength;
    int			i, j;
    gclient_t	*cl;
    int			numSorted, scoreFlags, accuracy, perfect;

    // send the latest information on all clients
    string[0] = 0;
    stringlength = 0;
    scoreFlags = 0;

    numSorted = level.numConnectedClients;

    if (numSorted > MAX_CLIENT_SCORE_SEND)
        numSorted = MAX_CLIENT_SCORE_SEND;

    for (i=0 ; i < numSorted ; i++) {
        int		ping;

        cl = &level.clients[level.sortedClients[i]];

        if ( cl->pers.connected == CON_CONNECTING ) {
            ping = -1;
            //[BotTweaks] 
        }
        else if ( (g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) && roar_BotFakePings.integer == 1 )
        {//make fake pings for bots.
            ping = Q_irand(40, 75);
            //[/BotTweaks]
        } 
        else {
            ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
        }

        if( cl->accuracy_shots ) {
            accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
        }
        else {
            accuracy = 0;
        }
        perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

        Com_sprintf (entry, sizeof(entry),
                " %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
                cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
                scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy, 
                cl->ps.persistant[PERS_MONEY],
                cl->ps.persistant[PERS_EXPERIENCE],
                cl->ps.persistant[PERS_ICON], 
                cl->ps.persistant[PERS_FULLGOD], 
                cl->ps.persistant[PERS_SKILL_EARNED], 
                perfect,
                cl->ps.persistant[PERS_REGEN]);
        j = strlen(entry);
        if (stringlength + j > 1022)
            break;
        strcpy (string + stringlength, entry);
        stringlength += j;
    }

    //still want to know the total # of clients
    i = level.numConnectedClients;

    trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
                level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
                string ) );
}


/*
   ==================
   Cmd_Score_f

   Request current scoreboard information
   ==================
 */
void Cmd_Score_f( gentity_t *ent ) {
    DeathmatchScoreboardMessage( ent );
}

/* 
   ================== 
   ClanSayOk 

   Admin commands
   ================== 
 */ 
qboolean   ClanSayOk( gentity_t *ent ) {
    if (ent->r.svFlags & SVF_CLANSAY)
        return qtrue; 
    else
        return qfalse;
}



/*
   ==================
   CheatsOk
   ==================
 */
qboolean	CheatsOk( gentity_t *ent ) {
    if ( !g_cheats.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
        return qfalse;
    }
    if ( ent->health <= 0 ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
        return qfalse;
    }
    return qtrue;
}


/*
   ==================
   ConcatArgs
   ==================
 */
char	*ConcatArgs( int start ) {
    int		i, c, tlen;
    static char	line[MAX_STRING_CHARS];
    int		len;
    char	arg[MAX_STRING_CHARS];

    len = 0;
    c = trap_Argc();
    for ( i = start ; i < c ; i++ ) {
        trap_Argv( i, arg, sizeof( arg ) );
        tlen = strlen( arg );
        if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
            break;
        }
        memcpy( line + len, arg, tlen );
        len += tlen;
        if ( i != c - 1 ) {
            line[len] = ' ';
            len++;
        }
    }

    line[len] = 0;

    return line;
}

/*
   ==================
   SanitizeString

   Remove case and control characters
   ==================
 */
void SanitizeString( char *in, char *out ) {
    while ( *in ) {
        if ( *in == 27 ) {
            in += 2;		// skip color code
            continue;
        }
        if ( *in < 32 ) {
            in++;
            continue;
        }
        *out++ = tolower( (unsigned char) *in++ );
    }

    *out = 0;
}

/*
   ==================
   ClientNumberFromString

   Returns a player number for either a number or name string
   Returns -1 if invalid
   ==================
 */
int ClientNumberFromString( gentity_t *to, char *s ) {
    gclient_t	*cl;
    int			idnum;
    char		s2[MAX_STRING_CHARS];
    char		n2[MAX_STRING_CHARS];

    // numeric values are just slot numbers
    if (s[0] >= '0' && s[0] <= '9') {
        idnum = atoi( s );
        if ( idnum < 0 || idnum >= level.maxclients ) {
            trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
            return -1;
        }

        cl = &level.clients[idnum];
        if ( cl->pers.connected != CON_CONNECTED ) {
            trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
            return -1;
        }
        return idnum;
    }

    // check for a name match
    SanitizeString( s, s2 );
    for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
        if ( cl->pers.connected != CON_CONNECTED ) {
            continue;
        }
        SanitizeString( cl->pers.netname, n2 );
        if ( !strcmp( n2, s2 ) ) {
            return idnum;
        }
    }

    trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
    return -1;
}

/*
   ==================
   Cmd_Give_f

   Give items to a client
   ==================
 */
void Cmd_Give_f (gentity_t *cmdent, int baseArg)
{
    char		name[MAX_TOKEN_CHARS];
    gentity_t	*ent;
    gitem_t		*it;
    int			i;
    qboolean	give_all;
    gentity_t		*it_ent;
    trace_t		trace;
    char		arg[MAX_TOKEN_CHARS];

    if ( !CheatsOk( cmdent ) ) {
        return;
    }

    if (baseArg)
    {
        char otherindex[MAX_TOKEN_CHARS];

        trap_Argv( 1, otherindex, sizeof( otherindex ) );

        if (!otherindex[0])
        {
            Com_Printf("giveother requires that the second argument be a client index number.\n");
            return;
        }

        i = atoi(otherindex);

        if (i < 0 || i >= MAX_CLIENTS)
        {
            Com_Printf("%i is not a client index\n", i);
            return;
        }

        ent = &g_entities[i];

        if (!ent->inuse || !ent->client)
        {
            Com_Printf("%i is not an active client\n", i);
            return;
        }
    }
    else
    {
        ent = cmdent;
    }

    trap_Argv( 1+baseArg, name, sizeof( name ) );

    if (Q_stricmp(name, "all") == 0)
        give_all = qtrue;
    else
        give_all = qfalse;

    if (give_all)
    {
        i = 0;
        while (i < HI_NUM_HOLDABLE)
        {
            ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
            i++;
        }
        i = 0;
    }

    if (give_all || Q_stricmp( name, "health") == 0)
    {
        if (trap_Argc() == 3+baseArg) {
            trap_Argv( 2+baseArg, arg, sizeof( arg ) );
            ent->health = atoi(arg);
            if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
                ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
            }
        }
        else {
            ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0)
    {
        ent->client->ps.stats[STAT_WEAPONS] = (1 << (LAST_USEABLE_WEAPON+1))  - ( 1 << WP_NONE );
        if (!give_all)
            return;
    }

    if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
    {
        trap_Argv( 2+baseArg, arg, sizeof( arg ) );
        ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
        return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0)
    {
        int num = 999;
        if (trap_Argc() == 3+baseArg) {
            trap_Argv( 2+baseArg, arg, sizeof( arg ) );
            num = atoi(arg);
        }
        for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
            ent->client->ps.ammo[i] = num;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "armor") == 0)
    {
        if (trap_Argc() == 3+baseArg) {
            trap_Argv( 2+baseArg, arg, sizeof( arg ) );
            ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
        } else {
            ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
        }

        if (!give_all)
            return;
    }

    /* excellent and impressive awards no longer exist and not Cmd_Give_f friendly (-at-)
       if (Q_stricmp(name, "excellent") == 0) {
       ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
       return;
       }
       if (Q_stricmp(name, "impressive") == 0) {
       ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
       return;
       }
       if (Q_stricmp(name, "gauntletaward") == 0) {
       ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
       return;
       }
       if (Q_stricmp(name, "defend") == 0) {
       ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
       return;
       }
       if (Q_stricmp(name, "assist") == 0) {
       ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
       return;
       }*/

    // spawn a specific item right on the player
    // Cmd_Give_f money <amount> (-at-)


    // spawn a specific item right on the player
    if ( !give_all ) {
        it = BG_FindItem (name);
        if (!it) {
            return;
        }

        it_ent = G_Spawn();
        VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
        it_ent->classname = it->classname;
        G_SpawnItem (it_ent, it);
        FinishSpawningItem(it_ent );
        memset( &trace, 0, sizeof( trace ) );
        Touch_Item (it_ent, ent, &trace);
        if (it_ent->inuse) {
            G_FreeEntity( it_ent );
        }
    }
}

/*
   ==================
   Cmd_God_f

   Sets client to godmode

   argv(0) god
   ==================
 */
void Cmd_God_f (gentity_t *ent)
{
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE) )
        msg = "godmode OFF\n";
    else
        msg = "godmode ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}

/*
   ==================
   Cmd_God_f

   Sets client to godmode

   argv(0) god
   ==================
 */
void Cmd_Undying_f (gentity_t *ent)
{
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_UNDYING;
    if (!(ent->flags & FL_UNDYING) )
        msg = "undying OFF\n";
    else
        msg = "undying ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
   ==================
   Cmd_Notarget_f

   Sets client to notarget

   argv(0) notarget
   ==================
 */
void Cmd_Notarget_f( gentity_t *ent ) {
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_NOTARGET;
    if (!(ent->flags & FL_NOTARGET) )
        msg = "notarget OFF\n";
    else
        msg = "notarget ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
   ==================
   Cmd_Noclip_f

   argv(0) noclip
   ==================
 */
void Cmd_Noclip_f( gentity_t *ent ) {
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    if ( ent->client->noclip ) {
        msg = "noclip OFF\n";
    } else {
        msg = "noclip ON\n";
    }
    ent->client->noclip = !ent->client->noclip;

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
   ==================
   Cmd_LevelShot_f

   This is just to help generate the level pictures
   for the menus.  It goes to the intermission immediately
   and sends over a command to the client to resize the view,
   hide the scoreboard, and take a special screenshot
   ==================
 */
void Cmd_LevelShot_f( gentity_t *ent ) {
    if ( !CheatsOk( ent ) ) {
        return;
    }

    // doesn't work in single player
    if ( g_gametype.integer != 0 ) {
        trap_SendServerCommand( ent-g_entities, 
                "print \"Must be in g_gametype 0 for levelshot\n\"" );
        return;
    }

    BeginIntermission();
    trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
   ==================
   Cmd_TeamTask_f

   From TA.
   ==================
 */
void Cmd_TeamTask_f( gentity_t *ent ) {
    char userinfo[MAX_INFO_STRING];
    char		arg[MAX_TOKEN_CHARS];
    int task;
    int client = ent->client - level.clients;

    if ( trap_Argc() != 2 ) {
        return;
    }
    trap_Argv( 1, arg, sizeof( arg ) );
    task = atoi( arg );

    trap_GetUserinfo(client, userinfo, sizeof(userinfo));
    Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
    trap_SetUserinfo(client, userinfo);
    ClientUserinfoChanged(client);
}



/*
   =================
   Cmd_Kill_f
   =================
 */
void Cmd_Kill_f( gentity_t *ent ) {
    //[BugFix41]
    //if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->tempSpectate >= level.time ) {
        //[/BugFix41]
        return;
    }

    //RoAR mod NOTE: I just added these due to a post. http://clanmod.org/forums/viewtopic.php?f=29&t=245
    if (ent->client->pers.amsleep == qtrue || ent->client->pers.amsplat == qtrue ||
            ent->client->pers.ampunish == qtrue || ent->client->pers.amfreeze == qtrue){
        return;
    }

    if (ent->health <= 0) {
        return;
    }

    if (dueltypes[ent->client->ps.clientNum] == 2 && ent->client->ps.duelInProgress)
    {
        return;
    }

    if ((g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) &&
            level.numPlayingClients > 1 && !level.warmupTime)
    {
        if (!g_allowDuelSuicide.integer)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "ATTEMPTDUELKILL")) );
            return;
        }
    }

    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
    player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
    gclient_t *wCl;
    int i;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        wCl = &level.clients[i];

        if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
                wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
        {
            return &g_entities[wCl->ps.clientNum];
        }
    }

    return NULL;
}

/*
   =================
   BroadCastTeamChange

   Let everyone know about a team change
   =================
 */
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
    client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

    if (g_gametype.integer == GT_SIEGE)
    { //don't announce these things in siege
        return;
    }

    if ( client->sess.sessionTeam == TEAM_RED ) {
        trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
                    client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEREDTEAM")) );
    } else if ( client->sess.sessionTeam == TEAM_BLUE ) {
        trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
                    client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBLUETEAM")));
    } else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
        trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
                    client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));
    } else if ( client->sess.sessionTeam == TEAM_FREE ) {
        if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
        {
            /*
               gentity_t *currentWinner = G_GetDuelWinner(client);

               if (currentWinner && currentWinner->client)
               {
               trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
               currentWinner->client->pers.netname, G_GetStringEdString("MP_SVGAME", "VERSUS"), client->pers.netname));
               }
               else
               {
               trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
               client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
               }
             */
            //NOTE: Just doing a vs. once it counts two players up
        }
        else
        {
            trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
                        client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
        }
    }

    G_LogPrintf ( "setteam:  %i %s %s\n",
            client - &level.clients[0],
            TeamName ( oldTeam ),
            TeamName ( client->sess.sessionTeam ) );
}

qboolean G_PowerDuelCheckFail(gentity_t *ent)
{
    int			loners = 0;
    int			doubles = 0;

    if (!ent->client || ent->client->sess.duelTeam == DUELTEAM_FREE)
    {
        return qtrue;
    }

    G_PowerDuelCount(&loners, &doubles, qfalse);

    if (ent->client->sess.duelTeam == DUELTEAM_LONE && loners >= 1)
    {
        return qtrue;
    }

    if (ent->client->sess.duelTeam == DUELTEAM_DOUBLE && doubles >= 2)
    {
        return qtrue;
    }

    return qfalse;
}

/*
   =================
   SetTeam
   =================
 */
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) {
    int					team, oldTeam;
    gclient_t			*client;
    int					clientNum;
    spectatorState_t	specState;
    int					specClient;
    int					teamLeader;

    //
    // see what change is requested
    //
    client = ent->client;

    clientNum = client - level.clients;
    specClient = 0;
    specState = SPECTATOR_NOT;
    if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_SCOREBOARD;
    } else if ( !Q_stricmp( s, "follow1" ) ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FOLLOW;
        specClient = -1;
    } else if ( !Q_stricmp( s, "follow2" ) ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FOLLOW;
        specClient = -2;
    } else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" )  || !Q_stricmp( s, "spectate" ) ) {
        if(level.isLockedspec ){
            trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^7The ^3Spectator ^7Access has been locked!\n\""));
            trap_SendServerCommand( ent->client->ps.clientNum, va("cp \"^7Sorry, the ^3Spectator ^7Access is locked.\""));
            return;
        }
        else{
            team = TEAM_SPECTATOR;
            specState = SPECTATOR_FREE;
        }
    } else if ( g_gametype.integer >= GT_TEAM ) {
        // if running a team game, assign player to one of the teams
        specState = SPECTATOR_NOT;
        if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
            if(level.isLockedred){
                trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^7The ^1Red ^7team is locked!\n\""));
                trap_SendServerCommand( ent->client->ps.clientNum, va("cp \"^7Sorry, the ^1Red ^7Team has been locked.\""));
                return;
            }
            else{
                team = TEAM_RED;
            }
        } else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
            if(level.isLockedblue){
                trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^7The ^4Blue ^7team is locked!\n\""));
                trap_SendServerCommand( ent->client->ps.clientNum, va("cp \"^7Sorry, the ^4Blue ^7Team has been locked.\""));
                return;
            }
            else{
                team = TEAM_BLUE;
            }
        } else {
            // pick the team with the least number of players
            //For now, don't do this. The legalize function will set powers properly now.
            /*
               if (g_forceBasedTeams.integer)
               {
               if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
               {
               team = TEAM_BLUE;
               }
               else
               {
               team = TEAM_RED;
               }
               }
               else
               {
             */
            team = PickTeam( clientNum );
            //}
        }

        if ( g_teamForceBalance.integer && !g_trueJedi.integer ) {
            int		counts[TEAM_NUM_TEAMS];

            counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
            counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

            // We allow a spread of two
            if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
                //For now, don't do this. The legalize function will set powers properly now.
                /*
                   if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
                   {
                   trap_SendServerCommand( ent->client->ps.clientNum, 
                   va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED_SWITCH")) );
                   }
                   else
                 */
                {
                    trap_SendServerCommand( ent->client->ps.clientNum, 
                            va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
                }
                return; // ignore the request
            }
            if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
                //For now, don't do this. The legalize function will set powers properly now.
                /*
                   if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
                   {
                   trap_SendServerCommand( ent->client->ps.clientNum, 
                   va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE_SWITCH")) );
                   }
                   else
                 */
                {
                    trap_SendServerCommand( ent->client->ps.clientNum, 
                            va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
                }
                return; // ignore the request
            }

            // It's ok, the team we are switching to has less or same number of players
        }

        //For now, don't do this. The legalize function will set powers properly now.
        /*
           if (g_forceBasedTeams.integer)
           {
           if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
           {
           trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBELIGHT")) );
           return;
           }
           if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
           {
           trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEDARK")) );
           return;
           }
           }
         */

    } else {
        // force them to spectators if there aren't any spots free
        if(level.isLockedjoin){
            trap_SendServerCommand( ent->client->ps.clientNum, va("print \"^7The ^2Join ^7team is locked!\n\""));
            trap_SendServerCommand( ent->client->ps.clientNum, va("cp \"^7Sorry, the ^2Join ^7Team has been locked.\""));
            return;
        }
        else{
            team = TEAM_FREE;
        }
    }

    //[BugFix41]
    oldTeam = client->sess.sessionTeam;
    //[/BugFix41]

    if (g_gametype.integer == GT_SIEGE)
    {
        if (client->tempSpectate >= level.time &&
                team == TEAM_SPECTATOR)
        { //sorry, can't do that.
            return;
        }

        //[BugFix41]
        if ( team == oldTeam && team != TEAM_SPECTATOR ) {
            return;
        }
        //[/BugFix41]

        client->sess.siegeDesiredTeam = team;
        //oh well, just let them go.
        /*
           if (team != TEAM_SPECTATOR)
           { //can't switch to anything in siege unless you want to switch to being a fulltime spectator
        //fill them in on their objectives for this team now
        trap_SendServerCommand(ent-g_entities, va("sb %i", client->sess.siegeDesiredTeam));

        trap_SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time the round begins.\n\"") );
        return;
        }
         */
        if (client->sess.sessionTeam != TEAM_SPECTATOR &&
                team != TEAM_SPECTATOR)
        { //not a spectator now, and not switching to spec, so you have to wait til you die.
            //trap_SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time you respawn.\n\"") );
            qboolean doBegin;
            if (ent->client->tempSpectate >= level.time)
            {
                doBegin = qfalse;
            }
            else
            {
                doBegin = qtrue;
            }

            if (doBegin)
            {
                // Kill them so they automatically respawn in the team they wanted.
                if (ent->health > 0)
                {
                    ent->flags &= ~FL_GODMODE;
                    ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
                    player_die( ent, ent, ent, 100000, MOD_TEAM_CHANGE ); 
                }
            }

            if (ent->client->sess.sessionTeam != ent->client->sess.siegeDesiredTeam)
            {
                SetTeamQuick(ent, ent->client->sess.siegeDesiredTeam, qfalse);
            }

            return;
        }
    }

    // override decision if limiting the players
    if ( (g_gametype.integer == GT_DUEL)
            && level.numNonSpectatorClients >= 2 )
    {
        team = TEAM_SPECTATOR;
    }
    else if ( (g_gametype.integer == GT_POWERDUEL)
            && (level.numPlayingClients >= 3 || G_PowerDuelCheckFail(ent)) )
    {
        team = TEAM_SPECTATOR;
    }
    else if ( g_maxGameClients.integer > 0 && 
            level.numNonSpectatorClients >= g_maxGameClients.integer )
    {
        team = TEAM_SPECTATOR;
    }

    //
    // decide if we will allow the change
    //
    //[BugFix41]
    // moved this up above the siege check
    //oldTeam = client->sess.sessionTeam;
    //[/BugFix41]
    if ( team == oldTeam && team != TEAM_SPECTATOR ) {
        return;
    }

    //
    // execute the team change
    //

    //If it's siege then show the mission briefing for the team you just joined.
    //	if (g_gametype.integer == GT_SIEGE && team != TEAM_SPECTATOR)
    //	{
    //		trap_SendServerCommand(clientNum, va("sb %i", team));
    //	}

    // if the player was dead leave the body
    if ( client->ps.stats[STAT_HEALTH] <= 0 && client->sess.sessionTeam != TEAM_SPECTATOR ) {
        MaintainBodyQueue(ent);
    }

    // he starts at 'base'
    client->pers.teamState.state = TEAM_BEGIN;
    if ( oldTeam != TEAM_SPECTATOR ) {
        // Kill him (makes sure he loses flags, etc)
        ent->flags &= ~FL_GODMODE;
        ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
        g_dontPenalizeTeam = qtrue;
        player_die (ent, ent, ent, 100000, MOD_SPECTATE);
        g_dontPenalizeTeam = qfalse;

    }
    // they go to the end of the line for tournements
    if ( team == TEAM_SPECTATOR ) {
        if ( (g_gametype.integer != GT_DUEL) || (oldTeam != TEAM_SPECTATOR) )	{//so you don't get dropped to the bottom of the queue for changing skins, etc.
            client->sess.spectatorTime = level.time;
        }
    }

    // hack to preserve score information around spectating sessions!
    if ( team == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR )
    {
        ent->client->pers.save_score = ent->client->ps.persistant[PERS_SCORE]; // tuck this away (lmo)
    }
    if ( team != TEAM_SPECTATOR && oldTeam == TEAM_SPECTATOR )
    { // when oldteam is spectator, take care to restore old score info!
        // this is potentially changed during a specator/follow! (see SpectatorClientEndFrame)
        if (roar_spectate_keep_score.integer >= 1 && g_gametype.integer != GT_DUEL
                && g_gametype.integer != GT_POWERDUEL && g_gametype.integer != GT_SIEGE){
            ent->client->ps.persistant[PERS_SCORE] = ent->client->pers.save_score; // restore!
        }
        else {
            if (g_gametype.integer != GT_DUEL && g_gametype.integer != GT_POWERDUEL && ent->s.eType != ET_NPC){
                if (roar_spectate_keep_score.integer >= 0){
                    ent->client->sess.losses = 0;
                }
                ent->client->sess.wins = 0;
            }
        }
    }

    client->sess.sessionTeam = team;
    client->sess.spectatorState = specState;
    client->sess.spectatorClient = specClient;

    client->sess.teamLeader = qfalse;
    if ( team == TEAM_RED || team == TEAM_BLUE ) {
        teamLeader = TeamLeader( team );
        // if there is no team leader or the team leader is a bot and this client is not a bot
        if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
            //SetLeader( team, clientNum );
        }
    }
    // make sure there is a team leader on the team the player came from
    if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
        CheckTeamLeader( oldTeam );
    }

    BroadcastTeamChange( client, oldTeam );

    //make a disappearing effect where they were before teleporting them to the appropriate spawn point,
    //if we were not on the spec team
    if (oldTeam != TEAM_SPECTATOR)
    {
        gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
        tent->s.clientNum = clientNum;
    }

    // get and distribute relevent paramters
    ClientUserinfoChanged( clientNum );

    if (!g_preventTeamBegin)
    {
        ClientBegin( clientNum, qfalse );
    }
}

/*
   =================
   StopFollowing

   If the client being followed leaves the game, or you just want to drop
   to free floating spectator mode
   =================
 */
//[BugFix38]
extern void G_LeaveVehicle( gentity_t *ent, qboolean ConCheck );
//[/BugFix38]
void StopFollowing( gentity_t *ent ) {
    ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
    ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
    ent->client->sess.spectatorState = SPECTATOR_FREE;
    ent->client->ps.pm_flags &= ~PMF_FOLLOW;
    ent->r.svFlags &= ~SVF_BOT;
    ent->client->ps.clientNum = ent-g_entities;
    ent->client->ps.weapon = WP_NONE;
    //[BugFix38]
    G_LeaveVehicle( ent, qfalse ); // clears m_iVehicleNum as well
    //ent->client->ps.m_iVehicleNum = 0;
    //[/BugFix38]
    ent->client->ps.viewangles[ROLL] = 0.0f;
    ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
    ent->client->ps.forceHandExtendTime = 0;
    ent->client->ps.zoomMode = 0;
    ent->client->ps.zoomLocked = 0;
    ent->client->ps.zoomLockTime = 0;
    ent->client->ps.legsAnim = 0;
    ent->client->ps.legsTimer = 0;
    ent->client->ps.torsoAnim = 0;
    ent->client->ps.torsoTimer = 0;
    //[DuelSys]
    ent->client->ps.duelInProgress = qfalse; // MJN - added to clean it up a bit.
    //[/DuelSys]
    //[BugFix38]
    //[OLDGAMETYPES]
    ent->client->ps.isJediMaster = qfalse; // major exploit if you are spectating somebody and they are JM and you reconnect
    //[/OLDGAMETYPES]
    ent->health = ent->client->ps.stats[STAT_HEALTH] = 100; // so that you don't keep dead angles if you were spectating a dead person
    //[/BugFix38]
}

/*
   =================
   Cmd_Team_f
   =================
 */
void Cmd_Team_f( gentity_t *ent ) {
    int			oldTeam;
    char		s[MAX_TOKEN_CHARS];

    if ( trap_Argc() != 2 ) {
        oldTeam = ent->client->sess.sessionTeam;
        switch ( oldTeam ) {
            case TEAM_BLUE:
                trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTBLUETEAM")) );
                break;
            case TEAM_RED:
                trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTREDTEAM")) );
                break;
            case TEAM_FREE:
                trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
                break;
            case TEAM_SPECTATOR:
                trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
                break;
        }
        return;
    }

    if ( ent->client->switchTeamTime > level.time ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
        return;
    }

    if (gEscaping)
    {
        return;
    }

    //RoAR mod NOTE: This was in v1.10 B2, I removed it because if someone were to change their information,
    // the server would then reset the persons losses.
    /*if (g_gametype.integer != GT_DUEL && g_gametype.integer != GT_POWERDUEL && ent->s.eType != ET_NPC){
      if (roar_spectate_keep_score.integer >= 0){
      ent->client->sess.losses = 0;
      }
      ent->client->sess.wins = 0;
      }*/

    // if they are playing a tournement game, count as a loss
    if ( g_gametype.integer == GT_DUEL
            && ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
        //disallow changing teams
        trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
        return;
        //FIXME: why should this be a loss???
        //ent->client->sess.losses++;
    }

    if (g_gametype.integer == GT_POWERDUEL)
    { //don't let clients change teams manually at all in powerduel, it will be taken care of through automated stuff
        trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Power Duel\n\"" );
        return;
    }

    trap_Argv( 1, s, sizeof( s ) );

    SetTeam( ent, s );

    ent->client->switchTeamTime = level.time + 5000;
}

/*
   =================
   Cmd_DuelTeam_f
   =================
 */
void Cmd_DuelTeam_f(gentity_t *ent)
{
    int			oldTeam;
    char		s[MAX_TOKEN_CHARS];

    if (g_gametype.integer != GT_POWERDUEL)
    { //don't bother doing anything if this is not power duel
        return;
    }

    /*
       if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
       {
       trap_SendServerCommand( ent-g_entities, va("print \"You cannot change your duel team unless you are a spectator.\n\""));
       return;
       }
     */

    if ( trap_Argc() != 2 )
    { //No arg so tell what team we're currently on.
        oldTeam = ent->client->sess.duelTeam;
        switch ( oldTeam )
        {
            case DUELTEAM_FREE:
                trap_SendServerCommand( ent-g_entities, va("print \"None\n\"") );
                break;
            case DUELTEAM_LONE:
                trap_SendServerCommand( ent-g_entities, va("print \"Single\n\"") );
                break;
            case DUELTEAM_DOUBLE:
                trap_SendServerCommand( ent-g_entities, va("print \"Double\n\"") );
                break;
            default:
                break;
        }
        return;
    }

    if ( ent->client->switchDuelTeamTime > level.time )
    { //debounce for changing
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
        return;
    }

    trap_Argv( 1, s, sizeof( s ) );

    oldTeam = ent->client->sess.duelTeam;

    if (!Q_stricmp(s, "free"))
    {
        ent->client->sess.duelTeam = DUELTEAM_FREE;
    }
    else if (!Q_stricmp(s, "single"))
    {
        ent->client->sess.duelTeam = DUELTEAM_LONE;
    }
    else if (!Q_stricmp(s, "double"))
    {
        ent->client->sess.duelTeam = DUELTEAM_DOUBLE;
    }
    else
    {
        trap_SendServerCommand( ent-g_entities, va("print \"'%s' not a valid duel team.\n\"", s) );
    }

    if (oldTeam == ent->client->sess.duelTeam)
    { //didn't actually change, so don't care.
        return;
    }

    if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
    { //ok..die
        int curTeam = ent->client->sess.duelTeam;
        ent->client->sess.duelTeam = oldTeam;
        G_Damage(ent, ent, ent, NULL, ent->client->ps.origin, 99999, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
        ent->client->sess.duelTeam = curTeam;
    }
    //reset wins and losses
    ent->client->sess.wins = 0;
    ent->client->sess.losses = 0;

    //get and distribute relevent paramters
    ClientUserinfoChanged( ent->s.number );

    ent->client->switchDuelTeamTime = level.time + 5000;
}

int G_TeamForSiegeClass(const char *clName)
{
    int i = 0;
    int team = SIEGETEAM_TEAM1;
    siegeTeam_t *stm = BG_SiegeFindThemeForTeam(team);
    siegeClass_t *scl;

    if (!stm)
    {
        return 0;
    }

    while (team <= SIEGETEAM_TEAM2)
    {
        scl = stm->classes[i];

        if (scl && scl->name[0])
        {
            if (!Q_stricmp(clName, scl->name))
            {
                return team;
            }
        }

        i++;
        if (i >= MAX_SIEGE_CLASSES || i >= stm->numClasses)
        {
            if (team == SIEGETEAM_TEAM2)
            {
                break;
            }
            team = SIEGETEAM_TEAM2;
            stm = BG_SiegeFindThemeForTeam(team);
            i = 0;
        }
    }

    return 0;
}

/*
   =================
   Cmd_SiegeClass_f
   =================
 */
void Cmd_SiegeClass_f( gentity_t *ent )
{
    char className[64];
    int team = 0;
    int preScore;
    qboolean startedAsSpec = qfalse;

    if (g_gametype.integer != GT_SIEGE)
    { //classes are only valid for this gametype
        return;
    }

    if (!ent->client)
    {
        return;
    }

    if (trap_Argc() < 1)
    {
        return;
    }

    if ( ent->client->switchClassTime > level.time )
    {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSSWITCH")) );
        return;
    }

    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
    {
        startedAsSpec = qtrue;
    }

    trap_Argv( 1, className, sizeof( className ) );

    team = G_TeamForSiegeClass(className);

    if (!team)
    { //not a valid class name
        return;
    }

    if (ent->client->sess.sessionTeam != team)
    { //try changing it then
        g_preventTeamBegin = qtrue;
        if (team == TEAM_RED)
        {
            SetTeam(ent, "red");
        }
        else if (team == TEAM_BLUE)
        {
            SetTeam(ent, "blue");
        }
        g_preventTeamBegin = qfalse;

        if (ent->client->sess.sessionTeam != team)
        { //failed, oh well
            if (ent->client->sess.sessionTeam != TEAM_SPECTATOR ||
                    ent->client->sess.siegeDesiredTeam != team)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSTEAM")) );
                return;
            }
        }
    }

    //preserve 'is score
    preScore = ent->client->ps.persistant[PERS_SCORE];

    //Make sure the class is valid for the team
    BG_SiegeCheckClassLegality(team, className);

    //Set the session data
    strcpy(ent->client->sess.siegeClass, className);

    // get and distribute relevent paramters
    ClientUserinfoChanged( ent->s.number );

    if (ent->client->tempSpectate < level.time)
    {
        // Kill him (makes sure he loses flags, etc)
        if (ent->health > 0 && !startedAsSpec)
        {
            ent->flags &= ~FL_GODMODE;
            ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
            player_die (ent, ent, ent, 100000, MOD_SUICIDE);
        }

        if (ent->client->sess.sessionTeam == TEAM_SPECTATOR || startedAsSpec)
        { //respawn them instantly.
            ClientBegin( ent->s.number, qfalse );
        }
    }
    //set it back after we do all the stuff
    ent->client->ps.persistant[PERS_SCORE] = preScore;

    ent->client->switchClassTime = level.time + 5000;
}

/*
   =================
   Cmd_ForceChanged_f
   =================
 */
void Cmd_ForceChanged_f( gentity_t *ent )
{
    char fpChStr[1024];
    const char *buf;
    //	Cmd_Kill_f(ent);
    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
    { //if it's a spec, just make the changes now
        //trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "FORCEAPPLIED")) );
        //No longer print it, as the UI calls this a lot.
        WP_InitForcePowers( ent );
        goto argCheck;
    }

    buf = G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED");

    strcpy(fpChStr, buf);

    trap_SendServerCommand( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, fpChStr) );

    ent->client->ps.fd.forceDoInit = 1;
argCheck:
    if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
    { //If this is duel, don't even bother changing team in relation to this.
        return;
    }

    if (trap_Argc() > 1)
    {
        char	arg[MAX_TOKEN_CHARS];

        trap_Argv( 1, arg, sizeof( arg ) );

        if (arg && arg[0])
        { //if there's an arg, assume it's a combo team command from the UI.
            Cmd_Team_f(ent);
        }
    }
}

extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride)
{
    char truncSaberName[64];
    int i = 0;

    if (!siegeOverride &&
            g_gametype.integer == GT_SIEGE &&
            ent->client->siegeClass != -1 &&
            (
             bgSiegeClasses[ent->client->siegeClass].saberStance ||
             bgSiegeClasses[ent->client->siegeClass].saber1[0] ||
             bgSiegeClasses[ent->client->siegeClass].saber2[0]
            ))
    { //don't let it be changed if the siege class has forced any saber-related things
        return qfalse;
    }

    while (saberName[i] && i < 64-1)
    {
        truncSaberName[i] = saberName[i];
        i++;
    }
    truncSaberName[i] = 0;

    if ( saberNum == 0 && (Q_stricmp( "none", truncSaberName ) == 0 || Q_stricmp( "remove", truncSaberName ) == 0) )
    { //can't remove saber 0 like this
        strcpy(truncSaberName, "Kyle");
    }

    //Set the saber with the arg given. If the arg is
    //not a valid sabername defaults will be used.
    WP_SetSaber(ent->s.number, ent->client->saber, saberNum, truncSaberName);

    if (!ent->client->saber[0].model[0])
    {
        assert(0); //should never happen!
        strcpy(ent->client->sess.saberType, "none");
    }
    else
    {
        strcpy(ent->client->sess.saberType, ent->client->saber[0].name);
    }

    if (!ent->client->saber[1].model[0])
    {
        strcpy(ent->client->sess.saber2Type, "none");
    }
    else
    {
        strcpy(ent->client->sess.saber2Type, ent->client->saber[1].name);
    }

    if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
    {
        WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
        ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
    }

    return qtrue;
}

/*
   =================
   Cmd_Follow_f
   =================
 */
void Cmd_Follow_f( gentity_t *ent ) {
    int		i;
    char	arg[MAX_TOKEN_CHARS];

    if ( trap_Argc() != 2 ) {
        if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
            StopFollowing( ent );
        }
        return;
    }

    trap_Argv( 1, arg, sizeof( arg ) );
    i = ClientNumberFromString( ent, arg );
    if ( i == -1 ) {
        return;
    }

    // can't follow self
    if ( &level.clients[ i ] == ent->client ) {
        return;
    }

    // can't follow another spectator
    if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
        return;
    }

    // Fix: ensiform
    if ( level.clients[ i ].tempSpectate >= level.time ) {
        return;
    }

    // if they are playing a tournement game, count as a loss
    if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
            && ent->client->sess.sessionTeam == TEAM_FREE ) {
        //WTF???
        ent->client->sess.losses++;
    }

    // first set them to spectator
    if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
        SetTeam( ent, "spectator" );
    }

    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    ent->client->sess.spectatorClient = i;
}

/*
   =================
   Cmd_FollowCycle_f
   =================
 */
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
    int		clientnum;
    int		original;

    // if they are playing a tournement game, count as a loss
    if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
            && ent->client->sess.sessionTeam == TEAM_FREE ) {
        //WTF???
        ent->client->sess.losses++;
    }
    // first set them to spectator
    if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
        SetTeam( ent, "spectator" );
    }

    if ( dir != 1 && dir != -1 ) {
        G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
    }

    clientnum = ent->client->sess.spectatorClient;
    original = clientnum;
    do {
        clientnum += dir;
        if ( clientnum >= level.maxclients ) {
            clientnum = 0;
        }
        if ( clientnum < 0 ) {
            clientnum = level.maxclients - 1;
        }

        // can only follow connected clients
        if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
            continue;
        }

        // can't follow another spectator
        if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
            continue;
        }

        // Fix: ensiform
        if ( level.clients[ clientnum ].tempSpectate >= level.time ) {
            continue;
        }

        // this is good, we can use it
        ent->client->sess.spectatorClient = clientnum;
        ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
        return;
    } while ( clientnum != original );

    // leave it where it was
}


/*
   ==================
   G_Say
   ==================
 */

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg )
{
    if (strlen(message) > MAX_SAY_TEXT) {
        return;
    }

    if (!other) {
        return;
    }
    if (!other->inuse) {
        return;
    }
    if (!other->client) {
        return;
    }
    if ( other->client->pers.connected != CON_CONNECTED ) {
        return;
    }
    if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
        return;
    }
    if ( mode == SAY_ADMIN && !other->client->pers.iamanadmin ) {
        return;
    }
    if ( mode == SAY_REPORT && !other->client->pers.iamanadmin ) {
        return;
    }
    if ( mode == SAY_CLAN && !other->client->pers.iamclan ) {
        return;
    }
    /*
    // no chatting to players in tournements
    if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
    && other->client->sess.sessionTeam == TEAM_FREE
    && ent->client->sess.sessionTeam != TEAM_FREE ) {
//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
return;
}
     */
//They've requested I take this out.

if (g_gametype.integer == GT_SIEGE &&
        ent->client && (ent->client->tempSpectate >= level.time || ent->client->sess.sessionTeam == TEAM_SPECTATOR) &&
        other->client->sess.sessionTeam != TEAM_SPECTATOR &&
        other->client->tempSpectate < level.time)
{ //siege temp spectators should not communicate to ingame players
    return;
}

// MJN - Ignoring Someone?
if ( G_IsClientChatIgnored ( other->s.number, ent->s.number ) )
{
    return;
}

if (locMsg)
{
    trap_SendServerCommand( other-g_entities, va("%s \"%s\" \"%s\" \"%c\" \"%s\"", 
                mode == SAY_TEAM || mode == SAY_ADMIN || mode == SAY_CLAN ? "ltchat" : "lchat",
                name, locMsg, color, message));
}
else
{
    trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"", 
                mode == SAY_TEAM || mode == SAY_ADMIN || mode == SAY_CLAN ? "tchat" : "chat",
                name, Q_COLOR_ESCAPE, color, message));
}
}
#define EC		"\x19"

char *strsep(char **stringp, const char *delim) {
    if (*stringp == NULL) { return NULL; }
    char *token_start = *stringp;
    *stringp = strpbrk(token_start, delim);
    if (*stringp) {
        **stringp = '\0';
        (*stringp)++;
    }
    return token_start;
}

void cmfreezeMOTD(gentity_t *ent) {
    trap_SendServerCommand(ent-g_entities, va("print \"^7Freezing MOTD... ^3(TYPE IN !hidemotd OR /hidemotd TO HIDE IT!)\n\""));
    strcpy(ent->client->csMessage, G_NewString(va("^0*^1%s^0*\n^7%s", GAMEVERSION, roar_motd_line.string)));
    ent->client->csTimeLeft = Q3_INFINITE;
}

void showMOTD(gentity_t *ent) {
    trap_SendServerCommand(ent-g_entities, va("print \"^7Showing MOTD...\n\""));
    strcpy(ent->client->csMessage, G_NewString(va("^0*^1%s^0*\n^7%s", GAMEVERSION, roar_motd_line.string)));
    ent->client->csTimeLeft = x_cstime.integer;
}

void hideMOTD(gentity_t *ent) {
    trap_SendServerCommand(ent-g_entities, va("print \"^7Hiding MOTD...\n\""));
    ent->client->csTimeLeft = 0;
    strcpy(ent->client->csMessage, G_NewString(va(" \n")));
}

void cmStats(gentity_t *ent, const char *user) { //MYSQL NEEDS TESTING
    int client_id = ent->client->ps.clientNum;
    char clean_name[MAX_NETNAME];

    if (cm_database.integer <= 0) {
        trap_SendServerCommand(client_id, va("print \"^1Database commands have been disabled on this server.\n\""));
        return;
    }

    if (ent->client && ent->client->mysqlDebounceTime > level.time)
    {
        if (ent->client->mysqlDebounceTime > 0)
            trap_SendServerCommand(ent->s.number, va("print \"^2wait %.2f seconds before trying again.\n\"", ((float)5000 / (float)1000)));
        return;
    }
    else
        ent->client->mysqlDebounceTime = level.time + 5000;

    if ((user != NULL) && (user[0] == '\0')) { //no user
        if (ent->client->pers.userID > 0) {
            if (cm_database.integer == 1) {
                trap_SendServerCommand(client_id, va("print \"^3===^1YOUR PLAYER STATUS^3===\n\n%s\n\"", sqliteGetStats("SELECT * FROM stats WHERE user_id = '%i'", ent->client->pers.userID)));
                strcpy(ent->client->csMessage, G_NewString(va("^3===^1YOUR PLAYER STATUS^3===\n\n%s\n\"", sqliteGetStats("SELECT * FROM stats WHERE user_id = '%i'", ent->client->pers.userID))));
            }
            else if (cm_database.integer == 2)
                parse_output(ent, va("mysqlUserStats curl --data \"key=%s&p=stats&g=jedi_academy&id=%i\" %s", cm_mysql_secret.string, ent->client->pers.userID, cm_mysql_url.string));
            ent->client->csTimeLeft = 10;
        }
        else
            trap_SendServerCommand(client_id, va("print \"^1Login first using the /cmlogin command.\n\""));
    }
    else {
        int userID = sqliteSelectUserID("SELECT * FROM users WHERE user = '%s'", user);

        if (cm_database.integer == 1) {
            Q_strncpyz(clean_name, g_entities[client_id].client->pers.netname, sizeof(clean_name));
            Q_CleanStr(clean_name);
            userID = sqliteSelectUserID("SELECT * FROM users WHERE user = '%s'", clean_name);
            if (userID <= 0)
                trap_SendServerCommand(client_id, va("print \"^1Player name %s not found in DB.\n\"", user));
            trap_SendServerCommand(client_id, va("print \"^3===^1PLAYER %s ^1STATUS^3===\n%s\n\"", user, sqliteGetStats("SELECT * FROM stats WHERE user_id = '%i'", userID)));
        }
        else if (cm_database.integer == 2)
            parse_output(ent, va("mysqlUserStats curl --data \"key=%s&p=stats&g=jedi_academy&id=%i\" %s", cm_mysql_secret.string, ent->client->pers.userID, cm_mysql_url.string));
        ent->client->csTimeLeft = 10;		
    }
}

void cmJetpack(gentity_t *ent) {
    if (g_gametype.integer == GT_CTY || g_gametype.integer == GT_SIEGE
            || g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) {
        trap_SendServerCommand(ent-g_entities, va("print \"Cannot use this command in this gametype.\n\""));
        return;
    }
    if (ent->client->pers.amdemigod == 1 || ent->client->pers.amhuman == 1
            || ent->client->pers.ampunish == 1 || ent->client->pers.amsleep == 1 || ent->client->pers.amfreeze == 1) {
        trap_SendServerCommand(ent-g_entities, va("print \"Cannot put on jetpack in your current state.\n\""));
        return;
    }
    if (level.modeMeeting == qtrue) {
        return;
    }
    if (ent->client->pers.amhuman == 1) {
        trap_SendServerCommand(ent-g_entities, va("print \"Monk's can't have jetpacks!\n\""));
        return;
    }
    if (roar_allow_jetpack_command.integer == 0) {
        trap_SendServerCommand(ent-g_entities, va("print \"Jetpack is disabled on this server!\n\""));
        return;
    }
    if (ent->client->ps.duelInProgress) {
        trap_SendServerCommand(ent-g_entities, va("print \"Jetpack is not allowed in duels!\n\""));
        return;
    }
    if (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_JETPACK))
    {
        ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_JETPACK);
        if (ent->client->jetPackOn)
            Jetpack_Off(ent);
    }
    else
        ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << HI_JETPACK);
}

void cmLogin(gentity_t *ent, char *pass) {
    int client_id = -1;
    char clean_name[MAX_NETNAME];

    client_id = ent->client->ps.clientNum;

    if (cm_database.integer <= 0) {
        trap_SendServerCommand(client_id, va("print \"^1Database commands have been disabled on this server.\n\""));
        return;
    }

    int userID = 0;

    //G_Printf("[DEBUG] USER:[%s] PASS:[%s] SHA:[%s]", cleanName(g_entities[client_id].client->pers.netname), pass, SHA1ThisPass(pass));
    char myname[1024];
    Q_strncpyz(clean_name, g_entities[client_id].client->pers.netname, sizeof(clean_name));
    Q_CleanStr(clean_name);
    Q_strncpyz(myname, clean_name, sizeof(myname));

    if (cm_database.integer == 1) {
        userID = sqliteSelectUserID("SELECT * FROM users WHERE user = '%s' AND pass = '%s'", myname, SHA1ThisPass(pass));

        if (userID > 0) {
            ent->client->pers.userID = userID;
            trap_SendServerCommand(client_id, va("print \"^3You are now logged in.\n\""));
        }
        else
            trap_SendServerCommand(client_id, va("print \"^1User not found\n\""));
    }
    else if (cm_database.integer == 2) {
        parse_output(ent, va("mysqlUserLogin curl --data \"key=%s&p=login&user=%s&pass=%s\" %s", cm_mysql_secret.string, myname, SHA1ThisPass(pass), cm_mysql_url.string));
    }

    if (cm_database.integer == 1) { //only sqlite needs to check if stats table exists
        int stat_id = sqliteSelectUserID("SELECT user_id FROM stats WHERE user_id = '%d'", userID);
        if (stat_id <= 0)
            sqliteRegisterUser("INSERT INTO stats (user_id) VALUES ('%d')", userID);
    }
}

void cmLeaders(gentity_t *ent) {
    if (cm_database.integer <= 0)
        trap_SendServerCommand(ent->client->ps.clientNum, va("print \"^1Database commands have been disabled on this server.\n\""));

    if (ent->client && ent->client->mysqlDebounceTime > level.time)
    {
        if (ent->client->mysqlDebounceTime > 0)
            trap_SendServerCommand(ent->s.number, va("print \"^2wait %.2f seconds before trying again.\n\"", ((float)5000 / (float)1000)));
        return;
    }
    else
        ent->client->mysqlDebounceTime = level.time + 5000;

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
        trap_SendServerCommand(ent->client->ps.clientNum, va("print \"%s\n\"", query));
        strcpy(ent->client->csMessage, G_NewString(va("%s", query)));
    }
    else if (cm_database.integer == 2)
        parse_output(ent,va("mysqlGetLeaders curl --data \"key=%s&p=leaders&g=jedi_academy&r=%s&o=%s\" %s", cm_mysql_secret.string, rows, column, cm_mysql_url.string));
    //Q_strncpyz(query, mysqlGetLeaders(parse_output(va("curl --data \"key=%s&p=leaders&g=jedi_academy&r=%s&o=%s\" %s", cm_mysql_secret.string, rows, column, cm_mysql_url.string))), sizeof(query));

    //trap_SendServerCommand(ent->client->ps.clientNum, va("print \"%s\n\"", query));
    //strcpy(ent->client->csMessage, G_NewString(va("%s", query)));
    ent->client->csTimeLeft = 5;
}

void endDuel(gentity_t *ent) {
    gentity_t *duelAgainst = &g_entities[ent->client->ps.duelIndex];
    if (dueltypes[ent->client->ps.clientNum] == 2 && ent->client->ps.duelInProgress)
    {
        ent->client->ps.duelInProgress = 0;
        duelAgainst->client->ps.duelInProgress = 0;

        G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
        G_AddEvent(duelAgainst, EV_PRIVATE_DUEL, 0);

        ent->flags &= ~FL_GODMODE;
        duelAgainst->flags &= ~FL_GODMODE;

        ent->client->ps.stats[STAT_MAX_HEALTH] = 100;
        ent->client->ps.stats[STAT_HEALTH] = ent->health = 100;
        ent->client->ps.stats[STAT_ARMOR] = 100;

        duelAgainst->client->ps.stats[STAT_MAX_HEALTH] = 100;
        duelAgainst->client->ps.stats[STAT_HEALTH] = duelAgainst->health = 100;
        duelAgainst->client->ps.stats[STAT_ARMOR] = 100;

        trap_SendServerCommand(-1, va("print \"Training duel ended by %s\n\"", ent->client->pers.netname));
        trap_SendServerCommand(-1, va("cp \"Training duel ended by\n%s\n\"", ent->client->pers.netname));
    }
    else {
        trap_SendServerCommand(ent-g_entities, va("print \"^7Must be in a training session to do this command.\n\""));
        return;
    }
}

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
    int			j;
    gentity_t	*other;
    int			color;
    char		name[64];
    // don't let text be too long for malicious reasons
    char		text[MAX_SAY_TEXT];
    char		location[64];
    char		*locMsg = NULL;

    //RoAR mod NOTE: buffer-overflow FIX!
    /*if (strlen(chatText) > MAX_SAY_TEXT) {
      return;
      }*/

    //[ChatSpamProtection]
    if(!(ent->r.svFlags & SVF_BOT))
    {//don't chat protect the bots.
        if(ent->client && ent->client->chatDebounceTime > level.time //debounce isn't up
                //and we're not bouncing our message back to our self while using SAY_TELL 
                && (mode != SAY_TELL || ent != target)) 
        {//prevent players from spamming chat.
            //Warn them.
            if(ent->client->chatDebounceTime > 0)
            {
                trap_SendServerCommand(ent->s.number, 
                        va("print \"^1please don't spam. ^2wait %.2f seconds before trying again.\n\"", 
                            ((float) 250/(float) 1000)));
            }
            return;
        }
        else //we can chat, bump the debouncer
            ent->client->chatDebounceTime = level.time + 250;
    }
    //[/ChatSpamProtection]

    switch ( mode ) {
        default:
        case SAY_ALL:
            G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );

            if (!(ent->r.svFlags & SVF_BOT))
                WebHook(ent, W_CHAT, va("%s: %s", Q_CleanStr(ent->client->pers.netname), Q_CleanStr(chatText)));

            Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
            if (roar_allow_chatColors.integer == 1){
                if (Q_stristr(ent->client->pers.chatcolor, "red")){
                    color = COLOR_RED;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "green")){
                    color = COLOR_GREEN;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "black")){
                    color = COLOR_BLACK;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "yellow")){
                    color = COLOR_YELLOW;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "blue")){
                    color = COLOR_BLUE;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "cyan")){
                    color = COLOR_CYAN;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "purple")){
                    color = COLOR_MAGENTA;
                }
                else if (Q_stristr(ent->client->pers.chatcolor, "white")){
                    color = COLOR_WHITE;
                }
                else {
                    color = COLOR_GREEN;
                }
            }
            else {
                color = COLOR_GREEN;
            }
            break;
        case SAY_ADMIN:
            if (ent->client->pers.iamanadmin != 0){
                G_LogPrintf( "sayteam: <admin>[%s]: %s\n", ent->client->pers.netname, chatText );
                Com_sprintf (name, sizeof(name), EC"^3<Admin>^7%s: ", 
                        ent->client->pers.netname );
            }
            else {
                return;
            }
            color = COLOR_YELLOW;
            break;

        case SAY_REPORT:
            if (cm_allow_report_command.integer == 1){
                G_LogPrintf( "sayteam: <report>[%s]: %s\n", ent->client->pers.netname, chatText );
                Com_sprintf (name, sizeof(name), EC"^5<Report>^7%s: ", 
                        ent->client->pers.netname );
            }
            else {
                return;
            }
            color = COLOR_CYAN;
            break;

        case SAY_CLAN:
            if (ent->client->pers.iamclan == 1){
                G_LogPrintf( "sayteam: <clan>[%s]: %s\n", ent->client->pers.netname, chatText );
                Com_sprintf (name, sizeof(name), EC"^1<clan>^7%s: ", 
                        ent->client->pers.netname );
            }
            else {
                return;
            }
            color = COLOR_RED;
            break;

        case SAY_TEAM:
            G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
            if (Team_GetLocationMsg(ent, location, sizeof(location)))
            {
                Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
                        ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
                locMsg = location;
            }
            else
            {
                Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
                        ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
            }
            color = COLOR_CYAN;
            break;
        case SAY_TELL:
            if (target && g_gametype.integer >= GT_TEAM &&
                    target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
                    Team_GetLocationMsg(ent, location, sizeof(location)))
            {
                Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
                locMsg = location;
            }
            else
            {
                Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
            }
            color = COLOR_MAGENTA;
            break;
    }

    Q_strncpyz( text, chatText, sizeof(text) );

    if ( target ) {
        G_SayTo( ent, target, mode, color, name, text, locMsg );
        return;
    }

    // echo the text to the console
    if ( g_dedicated.integer ) {
        G_Printf( "%s%s\n", name, text);
    }

    //RoAR mod: BADWORD FILTER
    // this whole thing needs testing
    if (*cm_badwords.string && cm_badwords.string[0]) {
        const char *my_str_literal = cm_badwords.string;
        char *str = strdup(my_str_literal);  // We own str's memory now.
        char *token;
        while ((token = strsep(&str, ";"))) {
            if (strstr(text, token) != NULL) {
                trap_SendServerCommand(ent-g_entities, va("print \"You said bad word: %s\n\"", token));
                break;
            }
        }
        free(str);
    }
    //END

    if (strstr(text, "!freezemotd")) {
        cmfreezeMOTD(ent);
    }
    else if (strstr(text, "!endduel")) {
        endDuel(ent);
    }
    else if (strstr(text, "!showmotd")) {
        showMOTD(ent);
    }
    else if (strstr(text, "!leaders")) {
        cmLeaders(ent);
    }
    else if (strstr(text, "!jetpack")) {
        cmJetpack(ent);
    }
    else if (strstr(text, "!stats")) {
        cmStats(ent, "");
    }
    else if (strstr(text, "!hidemotd")) {
        hideMOTD(ent);
    }

    else if (strstr(text, "!slapme")) {
        if (ent->client->ps.weaponTime > 0)
        { //weapon busy
            return;
        }
        if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
        { //force power or knockdown or something
            return;
        }
        if (roar_allow_KnockMeDown_command.integer == 0)
        {
            trap_SendServerCommand(ent-g_entities, va("print \"^7KnockMeDown is disabled on this server!\n\""));
            return;
        }
        if (ent->client->pers.amdemigod == 1 || ent->client->pers.amhuman == 1
                || ent->client->pers.ampunish == 1 || ent->client->pers.amsleep == 1 || ent->client->pers.amfreeze == 1) {
            trap_SendServerCommand(ent-g_entities, va("print \"Cannot knock yourself down in your current state.\n\""));
            return;
        }
        else if (ent->health < 1 || (ent->client->ps.eFlags & EF_DEAD))
        {
        }
        else
        {
            ent->client->ps.velocity[2] = 375;
            ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
            ent->client->ps.forceDodgeAnim = 0;
            ent->client->ps.weaponTime = 1000;
            ent->client->ps.forceHandExtendTime = level.time + 700;
        }
    }

    else if (strstr(text, "adminlogin")) {
        if (cm_spillpassword.integer == 1) {
            trap_SendServerCommand(ent-g_entities, va("print \"cm_spillpassword is on. You are not allowed to say adminlogin for security reasons.\n\""));
            return;
        }
    }

    // send it to all the apropriate clients
    for (j = 0; j < level.maxclients; j++) {
        other = &g_entities[j];
        G_SayTo( ent, other, mode, color, name, text, locMsg );
    }
}


/*
   ==================
   Cmd_Say_f
   ==================
 */
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
    char		*p;

    if ( trap_Argc () < 2 && !arg0 ) {
        return;
    }

    if (arg0)
    {
        p = ConcatArgs( 0 );
    }
    else
    {
        p = ConcatArgs( 1 );
    }

    /* FIX Gamall : This bit should prevent crashes... */
    if ( strlen(p) > 150 )
    {
        p[149] = 0 ;
    }
    /* END OF FIX */

    G_Say( ent, NULL, mode, p );
}

/*
   ==================
   Cmd_Tell_f
   ==================
 */
static void Cmd_Tell_f( gentity_t *ent ) {
    int			targetNum;
    gentity_t	*target;
    char		*p;
    char		arg[MAX_TOKEN_CHARS];

    if ( trap_Argc () < 2 ) {
        return;
    }

    trap_Argv( 1, arg, sizeof( arg ) );

    targetNum = G_ClientNumberFromArg( arg );
    if ( targetNum < 0 || targetNum >= level.maxclients ) {
        return;
    }

    target = &g_entities[targetNum];
    if ( !target || !target->inuse || !target->client ) {
        return;
    }

    p = ConcatArgs( 2 );

    /* FIX Gamall : This bit should prevent crashes... */
    if ( strlen(p) > 150 )
    {
        p[149] = 0 ;
    }
    /* END OF FIX */

    G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
    G_Say( ent, target, SAY_TELL, p );
    // don't tell to the player self if it was already directed to this player
    // also don't send the chat back to a bot
    if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
        G_Say( ent, ent, SAY_TELL, p );
    }
}

//siege voice command
static void Cmd_VoiceCommand_f(gentity_t *ent)
{
    gentity_t *te;
    char arg[MAX_TOKEN_CHARS];
    char *s;
    int i = 0;

    if (g_gametype.integer < GT_TEAM)
    {
        return;
    }

    if (trap_Argc() < 2)
    {
        return;
    }

    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
            ent->client->tempSpectate >= level.time)
    {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOICECHATASSPEC")) );
        return;
    }

    trap_Argv(1, arg, sizeof(arg));

    if (arg[0] == '*')
    { //hmm.. don't expect a * to be prepended already. maybe someone is trying to be sneaky.
        return;
    }

    s = va("*%s", arg);

    //now, make sure it's a valid sound to be playing like this.. so people can't go around
    //screaming out death sounds or whatever.
    while (i < MAX_CUSTOM_SIEGE_SOUNDS)
    {
        if (!bg_customSiegeSoundNames[i])
        {
            break;
        }
        if (!Q_stricmp(bg_customSiegeSoundNames[i], s))
        { //it matches this one, so it's ok
            break;
        }
        i++;
    }

    if (i == MAX_CUSTOM_SIEGE_SOUNDS || !bg_customSiegeSoundNames[i])
    { //didn't find it in the list
        return;
    }

    te = G_TempEntity(vec3_origin, EV_VOICECMD_SOUND);
    te->s.groundEntityNum = ent->s.number;
    te->s.eventParm = G_SoundIndex((char *)bg_customSiegeSoundNames[i]);
    te->r.svFlags |= SVF_BROADCAST;
}


static char	*gc_orders[] = {
    "hold your position",
    "hold this position",
    "come here",
    "cover me",
    "guard location",
    "search and destroy",
    "report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
    int		player;
    int		order;
    char	str[MAX_TOKEN_CHARS];

    trap_Argv( 1, str, sizeof( str ) );
    player = atoi( str );
    trap_Argv( 2, str, sizeof( str ) );
    order = atoi( str );

    if ( player < 0 || player >= MAX_CLIENTS ) {
        return;
    }
    if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
        return;
    }
    G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
    G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
   ==================
   Cmd_Where_f
   ==================
 */
void Cmd_Where_f( gentity_t *ent ) {
    // Fix: ensiform
    //trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
    trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
}

static const char *gameNames[] = {
    "Free For All",
    "Holocron FFA",
    "Jedi Master",
    "Duel",
    "Power Duel",
    "Single Player",
    "Team FFA",
    "Siege",
    "Capture the Flag",
    "Capture the Ysalamiri",
    "Role Playing"
};

/*
   ==================
   G_ClientNumberFromName

   Finds the client number of the client with the given name
   ==================
 */
int G_ClientNumberFromName ( const char* name )
{
    char		s2[MAX_STRING_CHARS];
    char		n2[MAX_STRING_CHARS];
    int			i;
    gclient_t*	cl;

    // check for a name match
    SanitizeString( (char*)name, s2 );
    for ( i=0 ; i < level.numConnectedClients ; i++ )
    {
        cl=&level.clients[level.sortedClients[i]];
        SanitizeString( cl->pers.netname, n2 );
        if ( !strcmp( n2, s2 ) ) 
        {
            return i;
        }
    }

    return -1;
}

/*
   ==================
   SanitizeString2

   Rich's revised version of SanitizeString
   ==================
 */
void SanitizeString2( char *in, char *out )
{
    int i = 0;
    int r = 0;

    while (in[i])
    {
        if (i >= MAX_NAME_LENGTH-1)
        { //the ui truncates the name here..
            break;
        }

        if (in[i] == '^')
        {
            if (in[i+1] >= 48 && //'0'
                    in[i+1] <= 57) //'9'
            { //only skip it if there's a number after it for the color
                i += 2;
                continue;
            }
            else
            { //just skip the ^
                i++;
                continue;
            }
        }

        if (in[i] < 32)
        {
            i++;
            continue;
        }

        out[r] = tolower(in[i]);
        r++;
        i++;
    }
    out[r] = 0;
}

//lmo added for ease of admin commands
int G_ClientNumberFromStrippedSubstring ( const char* name )
{
    char		s2[MAX_STRING_CHARS];
    char		n2[MAX_STRING_CHARS];
    int			i, match = -1;
    gclient_t	*cl;

    // check for a name match
    SanitizeString2( (char*)name, s2 );

    for ( i=0 ; i < level.numConnectedClients ; i++ ) 
    {
        cl=&level.clients[level.sortedClients[i]];
        SanitizeString2( cl->pers.netname, n2 );
        if ( strstr( n2, s2 ) ) 
        {
            if( match != -1 )
            { //found more than one match
                return -2;
            }
            match = level.sortedClients[i];
        }
    }

    return match;
}

////lmo to snag client id from argument to admin command

int G_ClientNumberFromArg ( char* name)
{
    int client_id = 0;
    char *cp;

    cp = name;
    while (*cp)
    {
        if ( *cp >= '0' && *cp <= '9' ) cp++;
        else
        {
            client_id = -1; //mark as alphanumeric
            break;
        }
    }

    if ( client_id == 0 )
    { // arg is assumed to be client number
        client_id = atoi(name);
    }
    // arg is client name
    if ( client_id == -1 )
    {
        client_id = G_ClientNumberFromStrippedSubstring(name);
    }
    return client_id;
}

/*
   ==================
   Cmd_CallVote_f
   ==================
 */
typedef enum
{
    MAP_RESTART = 0,
    NEXTMAP,
    MAP,
    G_GAMETYPE,
    KICK,
    CLIENTKICK,
    G_DOWARMUP,
    TIMELIMIT,
    FRAGLIMIT,
    MODCONTROL,
    SLEEP,
    SILENCE
} vote_type_t;

extern void SiegeClearSwitchData(void); //g_saga.c
const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
    int		i;
    char	arg1[MAX_STRING_TOKENS];
    char	arg2[MAX_STRING_TOKENS];
    //	int		n = 0;
    //	char*	type = NULL;
    char*		mapName = 0;
    const char*	arenaInfo;

    if ( !g_allowVote.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
        return;
    }

    if ( level.voteTime || level.voteExecuteTime >= level.time ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEINPROGRESS")) );
        return;
    }
    if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXVOTES")) );
        return;
    }

    if (g_gametype.integer != GT_DUEL &&
            g_gametype.integer != GT_POWERDUEL)
    {
        if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
            trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
            return;
        }
    }

    // make sure it is a valid command to vote on
    trap_Argv( 1, arg1, sizeof( arg1 ) );
    trap_Argv( 2, arg2, sizeof( arg2 ) );

    if( strchr( arg1, ';'  ) || strchr( arg2, ';'  ) ||
            strchr( arg1, '\n' ) || strchr( arg2, '\n' ) || 
            strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ) {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        return;
    }

    if ( !Q_stricmp( arg1, "map_restart" ) ) {
    } else if ( !Q_stricmp( arg1, "nextmap" ) ) {
    } else if ( !Q_stricmp( arg1, "map" ) ) {
    } else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
    } else if ( !Q_stricmp( arg1, "kick" ) ) {
    } else if ( !Q_stricmp( arg1, "clientkick" ) ) {
    } else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
    } else if ( !Q_stricmp( arg1, "timelimit" ) ) {
    } else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
    } else if ( !Q_stricmp( arg1, "modcontrol" ) ) {
    } else if ( !Q_stricmp( arg1, "sleep" ) ) {
    } else if ( !Q_stricmp( arg1, "silence" ) ) {
    } else {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>, modcontrol <num>, sleep <player>, silence <player>.\n\"" );
        return;
    }

    // if there is still a vote to be executed
    if ( level.voteExecuteTime ) {
        level.voteExecuteTime = 0;
        trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
    }

    // special case for g_gametype, check for bad values
    if ( !Q_stricmp( arg1, "g_gametype" ) )
    {
        i = atoi( arg2 );
        if( !(m_rV.integer & (1 << G_GAMETYPE)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Game Type Change not allowed.\n\"");
            return;
        }
        if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
            trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
            return;
        }

        level.votingGametype = qtrue;
        level.votingGametypeTo = i;

        Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
    }
    else if ( !Q_stricmp( arg1, "map" ) ) 
    {
        // special case for map changes, we want to reset the nextmap setting
        // this allows a player to change maps, but not upset the map rotation
        char	s[MAX_STRING_CHARS];

        if( !(m_rV.integer & (1 << MAP)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Map Change not allowed.\n\"");
            return;
        }
        if (!G_DoesMapSupportGametype(arg2, trap_Cvar_VariableIntegerValue("g_gametype")))
        {
            //trap_SendServerCommand( ent-g_entities, "print \"You can't vote for this map, it isn't supported by the current gametype.\n\"" );
            trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
            return;
        }

        trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
        if (*s) {
            Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
        } else {
            Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
        }

        arenaInfo	= G_GetArenaInfoByMap(arg2);
        if (arenaInfo)
        {
            mapName = Info_ValueForKey(arenaInfo, "longname");
        }

        if (!mapName || !mapName[0])
        {
            mapName = "ERROR";
        }

        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", mapName);
    }
    else if ( !Q_stricmp ( arg1, "clientkick" ) )
    {
        int n = atoi ( arg2 );

        if( !(m_rV.integer & (1 << KICK)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Client Kick not allowed.\n\"");
            return;
        }
        if ( n < 0 || n >= MAX_CLIENTS )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
            return;
        }

        if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
            return;
        }

        Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
        Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );
    }
    else if ( !Q_stricmp ( arg1, "kick" ) )
    {
        int clientid = G_ClientNumberFromName ( arg2 );

        if( !(m_rV.integer & (1 << KICK)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Kick not allowed.\n\"");
            return;
        }

        if ( clientid == -1 )
        {
            clientid = G_ClientNumberFromArg(  arg2 );

            if (clientid == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
                return;
            }
        }

        Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
        Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
    }
    else if ( !Q_stricmp( arg1, "nextmap" ) ) 
    {
        char	s[MAX_STRING_CHARS];

        if( !(m_rV.integer & (1 << NEXTMAP)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Next Map not allowed.\n\"");
            return;
        }
        trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
        if (!*s) {
            trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
            return;
        }
        SiegeClearSwitchData();
        Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
    } 
    else if ( !Q_stricmp( arg1, "modcontrol") )
    {
        int votenum;

        if ( !(m_rV.integer & (1 << MODCONTROL)) )
        {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Mod Control not allowed.\n\"");
            return;
        }
        votenum = atoi(arg2);
        Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", mod_votes[votenum].voteexec );
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "modcontrol ^7%s", mod_votes[votenum].votestring );
    }
    else if ( !Q_stricmp ( arg1, "sleep" ) )
    {
        int clientid = G_ClientNumberFromName ( arg2 );

        if( !(m_rV.integer & (1 << SLEEP)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Sleep not allowed.\n\"");
            return;
        }

        if ( clientid == -1 )
        {
            clientid = G_ClientNumberFromArg(  arg2 );

            if (clientid == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
                return;
            }
        }

        Com_sprintf ( level.voteString, sizeof(level.voteString ), "sleep %d", clientid );
        Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "sleep %s", g_entities[clientid].client->pers.netname );
    }
    else if ( !Q_stricmp ( arg1, "silence" ) )
    {
        int clientid = G_ClientNumberFromName ( arg2 );

        if( !(m_rV.integer & (1 << SILENCE)) ) {
            trap_SendServerCommand(ent-g_entities, "print \"Voting for Silence not allowed.\n\"");
            return;
        }

        if ( clientid == -1 )
        {
            clientid = G_ClientNumberFromArg(  arg2 );

            if (clientid == -1)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
                return;
            }
        }

        Com_sprintf ( level.voteString, sizeof(level.voteString ), "silence %d", clientid );
        Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "silence %s", g_entities[clientid].client->pers.netname );
    }
    else
    {
        if( (!Q_stricmp( arg1, "g_dowarmup") && !(m_rV.integer & (1 << G_DOWARMUP))) ||
                (!Q_stricmp( arg1, "timelimit") && !(m_rV.integer & (1 << TIMELIMIT))) ||
                (!Q_stricmp( arg1, "map_restart") && !(m_rV.integer & (1 << MAP_RESTART))) ||
                (!Q_stricmp( arg1, "fraglimit") && !(m_rV.integer & (1 << FRAGLIMIT))))
        {
            trap_SendServerCommand(ent-g_entities, va("print \"Voting not allowed for %s\n\"", arg1));
            return;
        }
        Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
    }

    trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCALLEDVOTE") ) );

    // start the voting, the caller autoamtically votes yes
    level.voteTime = level.time;
    level.voteYes = 1;
    level.voteNo = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        level.clients[i].mGameFlags &= ~PSG_VOTED;
    }
    ent->client->mGameFlags |= PSG_VOTED;

    trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
    trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
    trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
    trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
}

/*
   ==================
   Cmd_Vote_f
   ==================
 */
void Cmd_Vote_f( gentity_t *ent ) {
    char		msg[64];

    if ( !level.voteTime ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEINPROG")) );
        return;
    }
    if ( ent->client->mGameFlags & PSG_VOTED ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEALREADY")) );
        return;
    }
    if (g_gametype.integer != GT_DUEL &&
            g_gametype.integer != GT_POWERDUEL)
    {
        if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
            trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
            return;
        }
    }

    trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLVOTECAST")) );

    ent->client->mGameFlags |= PSG_VOTED;

    trap_Argv( 1, msg, sizeof( msg ) );

    if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
        level.voteYes++;
        trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
    } else {
        level.voteNo++;
        trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
    }

    // a majority will be determined in CheckVote, which will also account
    // for players entering or leaving
}

/*
   ==================
   Cmd_CallTeamVote_f
   ==================
 */
void Cmd_CallTeamVote_f( gentity_t *ent ) {
    int		i, team, cs_offset;
    char	arg1[MAX_STRING_TOKENS];
    char	arg2[MAX_STRING_TOKENS];

    team = ent->client->sess.sessionTeam;
    if ( team == TEAM_RED )
        cs_offset = 0;
    else if ( team == TEAM_BLUE )
        cs_offset = 1;
    else
        return;

    if ( !g_allowVote.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
        return;
    }

    if ( level.teamVoteTime[cs_offset] ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADY")) );
        return;
    }
    if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXTEAMVOTES")) );
        return;
    }
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
        return;
    }

    // make sure it is a valid command to vote on
    trap_Argv( 1, arg1, sizeof( arg1 ) );
    arg2[0] = '\0';
    for ( i = 2; i < trap_Argc(); i++ ) {
        if (i > 2)
            strcat(arg2, " ");
        trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
    }

    if( strchr( arg1, ';'  ) || strchr( arg2, ';'  ) ||
            strchr( arg1, '\n' ) || strchr( arg2, '\n' ) || 
            strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ) {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        return;
    }

    if ( !Q_stricmp( arg1, "leader" ) ) {
        char netname[MAX_NETNAME], leader[MAX_NETNAME];

        if ( !arg2[0] ) {
            i = ent->client->ps.clientNum;
        }
        else {
            // numeric values are just slot numbers
            for (i = 0; i < 3; i++) {
                if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
                    break;
            }
            if ( i >= 3 || !arg2[i]) {
                i = atoi( arg2 );
                if ( i < 0 || i >= level.maxclients ) {
                    trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
                    return;
                }

                if ( !g_entities[i].inuse ) {
                    trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
                    return;
                }
            }
            else {
                Q_strncpyz(leader, arg2, sizeof(leader));
                Q_CleanStr(leader);
                for ( i = 0 ; i < level.maxclients ; i++ ) {
                    if ( level.clients[i].pers.connected == CON_DISCONNECTED )
                        continue;
                    if (level.clients[i].sess.sessionTeam != team)
                        continue;
                    Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
                    Q_CleanStr(netname);
                    if ( !Q_stricmp(netname, leader) ) {
                        break;
                    }
                }
                if ( i >= level.maxclients ) {
                    trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
                    return;
                }
            }
        }
        Com_sprintf(arg2, sizeof(arg2), "%d", i);
    } else {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
        return;
    }

    Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED )
            continue;
        if (level.clients[i].sess.sessionTeam == team)
            trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
    }

    // start the voting, the caller autoamtically votes yes
    level.teamVoteTime[cs_offset] = level.time;
    level.teamVoteYes[cs_offset] = 1;
    level.teamVoteNo[cs_offset] = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if (level.clients[i].sess.sessionTeam == team)
            level.clients[i].mGameFlags &= ~PSG_TEAMVOTED;
    }
    ent->client->mGameFlags |= PSG_TEAMVOTED;

    trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
    trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
    trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
    trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
   ==================
   Cmd_TeamVote_f
   ==================
 */
void Cmd_TeamVote_f( gentity_t *ent ) {
    int			team, cs_offset;
    char		msg[64];

    team = ent->client->sess.sessionTeam;
    if ( team == TEAM_RED )
        cs_offset = 0;
    else if ( team == TEAM_BLUE )
        cs_offset = 1;
    else
        return;

    if ( !level.teamVoteTime[cs_offset] ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOTEAMVOTEINPROG")) );
        return;
    }
    if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADYCAST")) );
        return;
    }
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
        return;
    }

    trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLTEAMVOTECAST")) );

    ent->client->mGameFlags |= PSG_TEAMVOTED;

    trap_Argv( 1, msg, sizeof( msg ) );

    if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
        level.teamVoteYes[cs_offset]++;
        trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
    } else {
        level.teamVoteNo[cs_offset]++;
        trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );	
    }

    // a majority will be determined in TeamCheckVote, which will also account
    // for players entering or leaving
}


/*
   =================
   Cmd_SetViewpos_f
   =================
 */
void Cmd_SetViewpos_f( gentity_t *ent ) {
    vec3_t		origin, angles;
    char		buffer[MAX_TOKEN_CHARS];
    int			i;

    if ( !g_cheats.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
        return;
    }
    if ( trap_Argc() != 5 ) {
        trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
        return;
    }

    VectorClear( angles );
    for ( i = 0 ; i < 3 ; i++ ) {
        trap_Argv( i + 1, buffer, sizeof( buffer ) );
        origin[i] = atof( buffer );
    }

    trap_Argv( 4, buffer, sizeof( buffer ) );
    angles[YAW] = atof( buffer );

    TeleportPlayer( ent, origin, angles );
}

void Admin_Teleport( gentity_t *ent ) {
    vec3_t		origin;
    char		buffer[MAX_TOKEN_CHARS];
    int			i;

    if ( trap_Argc() != 4 ) {
        trap_SendServerCommand( ent-g_entities, va("print \"usage: tele (X) (Y) (Z)\ntype in /origin OR /origin (name) to find out (X) (Y) (Z)\n\""));
        return;
    }

    for ( i = 0 ; i < 3 ; i++ ) {
        trap_Argv( i + 1, buffer, sizeof( buffer ) );
        origin[i] = atof( buffer );
    }

    TeleportPlayer( ent, origin, ent->client->ps.viewangles );
}

//===============NPCMod===================//
/*
   =================
   Cmd_OrderNPC_f
   =================
 */
void Cmd_OrderNPC_f( gentity_t *ent )
{
    NPCORDER_FUNC	*order;
    char			buffer[MAX_TOKEN_CHARS];
    qboolean		applyOrder[MAX_FOLLOWERS];
    int				i, index = -1;
    qboolean		found = qfalse;

    if ( trap_Argc() < 2 )
    {
        orderNPC_t *type;
        trap_SendServerCommand( ent-g_entities, "print \"usage: ordernpc *ordertype* [ *npcclass or npcname* [ *number* ] ]\npossible values for *ordertype* are :\n\"");
        type = orderNPCTable;
        while ( type->order )
        {
            trap_SendServerCommand( ent-g_entities, va( "print \"'%s' - %s\n\"", type->name, type->description ) );			
            type++;
        }
        return;
    }

    if ( !ent->client->numPlFollowers )
    {
        trap_SendServerCommand( ent-g_entities, "print \"you don't have any follower!\n\"");
        return;
    }

    trap_Argv( 1, buffer, sizeof( buffer ) );
    order = NPCF_GetOrderForName( buffer );
    if ( !order )
    {
        trap_SendServerCommand( ent-g_entities, "print \"invalid order!\n\"");
        return;
    }

    //pour tous
    if ( trap_Argc() == 2 )
    {
        NPCF_OrderToAll( ent, order );
        return;
    }

    memset( buffer, 0, sizeof(buffer) );
    trap_Argv( 2, buffer, sizeof( buffer ) );

    for ( i = 0;i < ent->client->numPlFollowers;i++ )
    {
        gentity_t *fwr = ent->client->plFollower[i];

        applyOrder[i] = qfalse;

        if ( !Q_stricmp( buffer, fwr->targetname ) )
        {
            NPCF_Order( ent, fwr, order, qtrue );
            return;
        }

        //if ( !Q_wildmat( fwr->NPC_type, buffer ) )
        //{
        applyOrder[i] = qtrue;
        found = qtrue;
        //}
    }

    if ( !found )
    {
        trap_SendServerCommand( ent-g_entities, "print \"NPC not recognized!\n\"");
        return;
    }

    if ( trap_Argc() > 3 )
    {
        memset( buffer, 0, sizeof(buffer) );
        trap_Argv( 3, buffer, sizeof( buffer ) );
        index = atoi( buffer );

        if ( index < 0 )
            index = 0;
    }

    if ( index == -1 )
    {
        for ( i = 0;i < ent->client->numPlFollowers;i++ )
        {
            gentity_t *fwr = ent->client->plFollower[i];

            if ( !applyOrder[i] )
                continue;

            if ( index == -1 )
            {
                NPCF_Order( ent, fwr, order, qtrue );
                index = 0;
            }
            else
            {
                NPCF_Order( ent, fwr, order, qfalse );
            }
        }
    }
    else
    {
        for ( i = 0;i < ent->client->numPlFollowers;i++ )
        {
            gentity_t *fwr = ent->client->plFollower[i];

            if ( !applyOrder[i] )
                continue;

            if ( !index )
            {
                NPCF_Order( ent, fwr, order, qtrue );
                break;
            }

            index--;
        }
    }

    if ( index > 0 )
    {
        trap_SendServerCommand( ent-g_entities, "print \"Bad NPC Index!\n\"");
    }
}

//===============NPCMod===================//

/*
   =================
   Cmd_Stats_f
   =================
 */
void Cmd_Stats_f( gentity_t *ent ) {
    /*
       int max, n, i;

       max = trap_AAS_PointReachabilityAreaIndex( NULL );

       n = 0;
       for ( i = 0; i < max; i++ ) {
       if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
       n++;
       }

    //trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
    trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
     */
}

//[BugFix38]
void G_LeaveVehicle( gentity_t* ent, qboolean ConCheck ) {

    if (ent->client->ps.m_iVehicleNum)
    { //tell it I'm getting off
        gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

        if (veh->inuse && veh->client && veh->m_pVehicle)
        {
            if ( ConCheck ) { // check connection
                int pCon = ent->client->pers.connected;
                ent->client->pers.connected = 0;
                veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
                ent->client->pers.connected = pCon;
            } else { // or not.
                veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
            }
        }
    }

    ent->client->ps.m_iVehicleNum = 0;
}
//[/BugFix38]

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
    vec3_t fwd, fwdorg, dest, pos;
    vec3_t yawonly;
    vec3_t mins, maxs;
    vec3_t trtest;
    trace_t tr;

    if (ps->m_iVehicleNum)
    {
        return 0;
    }

    if (ps->pm_flags & PMF_USE_ITEM_HELD)
    { //force to let go first
        return 0;
    }

    if (!forcedUse)
    {
        forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
    }

    if (!BG_IsItemSelectable(ps, forcedUse))
    {
        return 0;
    }

    switch (forcedUse)
    {
        case HI_MEDPAC:
        case HI_MEDPAC_BIG:
            if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
            {
                return 0;
            }

            if (ps->stats[STAT_HEALTH] <= 0)
            {
                return 0;
            }

            return 1;
        case HI_SEEKER:
            if (ps->eFlags & EF_SEEKERDRONE)
            {
                G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
                return 0;
            }

            return 1;
        case HI_SENTRY_GUN:
            if (ps->fd.sentryDeployed)
            {
                G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
                return 0;
            }

            yawonly[ROLL] = 0;
            yawonly[PITCH] = 0;
            yawonly[YAW] = ps->viewangles[YAW];

            VectorSet( mins, -8, -8, 0 );
            VectorSet( maxs, 8, 8, 24 );

            AngleVectors(yawonly, fwd, NULL, NULL);

            fwdorg[0] = ps->origin[0] + fwd[0]*64;
            fwdorg[1] = ps->origin[1] + fwd[1]*64;
            fwdorg[2] = ps->origin[2] + fwd[2]*64;

            trtest[0] = fwdorg[0] + fwd[0]*16;
            trtest[1] = fwdorg[1] + fwd[1]*16;
            trtest[2] = fwdorg[2] + fwd[2]*16;

            trap_Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID);

            if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
            {
                G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
                return 0;
            }

            return 1;
        case HI_SHIELD:
            mins[0] = -8;
            mins[1] = -8;
            mins[2] = 0;

            maxs[0] = 8;
            maxs[1] = 8;
            maxs[2] = 8;

            AngleVectors (ps->viewangles, fwd, NULL, NULL);
            fwd[2] = 0;
            VectorMA(ps->origin, 64, fwd, dest);
            trap_Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT );
            if (tr.fraction > 0.9 && !tr.startsolid && !tr.allsolid)
            {
                VectorCopy(tr.endpos, pos);
                VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
                trap_Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID );
                if ( !tr.startsolid && !tr.allsolid )
                {
                    return 1;
                }
            }
            G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
            return 0;
        case HI_JETPACK: //do something?
            return 1;
        case HI_HEALTHDISP:
            return 1;
        case HI_AMMODISP:
            return 1;
        case HI_EWEB:
            return 1;
        case HI_CLOAK:
            return 1;
        default:
            return 1;
    }
}

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);

void Cmd_ToggleSaber_f(gentity_t *ent)
{
    //[TAUNTFIX]
    if (ent->client->ps.weapon != WP_SABER) {
        return;
    }

    if (level.intermissiontime) { // not during intermission
        return;
    }

    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ) { // not when spec
        return;
    }

    if (ent->client->tempSpectate >= level.time ) { // not when tempSpec
        return;
    }

    if (ent->client->ps.emplacedIndex) { //on an emplaced gun
        return;
    }

    if (ent->client->ps.m_iVehicleNum) { //in a vehicle like at-st
        gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

        if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
            return;

        if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER )
            return;
    }
    //[/TAUNTFIX]

    if (ent->client->ps.fd.forceGripCripple)
    { //if they are being gripped, don't let them unholster their saber
        if (ent->client->ps.saberHolstered)
        {
            return;
        }
    }

    if (ent->client->ps.saberInFlight)
    {
        if (ent->client->ps.saberEntityNum)
        { //turn it off in midair
            saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
        }
        return;
    }

    if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
    {
        return;
    }

    //[TAUNTFIX]
    /* ensiform - moved this up to the top of function
       if (ent->client->ps.weapon != WP_SABER)
       {
       return;
       }
     */
    //[/TAUNTFIX]

    //	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
    //	{
    //		return;
    //	}

    if (ent->client->ps.duelTime >= level.time)
    {
        return;
    }

    if (ent->client->ps.saberLockTime >= level.time)
    {
        return;
    }

    if (ent->client && ent->client->ps.weaponTime < 1)
    {
        if (ent->client->ps.saberHolstered == 2)
        {
            ent->client->ps.saberHolstered = 0;

            if (ent->client->saber[0].soundOn)
            {
                G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
            }
            if (ent->client->saber[1].soundOn)
            {
                G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
            }
        }
        else
        {
            ent->client->ps.saberHolstered = 2;
            if (ent->client->saber[0].soundOff)
            {
                G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
            }
            if (ent->client->saber[1].soundOff &&
                    ent->client->saber[1].model[0])
            {
                G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
            }
            //prevent anything from being done for 400ms after holster
            ent->client->ps.weaponTime = 400;
        }
    }
}

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
    int selectLevel = 0;
    qboolean usingSiegeStyle = qfalse;

    if ( !ent || !ent->client )
    {
        return;
    }

    // Fix: Wudan - Saber Cycle Fix
    if ( ent->client->ps.weapon != WP_SABER )
    {
        return;
    }

    /*
       if (ent->client->ps.weaponTime > 0)
       { //no switching attack level when busy
       return;
       }
     */

    //[TAUNTFIX]
    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
    { //not for spectators
        return;
    }

    if (ent->client->tempSpectate >= level.time)
    { //not for spectators
        return;
    }

    if (level.intermissiontime)
    { //not during intermission
        return;
    }

    if (ent->client->ps.m_iVehicleNum)
    { //in a vehicle like at-st
        gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

        if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
            return;

        if ( veh->m_pVehicle && veh->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER )
            return;
    }
    //[/TAUNTFIX]

    if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
    { //no cycling for akimbo
        if ( WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
        {//can turn second saber off 
            if ( ent->client->ps.saberHolstered == 1 )
            {//have one holstered
                //unholster it
                G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
                ent->client->ps.saberHolstered = 0;
                //g_active should take care of this, but...
                ent->client->ps.fd.saberAnimLevel = SS_DUAL;
            }
            else if ( ent->client->ps.saberHolstered == 0 )
            {//have none holstered
                if ( (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
                {//can't turn it off manually
                }
                else if ( ent->client->saber[1].bladeStyle2Start > 0
                        && (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
                {//can't turn it off manually
                }
                else
                {
                    //turn it off
                    G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
                    ent->client->ps.saberHolstered = 1;
                    //g_active should take care of this, but...
                    ent->client->ps.fd.saberAnimLevel = SS_FAST;
                }
            }

            if (d_saberStanceDebug.integer)
            {
                trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle dual saber blade.\n\"") );
            }
            return;
        }
    }
    else if (ent->client->saber[0].numBlades > 1
            && WP_SaberCanTurnOffSomeBlades( &ent->client->saber[0] ) )
    { //use staff stance then.
        if ( ent->client->ps.saberHolstered == 1 )
        {//second blade off
            if ( ent->client->ps.saberInFlight )
            {//can't turn second blade back on if it's in the air, you naughty boy!
                if (d_saberStanceDebug.integer)
                {
                    trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade in air.\n\"") );
                }
                return;
            }
            //turn it on
            G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
            ent->client->ps.saberHolstered = 0;
            //g_active should take care of this, but...
            if ( ent->client->saber[0].stylesForbidden )
            {//have a style we have to use
                WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
                if ( ent->client->ps.weaponTime <= 0 )
                { //not busy, set it now
                    ent->client->ps.fd.saberAnimLevel = selectLevel;
                }
                else
                { //can't set it now or we might cause unexpected chaining, so queue it
                    ent->client->saberCycleQueue = selectLevel;
                }
            }
        }
        else if ( ent->client->ps.saberHolstered == 0 )
        {//both blades on
            if ( (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
            {//can't turn it off manually
            }
            else if ( ent->client->saber[0].bladeStyle2Start > 0
                    && (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
            {//can't turn it off manually
            }
            else
            {
                //turn second one off
                G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
                ent->client->ps.saberHolstered = 1;
                //g_active should take care of this, but...
                if ( ent->client->saber[0].singleBladeStyle != SS_NONE )
                {
                    if ( ent->client->ps.weaponTime <= 0 )
                    { //not busy, set it now
                        ent->client->ps.fd.saberAnimLevel = ent->client->saber[0].singleBladeStyle;
                    }
                    else
                    { //can't set it now or we might cause unexpected chaining, so queue it
                        ent->client->saberCycleQueue = ent->client->saber[0].singleBladeStyle;
                    }
                }
            }
        }
        if (d_saberStanceDebug.integer)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade.\n\"") );
        }
        return;
    }

    if (ent->client->saberCycleQueue)
    { //resume off of the queue if we haven't gotten a chance to update it yet
        selectLevel = ent->client->saberCycleQueue;
    }
    else
    {
        selectLevel = ent->client->ps.fd.saberAnimLevel;
    }

    if (g_gametype.integer == GT_SIEGE &&
            ent->client->siegeClass != -1 &&
            bgSiegeClasses[ent->client->siegeClass].saberStance)
    { //we have a flag of useable stances so cycle through it instead
        int i = selectLevel+1;

        usingSiegeStyle = qtrue;

        while (i != selectLevel)
        { //cycle around upward til we hit the next style or end up back on this one
            if (i >= SS_NUM_SABER_STYLES)
            { //loop back around to the first valid
                i = SS_FAST;
            }

            if (bgSiegeClasses[ent->client->siegeClass].saberStance & (1 << i))
            { //we can use this one, select it and break out.
                selectLevel = i;
                break;
            }
            i++;
        }

        if (d_saberStanceDebug.integer)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle given class stance.\n\"") );
        }
    }
    else
    {
        selectLevel++;
        if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
        {
            selectLevel = FORCE_LEVEL_1;
        }
        if (d_saberStanceDebug.integer)
        {
            trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle stance normally.\n\"") );
        }
    }
    /*
#ifndef FINAL_BUILD
switch ( selectLevel )
{
case FORCE_LEVEL_1:
trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
break;
case FORCE_LEVEL_2:
trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
break;
case FORCE_LEVEL_3:
trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
break;
}
#endif
     */
if ( !usingSiegeStyle )
{
    //make sure it's valid, change it if not
    WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
}

if (ent->client->ps.weaponTime <= 0)
{ //not busy, set it now
    ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
}
else
{ //can't set it now or we might cause unexpected chaining, so queue it
    ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
}
}

qboolean G_OtherPlayersDueling(void)
{
    int i = 0;
    gentity_t *ent;

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
        {
            return qtrue;
        }
        i++;
    }

    return qfalse;
}

qboolean ScalePlayer( gentity_t *self, int scale );
void Cmd_EngageDuel_f(gentity_t *ent, int dueltype)
{
    trace_t tr;
    vec3_t forward, fwdOrg;
    int i;

    if (!g_privateDuel.integer)
    {
        return;
    }

    if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
    { //rather pointless in this mode..
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
        return;
    }

    //if (g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_SIEGE)
    if (g_gametype.integer >= GT_TEAM)
    { //no private dueling in team modes
        trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
        return;
    }

    if (ent->client->ps.duelTime >= level.time)
    {
        return;
    }

    //RoAR mod NOTE: MELEE DUEL
    /*if (ent->client->ps.weapon != WP_SABER)
      {
      return;
      }*/

    /*
       if (!ent->client->ps.saberHolstered)
       { //must have saber holstered at the start of the duel
       return;
       }
     */
    //NOTE: No longer doing this..

    if (ent->client->ps.saberInFlight)
    {
        return;
    }

    if (ent->client->ps.duelInProgress)
    {
        return;
    }

    if (ent->client->pers.amdemigod)
    {
        return;
    }

    if (ent->client->pers.amfreeze)
    {
        return;
    }

    if (ent->client->pers.amsleep )
    {
        return;
    }

    if (ent->client->pers.ampunish )
    {
        return;
    }

    if (ent->client->pers.refuseduels )
    {
        return;
    }


    //New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
    /*if (ent->client->ps.fd.privateDuelTime > level.time)
      {
      trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
      return;
      }*/

    //[DuelSys]
    // MJN - cvar g_multiDuel allows more than 1 private duel at a time.
    /*if (!g_multiDuel.integer && G_OtherPlayersDueling())
    //if (G_OtherPlayersDueling())
    //[/DuelSys]
    {
    trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_BUSY")) );
    return;
    }*/

    AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

    fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
    fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
    fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

    trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

    if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
    {
        gentity_t *challenged = &g_entities[tr.entityNum];

        //RoAR mod NOTE: Melee can now challenge saber ppl!
        if (!challenged || !challenged->client || !challenged->inuse ||
                challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
                /*challenged->client->ps.weapon != WP_SABER ||*/ challenged->client->ps.duelInProgress ||
                challenged->client->ps.saberInFlight)
        {
            return;
        }

        if (challenged->client->pers.refuseduels){
            trap_SendServerCommand( ent-g_entities, va("cp \"%s\n^7is refusing duels\n\"", challenged->client->pers.netname) );
            return;
        }

        if ((level.modeClanMatch == qtrue) && (level.modeMeeting == qtrue)){
            return;
        }

        //Cmd_EngageDuel_f(ent, dueltypes[challenged->client->ps.clientNum]);

        if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
        {
            return;
        }

        //RoAR mod NOTE: DUELS BEGIN HERE!
        if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time
                && dueltypes[challenged->client->ps.clientNum] == dueltype )
        {
            //dueltypes[challenged->client->ps.clientNum] = dueltypes[ent->client->ps.clientNum];
            dueltypes[ent->client->ps.clientNum] = dueltype;
            ent->client->ps.duelInProgress = qtrue;
            challenged->client->ps.duelInProgress = qtrue;			

            ent->client->ps.duelTime = level.time + 2000;
            challenged->client->ps.duelTime = level.time + 2000;

            ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_JETPACK);
            challenged->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_JETPACK);
            if (challenged->client->jetPackOn)
            {
                Jetpack_Off(challenged);
            }
            if (ent->client->jetPackOn)
            {
                Jetpack_Off(ent);
            }

            ScalePlayer(ent, 100);
            ScalePlayer(challenged, 100);

            ent->client->ps.powerups[PW_FORCE_ENLIGHTENED_LIGHT] = 0;
            ent->client->ps.powerups[PW_FORCE_ENLIGHTENED_DARK] = 0;
            ent->client->ps.powerups[PW_FORCE_BOON] = 0;
            challenged->client->ps.powerups[PW_FORCE_ENLIGHTENED_LIGHT] = 0;
            challenged->client->ps.powerups[PW_FORCE_ENLIGHTENED_DARK] = 0;
            challenged->client->ps.powerups[PW_FORCE_BOON] = 0;

            //WEAPON TAKEAWAY!!!
            ent->client->ps.eFlags &= ~EF_SEEKERDRONE;
            //if (g_gametype.integer != GT_RPG){ //RoAR mod NOTE: In RPG mode, you can duel with the current weapons you have.
            ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & ~(1 << HI_CLOAK) & ~(1 << HI_EWEB) & ~(1 << HI_SENTRY_GUN) & ~(1 << HI_BINOCULARS);
            ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
                & ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
                & ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
            //
            challenged->client->ps.eFlags &= ~EF_SEEKERDRONE;
            challenged->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << HI_SEEKER) & (1 << HI_CLOAK) & (1 << HI_EWEB) & (1 << HI_SENTRY_GUN) & (1 << HI_BINOCULARS);
            challenged->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON) & ~(1 << WP_BLASTER) & ~(1 << WP_DISRUPTOR) & ~(1 << WP_BOWCASTER)
                & ~(1 << WP_REPEATER) & ~(1 << WP_DEMP2) & ~(1 << WP_FLECHETTE) & ~(1 << WP_ROCKET_LAUNCHER) & ~(1 << WP_THERMAL) & ~(1 << WP_DET_PACK)
                & ~(1 << WP_BRYAR_OLD) & ~(1 << WP_CONCUSSION) & ~(1 << WP_TRIP_MINE) & ~(1 << WP_BRYAR_PISTOL);
            //

            switch (dueltype) {
                case 0: //SABER (NORMAL) DUEL
                    G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
                    G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);
                    ent->client->ps.stats[STAT_HEALTH] = ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
                    ent->client->ps.stats[STAT_ARMOR] = cm_duelshield.integer;
                    challenged->client->ps.stats[STAT_HEALTH] = challenged->health = challenged->client->ps.stats[STAT_MAX_HEALTH];
                    challenged->client->ps.stats[STAT_ARMOR] = cm_duelshield.integer;
                    trap_SendServerCommand( -1, va("print \"%s ^7has become engaged in a ^1saber^7 duel with %s!\n\"", challenged->client->pers.netname, ent->client->pers.netname) );
                    WebHook(ent, W_DUELS, va("%s has become engaged in a saber duel with %s!", Q_CleanStr(challenged->client->pers.netname), Q_CleanStr(ent->client->pers.netname)));
                    //if (g_gametype.integer != GT_RPG){ //RoAR mod NOTE: I don't think so.
                    ent->client->ps.weapon = WP_SABER;
                    challenged->client->ps.weapon = WP_SABER;
                    ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER);
                    challenged->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER);
                    ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    challenged->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    //}
                    break;

                case 1:
                default: //FORCE DUEL
                    G_AddEvent(ent, EV_PRIVATE_DUEL, 5);
                    G_AddEvent(challenged, EV_PRIVATE_DUEL, 5);
                    ent->client->ps.stats[STAT_HEALTH] = ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
                    ent->client->ps.stats[STAT_ARMOR] = cm_duelshield.integer;
                    challenged->client->ps.stats[STAT_HEALTH] = challenged->health = challenged->client->ps.stats[STAT_MAX_HEALTH];
                    challenged->client->ps.stats[STAT_ARMOR] = cm_duelshield.integer;
                    ent->client->ps.weapon = WP_SABER;
                    challenged->client->ps.weapon = WP_SABER;
                    ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER);
                    challenged->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER);
                    trap_SendServerCommand( -1, va("print \"%s ^7has become engaged in a ^5fullforce^7 duel with %s!\n\"", challenged->client->pers.netname, ent->client->pers.netname) );
                    WebHook(ent, W_DUELS, va("%s has become engaged in a fullforce duel with %s!", Q_CleanStr(challenged->client->pers.netname), Q_CleanStr(ent->client->pers.netname)));
                    ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    challenged->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_MELEE);
                    break;

                case 2: //TRAINING DUEL
                    G_AddEvent(ent, EV_PRIVATE_DUEL, 3);
                    G_AddEvent(challenged, EV_PRIVATE_DUEL, 3);
                    ent->client->ps.stats[STAT_HEALTH] = 999;
                    ent->client->ps.stats[STAT_ARMOR] = 999;
                    challenged->client->ps.stats[STAT_HEALTH] = 999;
                    challenged->client->ps.stats[STAT_ARMOR] = 999;
                    ent->flags |= FL_GODMODE;
                    challenged->flags |= FL_GODMODE;
                    ent->client->ps.weapon = WP_SABER;
                    challenged->client->ps.weapon = WP_SABER;
                    ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER);
                    challenged->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE) | (1 << WP_SABER);
                    trap_SendServerCommand( ent-g_entities, "print \"^0*^3To end this training session, type in /endduel in console!^0*\n\"" ); 
                    trap_SendServerCommand( challenged-g_entities, "print \"^0*^3To end this training session, type in /endduel in console!^0*\n\"" ); 
                    trap_SendServerCommand( ent-g_entities, "cp \"^3To end this training session\ntype in /endduel in console!\n\"" ); 
                    trap_SendServerCommand( challenged-g_entities, "cp \"^3To end this training session\ntype in /endduel in console!\n\"" ); 
                    trap_SendServerCommand( -1, va("print \"%s ^7has begun a ^3training^7 session with %s^7.\n\"", challenged->client->pers.netname, ent->client->pers.netname) );
                    break;

                case 3: //MELEE DUEL
                    G_AddEvent(ent, EV_PRIVATE_DUEL, 4);
                    G_AddEvent(challenged, EV_PRIVATE_DUEL, 4);
                    ent->client->ps.stats[STAT_MAX_HEALTH] = 200; //200 HP NO ARMOR
                    challenged->client->ps.stats[STAT_MAX_HEALTH] = 200;
                    ent->client->ps.stats[STAT_HEALTH] = ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
                    ent->client->ps.stats[STAT_ARMOR] = 0;
                    challenged->client->ps.stats[STAT_HEALTH] = challenged->health = challenged->client->ps.stats[STAT_MAX_HEALTH];
                    challenged->client->ps.stats[STAT_ARMOR] = 0;
                    ent->client->ps.weapon = WP_MELEE;
                    challenged->client->ps.weapon = WP_MELEE;
                    ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                    challenged->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MELEE);
                    trap_SendServerCommand( -1, va("print \"%s ^7has become engaged in a ^3melee^7 duel with %s!\n\"", challenged->client->pers.netname, ent->client->pers.netname) );
                    WebHook(ent, W_DUELS, va("%s has become engaged in a melee duel with %s!", Q_CleanStr(challenged->client->pers.netname), Q_CleanStr(ent->client->pers.netname)));
                    ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
                    challenged->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);
                    break;
            }

            if (roar_duel_begin_autobow.integer == 1){
                G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_BOW, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
                G_SetAnim(challenged, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_BOW, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
            }

            if ( ent->client->ps.saberHolstered < 2 ){
                if (ent->client->saber[0].soundOff){
                    G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
                }
                if (ent->client->saber[1].soundOff && ent->client->saber[1].model[0]){
                    G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
                }
                ent->client->ps.weaponTime = 400;
                ent->client->ps.saberHolstered = 2;
            }

            if ( challenged->client->ps.saberHolstered < 2 ){
                if (challenged->client->saber[0].soundOff){
                    G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
                }
                if (challenged->client->saber[1].soundOff && challenged->client->saber[1].model[0]){
                    G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
                }
                challenged->client->ps.weaponTime = 400;
                challenged->client->ps.saberHolstered = 2;
            }

            ent->client->ps.fd.forcePower = ent->client->ps.fd.forcePowerMax; //max force power too!
            challenged->client->ps.fd.forcePower = challenged->client->ps.fd.forcePowerMax;

            for ( i = 0; i < NUM_FORCE_POWERS; i++ ) //turn off forcepowers
            {
                if ( ent->client->ps.fd.forcePowerDuration[i] || (ent->client->ps.fd.forcePowersActive&( 1 << i )) )
                {
                    WP_ForcePowerStop( ent, (forcePowers_t)i );
                    ent->client->ps.fd.forcePowerDuration[i] = 0;
                }
                if ( challenged->client->ps.fd.forcePowerDuration[i] || (challenged->client->ps.fd.forcePowersActive&( 1 << i )) )
                {
                    WP_ForcePowerStop( challenged, (forcePowers_t)i );
                    challenged->client->ps.fd.forcePowerDuration[i] = 0;
                }
            }
        }

            else
            {
                //Print the message that a player has been challenged in private, only announce the actual duel initiation in private
                switch (dueltype) {
                    case	0:
                        trap_SendServerCommand( challenged-g_entities, va("cp \"%s\n^7challenged you\nto a ^1saber ^7duel\n\"", ent->client->pers.netname) );
                        trap_SendServerCommand( ent-g_entities, va("cp \"You have challenged\n%s\nto a ^1saber ^7duel\n\"", challenged->client->pers.netname) );
                        break;
                    case	1:
                    default:
                        trap_SendServerCommand( challenged-g_entities, va("cp \"%s\n^7challenged you\nto a ^5fullforce ^7duel\n\"", ent->client->pers.netname) );
                        trap_SendServerCommand( ent-g_entities, va("cp \"You have challenged\n%s\nto a ^5fullforce ^7duel\n\"", challenged->client->pers.netname) );
                        break;
                    case	2:
                        trap_SendServerCommand( challenged-g_entities, va("cp \"%s\n^7challenged you\nto a ^3training ^7session\n\"", ent->client->pers.netname) );
                        trap_SendServerCommand( ent-g_entities, va("cp \"You have challenged\n%s\nto a ^3training ^7session\n\"", challenged->client->pers.netname) );
                        break;
                    case	3:
                        trap_SendServerCommand( challenged-g_entities, va("cp \"%s\n^7challenged you\nto a ^1melee ^7duel\n\"", ent->client->pers.netname) );
                        trap_SendServerCommand( ent-g_entities, va("cp \"You have challenged\n%s\nto a ^1melee ^7duel\n\"", challenged->client->pers.netname) );
                        break;
                }
            }

            challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

            ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;

            ent->client->ps.forceHandExtendTime = level.time + 1000;

            ent->client->ps.duelIndex = challenged->s.number;
            ent->client->ps.duelTime = level.time + 5000;
            dueltypes[ent->client->ps.clientNum] = dueltype;
            //dueltypes[challenged->client->ps.clientNum] = dueltypes[ent->client->ps.clientNum];
        }
    }

#ifndef FINAL_BUILD
    extern stringID_table_t animTable[MAX_ANIMATIONS+1];

    void Cmd_DebugSetSaberMove_f(gentity_t *self)
    {
        int argNum = trap_Argc();
        char arg[MAX_STRING_CHARS];

        if (argNum < 2)
        {
            return;
        }

        trap_Argv( 1, arg, sizeof( arg ) );

        if (!arg[0])
        {
            return;
        }

        self->client->ps.saberMove = atoi(arg);
        self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

        if (self->client->ps.saberMove >= LS_MOVE_MAX)
        {
            self->client->ps.saberMove = LS_MOVE_MAX-1;
        }

        Com_Printf("Anim for move: %s\n", animTable[saberMoveData[self->client->ps.saberMove].animToUse].name);
    }

    void Cmd_DebugSetBodyAnim_f(gentity_t *self, int flags)
    {
        int argNum = trap_Argc();
        char arg[MAX_STRING_CHARS];
        int i = 0;

        if (argNum < 2)
        {
            return;
        }

        trap_Argv( 1, arg, sizeof( arg ) );

        if (!arg[0])
        {
            return;
        }

        while (i < MAX_ANIMATIONS)
        {
            if (!Q_stricmp(arg, animTable[i].name))
            {
                break;
            }
            i++;
        }

        if (i == MAX_ANIMATIONS)
        {
            Com_Printf("Animation '%s' does not exist\n", arg);
            return;
        }

        G_SetAnim(self, NULL, SETANIM_BOTH, i, flags, 0);

        Com_Printf("Set body anim to %s\n", arg);
    }
#endif

    void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
    {
        G_SetAnim(self, NULL, SETANIM_BOTH, anim, flags, 0);
    }

    void DismembermentTest(gentity_t *self);

    void Bot_SetForcedMovement(int bot, int forward, int right, int up);

    extern void DismembermentByNum(gentity_t *self, int num);
#ifndef FINAL_BUILD
    extern void G_SetVehDamageFlags( gentity_t *veh, int shipSurf, int damageLevel );
#endif

    static int G_ClientNumFromNetname(char *name)
    {
        int i = 0;
        gentity_t *ent;

        while (i < MAX_CLIENTS)
        {
            ent = &g_entities[i];

            if (ent->inuse && ent->client &&
                    !Q_stricmp(ent->client->pers.netname, name))
            {
                return ent->s.number;
            }
            i++;
        }

        return -1;
    }

    qboolean TryGrapple(gentity_t *ent)
    {
        if (ent->client->ps.weaponTime > 0)
        { //weapon busy
            return qfalse;
        }
        if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
        { //force power or knockdown or something
            return qfalse;
        }
        if (ent->client->grappleState)
        { //already grappling? but weapontime should be > 0 then..
            return qfalse;
        }

        if (ent->client->ps.weapon != WP_SABER && ent->client->ps.weapon != WP_MELEE)
        {
            return qfalse;
        }

        if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
        {
            Cmd_ToggleSaber_f(ent);
            if (!ent->client->ps.saberHolstered)
            { //must have saber holstered
                return qfalse;
            }
        }

        //G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_PA_1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
        G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_GRAB, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
        if (ent->client->ps.torsoAnim == BOTH_KYLE_GRAB)
        { //providing the anim set succeeded..
            ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
            if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
            {
                ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
            }
            ent->client->ps.weaponTime = ent->client->ps.torsoTimer;
            ent->client->ps.eFlags &= ~EF_INVULNERABLE;
            ent->client->invulnerableTimer = 0;
            //[BugFix35]
            ent->client->dangerTime = level.time;
            //[/BugFix35]
            return qtrue;
        }

        return qfalse;
    }

    //RoAR mod BEGIN
    //#ifndef FINAL_BUILD
    qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
    //#endif

    void sayInsult(gentity_t *ent) {
        char** tokens;

        int totalTokens = 0;
        const char *my_str_literal = cm_insults.string;
        char *str = strdup(my_str_literal);  // We own str's memory now.

        tokens = str_split(&str, ';');

        if (tokens)
        {
            int i;
            for (i = 0; *(tokens + i); i++)
            {
                totalTokens++;
                free(*(tokens + i));
            }
            free(tokens);
        }

        int RandIndex = rand() % totalTokens;
        G_Say(ent, NULL, SAY_ALL, tokens[RandIndex]);

        /*int r = rand() % 100;
          if (r <= 25)
          G_Say(ent, NULL, SAY_ALL, roar_silence_insult_1.string);
          else if (r <= 50)
          G_Say(ent, NULL, SAY_ALL, roar_silence_insult_2.string);
          else if (r <= 75)
          G_Say(ent, NULL, SAY_ALL, roar_silence_insult_3.string);
          else if (r <= 100)
          G_Say(ent, NULL, SAY_ALL, roar_silence_insult_4.string);*/
    }

    /*
       =================
       ClientCommand
       =================
     */
    extern qboolean G_SaberModelSetup(gentity_t *ent);
    void ClientCommand( int clientNum ) {
        gentity_t *ent;
        char	cmd[MAX_TOKEN_CHARS];

        ent = g_entities + clientNum;
        if ( !ent->client ) {
            return;		// not fully in game yet
        }


        trap_Argv( 0, cmd, sizeof( cmd ) );

        //rww - redirect bot commands
        if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
        {
            return;
        }
        //end rww

        if (Q_stricmp (cmd, "say") == 0) {

            if ( ent->client->pers.silent )
            {
                trap_SendServerCommand(ent-g_entities,"print \"You have been silenced by an Admin\n\"");
                trap_SendServerCommand(ent-g_entities,"cp \"You have been silenced by an Admin\n\"");
                return;
            }
            if ( ent->client->pers.ampunish )
            {
                trap_SendServerCommand(ent-g_entities,"print \"You are being punished!\n\"");
                trap_SendServerCommand(ent-g_entities,"cp \"You are being punished!\n\"");
                return;
            }
            if ( ent->client->pers.silent2 )
            {
                sayInsult(ent);
                return;
            }
            Cmd_Say_f (ent, SAY_ALL, qfalse);
            return;
        }
        if (Q_stricmp (cmd, "say_team") == 0) {
            if ( ent->client->pers.silent )
            {
                trap_SendServerCommand(ent-g_entities,"print \"You have been silenced by an Admin\n\"");
                trap_SendServerCommand(ent-g_entities,"cp \"You have been silenced by an Admin\n\"");
                return;
            }
            if ( ent->client->pers.ampunish )
            {
                trap_SendServerCommand(ent-g_entities,"print \"You are being punished!\n\"");
                trap_SendServerCommand(ent-g_entities,"cp \"You are being punished!\n\"");
                return;
            }
            if ( ent->client->pers.silent2 )
            {
                sayInsult(ent);
                return;
            }
            if (g_gametype.integer < GT_TEAM)
            { //not a team game, just refer to regular say.
                if (ent->client->pers.adminchat){
                    Cmd_Say_f (ent, SAY_ADMIN, qfalse);
                }
                else if (ent->client->pers.clanchat){
                    Cmd_Say_f (ent, SAY_CLAN, qfalse);
                }
                else if (ent->client->pers.teamchat){
                    Cmd_Say_f (ent, SAY_ALL, qfalse);
                }
            }
            else
            {
                if (ent->client->pers.adminchat){
                    Cmd_Say_f (ent, SAY_ADMIN, qfalse);
                }
                else if (ent->client->pers.clanchat){
                    Cmd_Say_f (ent, SAY_CLAN, qfalse);
                }

                else if (ent->client->pers.teamchat){
                    Cmd_Say_f (ent, SAY_TEAM, qfalse);
                }
            }
            return;
        }
        if (Q_stricmp (cmd, "tell") == 0) {
            if ( ent->client->pers.silent )
            {
                trap_SendServerCommand(ent-g_entities,"print \"You have been silenced by an Admin\n\"");
                trap_SendServerCommand(ent-g_entities,"cp \"You have been silenced by an Admin\n\"");
                return;
            }
            if ( ent->client->pers.ampunish )
            {
                trap_SendServerCommand(ent-g_entities,"print \"You are being punished!\n\"");
                trap_SendServerCommand(ent-g_entities,"cp \"You are being punished!\n\"");
                return;
            }
            if ( ent->client->pers.silent2 )
            {
                sayInsult(ent);
                return;
            }
            Cmd_Tell_f ( ent );
            return;
        }

        if (Q_stricmp(cmd, "voice_cmd") == 0)
        {
            Cmd_VoiceCommand_f(ent);
            return;
        }

        if (Q_stricmp (cmd, "score") == 0) {
            Cmd_Score_f (ent);
            return;
        }

        // ignore all other commands when at intermission
        if (level.intermissiontime)
        {
            qboolean giveError = qfalse;
            //rwwFIXMEFIXME: This is terrible, write it differently

            if (!Q_stricmp(cmd, "give"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "giveother"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "god"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "notarget"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "noclip"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "kill"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "teamtask"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "levelshot"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "follow"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "follownext"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "followprev"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "team"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "duelteam"))
            {
                giveError = qtrue;
            }
            /*else if (!Q_stricmp(cmd, "clanicon"))
              {
              giveError = qtrue;
              }*/
            else if (!Q_stricmp(cmd, "siegeclass"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "forcechanged"))
            { //special case: still update force change
                Cmd_ForceChanged_f (ent);
                return;
            }
            else if (!Q_stricmp(cmd, "where"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "callvote"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "vote"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "callteamvote"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "teamvote"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "gc"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "setviewpos"))
            {
                giveError = qtrue;
            }
            else if (!Q_stricmp(cmd, "stats"))
            {
                giveError = qtrue;
            }

            if (giveError)
            {
                trap_SendServerCommand( clientNum, va("print \"%s (%s) \n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd ) );
            }
            else
            {
                Cmd_Say_f (ent, qfalse, qtrue);
            }
            return;
        }

        if (Q_stricmp (cmd, "give") == 0)
        {
            Cmd_Give_f (ent, 0);
        }
        else if (Q_stricmp (cmd, "giveother") == 0)
        { //for debugging pretty much
            Cmd_Give_f (ent, 1);
        }
        else if (Q_stricmp (cmd, "t_use") == 0 && CheatsOk(ent))
        { //debug use map object
            if (trap_Argc() > 1)
            {
                char sArg[MAX_STRING_CHARS];
                gentity_t *targ;

                trap_Argv( 1, sArg, sizeof( sArg ) );
                targ = G_Find( NULL, FOFS(targetname), sArg );

                while (targ)
                {
                    if (targ->use)
                    {
                        targ->use(targ, ent, ent);
                    }
                    targ = G_Find( targ, FOFS(targetname), sArg );
                }
            }
        }
        else if (Q_stricmp (cmd, "god") == 0)
            Cmd_God_f (ent);
        else if (Q_stricmp (cmd, "undying") == 0)
            Cmd_Undying_f (ent);
        else if (Q_stricmp (cmd, "notarget") == 0)
            Cmd_Notarget_f (ent);
        else if (Q_stricmp (cmd, "noclip") == 0)
            Cmd_Noclip_f (ent);
        else if (Q_stricmp (cmd, "kill") == 0)
            Cmd_Kill_f (ent);
        else if (Q_stricmp (cmd, "teamtask") == 0)
            Cmd_TeamTask_f (ent);
        else if (Q_stricmp (cmd, "levelshot") == 0)
            Cmd_LevelShot_f (ent);
        else if (Q_stricmp (cmd, "follow") == 0)
            Cmd_Follow_f (ent);
        else if (Q_stricmp (cmd, "follownext") == 0)
            Cmd_FollowCycle_f (ent, 1);
        else if (Q_stricmp (cmd, "followprev") == 0)
            Cmd_FollowCycle_f (ent, -1);
        else if (Q_stricmp (cmd, "team") == 0)
            Cmd_Team_f (ent);
        else if (Q_stricmp (cmd, "duelteam") == 0)
            Cmd_DuelTeam_f (ent);
        /*else if (Q_stricmp (cmd, "clanicon") == 0) //RoAR mod NOTE: Clan icons...
          Cmd_ClanIcon_f (ent);*/
        else if (Q_stricmp (cmd, "siegeclass") == 0)
            Cmd_SiegeClass_f (ent);
        else if (Q_stricmp (cmd, "forcechanged") == 0)
            Cmd_ForceChanged_f (ent);
        else if (Q_stricmp (cmd, "where") == 0)
            Cmd_Where_f (ent);
        else if (Q_stricmp (cmd, "callvote") == 0)
            Cmd_CallVote_f (ent);
        else if (Q_stricmp (cmd, "vote") == 0)
            Cmd_Vote_f (ent);
        else if (Q_stricmp (cmd, "callteamvote") == 0)
            Cmd_CallTeamVote_f (ent);
        else if (Q_stricmp (cmd, "teamvote") == 0)
            Cmd_TeamVote_f (ent);
        else if (Q_stricmp (cmd, "gc") == 0)
            Cmd_GameCommand_f( ent );
        else if (Q_stricmp (cmd, "setviewpos") == 0)
            Cmd_SetViewpos_f( ent );
        else if (Q_stricmp (cmd, "stats") == 0)
            Cmd_Stats_f( ent );
        //
        //RoAR mod COMMAND LINES BEGIN
        else if ((Q_stricmp (cmd, "help") == 0) || (Q_stricmp (cmd, "info") == 0) || (Q_stricmp (cmd, "aminfo") == 0) || (Q_stricmp (cmd, "amhelp") == 0)){
            char   arg1[MAX_STRING_CHARS];
            trap_Argv( 1,  arg1, sizeof( arg1 ) );
            //trap_SendServerCommand( ent-g_entities, "print \"^3===^1BLANK COMMAND^3===\n\n^1DESCRIPTION: ^3BLANK\n\n^5Commands\n\n\"" );
            if(( Q_stricmp( arg1, "freeze" ) == 0 ) || (Q_stricmp( arg1, "amfreeze" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7FREEZE ADMIN COMMAND^5===\n\n^3DESCRIPTION: ^7This command freezes a player solid and holsters their lightsaber. Rendering them unable to move.\n\n^5/Freeze +all  <--- Freeze everyone on server\n/Freeze -all  <--- Unfreeze everyone on server\n/Freeze (client ID)  <--- Freeze a single person by their ID (type in /WHO to see everyones ID)\n/Freeze (client name)  <--- Freeze a single person by their name\n^1To unfreeze a client, you must type in the command again. It's toggled.\nYou can also use... /amfreeze\n\n\"" );
            }
            if(( Q_stricmp( arg1, "whoip" ) == 0 ) || (Q_stricmp( arg1, "amwhoip" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7WHOIP ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3This command displays the IP address of a specific client.\n\n^5/whoip (name)  <--- Display IP address by a clients name\n/whoip (client ID)  <--- Display IP address by a clients ID\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "terminator" ) == 0 ) || (Q_stricmp(arg1, "amterminator") == 0) || (Q_stricmp(arg1, "ammerc") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7TERMINATOR ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3This command makes a player have all weapons and infinite ammo. However, they cannot use force powers.\n\n^5/Terminator  <--- If command is typed in alone, it will perform Terminator on yourself\n/Terminator +all  <--- Terminator everyone on server\n/Terminator -all  <--- Unterminator everyone on server\n/Terminator (client ID)  <--- Terminator a single person by their ID (type in /WHO to see everyones ID)\n/Terminator (client name)  <--- Terminator a single person by their name\n^1To unterminator a client, you must type in the command again. It's toggled.\nYou can also use... /ammerc /amterminator\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "empower" ) == 0 ) || (Q_stricmp(arg1, "amempower") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7EMPOWER ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3This command makes a player have all the force powers and infinite force. However, they cannot use guns.\n\n^5/Empower  <--- If command is typed in alone, it will perform Empower on yourself\n/Empower +all  <--- Empower everyone on server\n/Empower -all  <--- UnEmpower everyone on server\n/Empower (client ID)  <--- Empower a single person by their ID (type in /WHO to see everyones ID)\n/Empower (client name)  <--- Empower a single person by their name\n^1To unempower a client, you must type in the command again. It's toggled.\nYou can also use... /amempower\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "origin" ) == 0 ) || (Q_stricmp(arg1, "amorigin") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7ORIGIN COMMAND^5===\n\n^1DESCRIPTION: ^3Type this in to find out your exact X Y and Z coordinates. An admin can teleport to these using the /teleport admin command.\n\n^5/Origin  <--- Find out your X Y and Z coordinates\n/Origin (client ID)  <--- Find out someone elses X Y and Z coordinates by their ID (type in /WHO to see everyones ID)\n/Origin (client name)  <--- Find out someone elses X Y and Z coordinates by their name\nYou can also use... /amorigin\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "monk" ) == 0 ) || (Q_stricmp(arg1, "ammonk") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7MONK ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3This command makes a player have only melee, but they regenerate +1HP every 1 second. However, they cannot use weapons, or force powers.\n\n^5/Monk  <--- If command is typed in alone, it will perform Monk on yourself\n/Monk +all  <--- Monk everyone on server\n/Monk -all  <--- UnMonk everyone on server\n/Monk (client ID)  <--- Monk a single person by their ID (type in /WHO to see everyones ID)\n/Monk (client name)  <--- Monk a single person by their name\n^1To unmonk a client, you must type in the command again. It's toggled.\nYou can also use... /ammonk\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "silence" ) == 0 ) || (Q_stricmp(arg1, "amsilence") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7SILENCE ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Silence a player on a server, making them silent till they leave the server.\n\n^5/Silence +all  <--- Silence everyone on server\n/Silence -all  <--- Unsilence everyone on server\n/Silence (client ID)  <--- Silence a single person by their ID (type in /WHO to see everyones ID)\n/Silence (client name)  <--- Silence a single person by their name\n^1To unsilence a client, you must type in the command again. It's toggled.\nYou can also use... /amsilence\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "protect" ) == 0 ) || (Q_stricmp(arg1, "amprotect") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7PROTECT ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Protect a player on a server. If the player attacks, they will become unprotected. You cannot unprotect idle-protected people.\n\n^5/Protect  <--- If command is typed in alone, it will perform Protect on yourself\n/Protect +all  <--- Protect everyone on server\n/Protect -all  <--- Unprotect everyone on server\n/Protect (client ID)  <--- Protect a single person by their ID (type in /WHO to see everyones ID)\n/Protect (client name)  <--- Protect a single person by their name\n^1To unprotect a client, you must type in the command again. It's toggled.\nYou can also use... /amprotect\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "demigod" ) == 0 ) || (Q_stricmp(arg1, "amdemigod") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7DEMIGOD ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Turn the character into a demi-god (half god). You can fly through walls, and act as a spectator. You cannot be injured, and cannot attack other people.\n\n^5/DemiGod  <--- If command is typed in alone, it will perform DemiGod on yourself\n/DemiGod +all  <--- DemiGod everyone on server\n/DemiGod -all  <--- UnDemiGod everyone on server\n/DemiGod (client ID)  <--- DemiGod a single person by their ID (type in /WHO to see everyones ID)\n/DemiGod (client name)  <--- DemiGod a single person by their name\n^1To undemigod a client, you must type in the command again. It's toggled.\nYou can also use... /amdemigod\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "splat" ) == 0 ) || (Q_stricmp(arg1, "amsplat") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7SPLAT ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Throw a player into the air, and let them fall to their death.\n\n^5/Splat (client ID)  <--- Splat a single person by their ID (type in /WHO to see everyones ID)\n/Splat (client name)  <--- Splat a single person by their name\nYou can also use... /amsplat\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "sleep" ) == 0 ) || (Q_stricmp(arg1, "amsleep") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7SLEEP ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Make a character fall on the ground, and unable to get up.\n\n^5/Sleep (client ID)  <--- Sleep a single person by their ID (type in /WHO to see everyones ID)\n/Sleep (client name)  <--- Sleep a single person by their name\nYou can also use... /amsleep\n\n\"" );
            }
            else if ((Q_stricmp(arg1, "slap") == 0) || (Q_stricmp(arg1, "amslap") == 0)){
                trap_SendServerCommand(ent-g_entities, "print \"^5===^7SLAP ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Slap a player in to the air.\n\n^5/Slap (client ID)  <--- Slap a single person by their ID (type in /WHO to see everyones ID)\n/Slap (client name)  <--- Slap a single person by their name\nYou can also use... /amslap\n\n\"");
            }
            else if(( Q_stricmp( arg1, "changemode" ) == 0 ) || (Q_stricmp(arg1, "amchangemode") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7CHANGEMODE ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Change the mode in FFA gametypes.\n\n^2Usage: /ChangeMode (mode)\n^5MODES = ClanMatch, ClanMeeting, and Clear\nYou can also use... /amChangeMode\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "punish" ) == 0 ) || (Q_stricmp(arg1, "ampunish") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7PUNISH ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Silence a client, and make them unable to move.\n\n^5/Punish (client ID)  <--- Punish a single person by their ID (type in /WHO to see everyones ID)\n/Punish (client name)  <--- Punish a single person by their name\nYou can also use... /ampunish\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "slay" ) == 0 ) || (Q_stricmp(arg1, "amslay") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7SLAY ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Force a player to kill themselves.\n\n^5/Slay (client ID)  <--- Slay a single person by their ID (type in /WHO to see everyones ID)\n/Slay (client name)  <--- Slay a single person by their name\nYou can also use... /amslay\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "lockteam" ) == 0 ) || (Q_stricmp(arg1, "amlockteam") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7LOCKTEAM ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Lock a team so clients cant join it.\n\n^5/LockTeam (team)\n^3TEAMS = red, blue, spectator, join\nYou can also use... /amLockTeam\n\n\"" );
            }
            else if (( Q_stricmp( arg1, "Teleport" ) == 0 ) || (Q_stricmp( arg1, "Tele") == 0 ) || (Q_stricmp(cmd, "admintele") == 0) || (Q_stricmp(cmd, "amtele") == 0)){
                //cm - Jake NOTE: I smashed both our ideas together. Hows it look?
                //cm - Dom: Great, only Teleport (client id/name) teleports YOU to them not the other way round
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7TELEPORT ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Teleport a client to a certain place.\n\n^5/Teleport (X)(Y)(Z)  <---- Teleport yourself to a certain coordinate (type in /origin to see your current coordinates)\n/Teleport (ID or Name)  <--- Teleport you to a single person by their ID or Name (type in /WHO to see everyones ID)\n/Teleport (client1) (client2) <---- Teleport client1 to client2\n/Teleport (client ID or name) (X) (Y) (Z)  <--- Teleport a client to coordinates\n/teleport <--- Command by itself will teleport you to your telemark^5\nYou can also use... /AmTele /AdminTele /Tele\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "insultsilence" ) == 0 ) || (Q_stricmp( arg1, "aminsultsilence" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7INSULTSILENCE ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Silence a person in a different way. If they do speak, they will automatically say 1 of the 4 defined insults. You can define these in the server.cfg file\n\n^5/InsultSilence (client ID)  <--- InsultSilence a single person by their ID (type in /WHO to see everyones ID)\n/InsultSilence (client name)  <--- InsultSilence a single person by their name\n^1To uninstultsilence a client, you must type in the command again. It's toggled.\nYou can also use... /aminsultsilence\n\n\"" );
            }
            else if( Q_stricmp( arg1, "npc" ) == 0 ){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7NPC ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Spawn an NPC or kill one.\n^5/NPC Spawn (ID)\n/NPC Spawn Vehicle (ID)\n/NPC kill all\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "rename" ) == 0 ) || (Q_stricmp( arg1, "amrename" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7RENAME ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Forcefully rename a player's name.\n\n/Rename (client) (new name)\nYou can also use... /amrename\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "lockname" ) == 0 ) || (Q_stricmp( arg1, "amlockname" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7LOCKNAME ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Stop a player from renaming by locking their name, clan, and rank.\n\n/LockName (client ID or Name)  <--- Lock a clients name.\n^1To unlock a client's name, you must type in the command again. It's toggled.\nYou can also use... /amlockname\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "forceteam" ) == 0 ) || (Q_stricmp( arg1, "amforceteam" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7FORCETEAM ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Force a client to a team.\n\n/ForceTeam (client ID or Name) (team)  <--- Force a client to a team. TEAMS = free, spectator, blue, red\nYou can also use... /amforceteam\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "scale" ) == 0 ) || (Q_stricmp( arg1, "amscale" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7SCALE ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Scale a players character model. MIN = 50, MAX = 200\n^5/Scale (client ID or Name) (scale)  <--- Scale a client by their name or ID, to a certain scale\n- or -\n/scale (scale)\nYou can also use... /amscale\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "csprint" ) == 0 ) || (Q_stricmp( arg1, "amcsprint" ) == 0) || (Q_stricmp( arg1, "ampsay" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7CSPRINT ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3CSPrint stands for Center Screen Print. You can center screen print any message you want to any player you want.\nUSAGE: ^5/CSPrint (player) (message) <--- Sends a message to player. You can use all instead of a client's name to send a message to everyone\nYou can also use... /amcsprint /ampsay\n\n\"" );
            }
            else if(Q_stricmp( arg1, "amkick" ) == 0){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7KICK ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Kick a client out of the server.\nUSAGE: ^5/amKick (client ID or Name)\n\n\"" );
            }
            else if((Q_stricmp( arg1, "amban" ) == 0) || (Q_stricmp( arg1, "ban" ) == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7BAN ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Ban a client from the server.\nUSAGE: ^5/amBan (client ID or Name)\n/addip <--- Add an IP address manually\n/removeip <--- Remove an IP address\n\n\"" );
            }
            else if(Q_stricmp( arg1, "adminban" ) == 0){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7ADMIN BAN ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Ban a client from administration use.\nUSAGE: ^5/adminBan (client ID or Name)\n/addadminip <--- Add an IP address manually\n/removeadminip <--- Remove an IP address\n\n\"" );
            }
            else if(Q_stricmp( arg1, "adminkick" ) == 0){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7ADMIN KICK ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Kick a client from administration use.\nUSAGE: ^5/adminKick (client ID or Name)\nIf you grant temporary administration to the client, you can use this to kick them out.\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "changemap" ) == 0 ) || (Q_stricmp( arg1, "amchangemap" ) == 0) || Q_stricmp( arg1, "ammap" ) == 0){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7CHANGEMAP ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Change the map and gametype.\nUSAGE: ^5/ChangeMap (gametype) (map)\nYou can also use... /amchangemap /ammap\n^1GAMETYPES\n0 = FFA\n1 = HOLOCRON\n2 = JEDIMASTER\n3 = DUEL\n4 = POWER DUEL\n6 = TEAM FFA\n7 = SIEGE\n8 = CTF\n9 = CTY\n\n\"" );
            }
            else if(( Q_stricmp( arg1, "weather" ) == 0 ) || (Q_stricmp(arg1, "setweather") == 0) || (Q_stricmp(arg1, "amweather") == 0)){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7WEATHER ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Change the weather on a map.\n\n^5/Weather (weather)\nWeathers = snow, rain, sandstorm, blizzard, fog, spacedust, acidrain, and clear\n^7You can also use... /amweather /setweather\n\n\"" );
            }
            else if( Q_stricmp( arg1, "addeffect" ) == 0 ){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7ADDEFFECT ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Place an effect on a map perminantly.\n\n^5/AddEffect (fxFile)\n^1WARNING: Once the effect is placed, it requires map restart to stop it!\nEXAMPLE: /addeffect env/small_fire\n^5/ClearEffects <-- if you want to erase the effects you've added.\n/AddEffectTemp <-- if you want to add a temporary effect.\nTo clear effects from the map, you must restart the map or server.\n\n\"" );
            }
            else if( Q_stricmp( arg1, "addmodel" ) == 0 ){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7ADDMODEL ADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3Place a model on a map.\n\n^5/AddModel (model)\n^1WARNING: Once the model is placed, it requires map restart to stop it!\nEXAMPLE: /addmodel models/map_objects/nar_shaddar/reelosdesk.md3\n\n^5/ClearModels <-- if you want to erase the models you've added.\nAddModelTemp <-- if you want to add a temporary effect.\nTo clear models from the map, you must restart the map or server.\n\n\"" );
            }
            else if( Q_stricmp( arg1, "grantadmin" ) == 0 ){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7GRANTADMIN COMMAND^5===\n\n^1DESCRIPTION: ^3This command grants a client an administration level.\n\n^5/GrantAdmin (client) (password)\nUse /adminKick to kick the client out of administration.\n\n\"" );
            }
            else if( Q_stricmp( arg1, "amvstr" ) == 0 ){
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7AMVSTR COMMAND^5===\n\n^3DESCRIPTION: ^7This command executes the vstr.cfg file via server, and executes a variable within it.\n\n^5/amvstr ffa <--- executes the variable ffa, etc.\nYou can edit the vstr.cfg and add your own custom variables in the gamedata/clanmod folder (server only).\n\n\"" );
            }
            else {
                trap_SendServerCommand( ent-g_entities, "print \"^5===^7HELP^5===\n\n^5/AdminCommands^7: see a list of admin commands\n^5/Commands^7: See a list of commands\n^5/Emotes^7: See a list of emotes\n^5/Sayings^7: list of sayings\n^5/AdminGuns^7: List of administration guns\n^5/saberdamages^7: Show server saber damages\n^5/weapondamages^7: Show server weapon damages\n^5/weaponvelocities^7: Show server weapon velocities\n^5/myadmincommands^7: Shows your admin commands available on your administration level\n^5/Manual^7: view in-game manual\n\n^3For more options type in /AMHELP (COMMAND)\n\n\"" );
            }
        }
        else if (Q_stricmp (cmd, "manual") == 0)
        {
            trap_SendServerCommand(ent-g_entities, "uicredits");
        }
        //RoAR mod [Admin CP]
        else if (Q_stricmp (cmd, "AdminCP") == 0)
        {
            trap_SendServerCommand(ent-g_entities, "uiacp");
        }
        else if (Q_stricmp (cmd, "sayings") == 0)
        {
            trap_SendServerCommand( ent-g_entities, "print \"^5===^7SAYINGS^5===\n\n^3DESCRIPTION: ^7Say these like you would any other word. In console type in /say (command)\n\n^5!showmotd, !hidemotd, !freezemotd, !slapme, !jetpack\n !endduel\n\n\"" );
        }
        else if (Q_stricmp (cmd, "adminguns") == 0)
        {
            //if (ent->client->ojpClientPlugIn10){
            trap_SendServerCommand( ent-g_entities, "print \"^5===^7ADMIN GUNS^5===\n\n^3DESCRIPTION: ^7Bind these commands to a key /BIND (key) (gun command). Aim your cross hair at a client, and press the button. It will execute the corresponding admin command on that client.\n\n^5gun_freeze, gun_kick, gun_silence, gun_insult, gun_terminator, gun_empower, gun_splat\ngun_slay, gun_monk, gun_sleep, gun_slap, gun_punish\n\n\"" );
            //}
            //else {
            //trap_SendServerCommand( ent-g_entities, va("print \"^1YOU DO NOT HAVE THE CLAN MOD PLUGIN! WITHOUT IT, YOU CANNOT DO ANY OF THESE COMMANDS! DOWNLOAD IT AT WWW.CLANMOD.ORG/DOWNLOADS\n\"" ) );
            //}
        }
        else if (Q_stricmp(cmd, "cmregister") == 0){
            int client_id = -1;
            char pass[MAX_STRING_CHARS];
            char clean_name[MAX_NETNAME];

            trap_Argv(1, pass, sizeof(pass));
            client_id = ent->client->ps.clientNum;

            if (cm_database.integer <= 0) {
                trap_SendServerCommand(client_id, va("print \"^1Database commands have been disabled on this server.\n\""));
                return;
            }

            Q_strncpyz(clean_name, g_entities[client_id].client->pers.netname, sizeof(clean_name));
            Q_CleanStr(clean_name);

            if (strcmp(clean_name, "Padawan") == 0) {
               trap_SendServerCommand(client_id, va("print \"Users are registered to their player names. Padawan is not allowed.\n\""));
               return;
            }

            if ((trap_Argc() < 2) || (trap_Argc() > 2))
            {
                trap_SendServerCommand(client_id, va("print \"Usage: /cmregister <password>\n\""));
                return;
            }

            if (cm_database.integer == 1) {
                sqliteRegisterUser("INSERT INTO users (user, pass, ipaddress) VALUES ('%s', '%s', '%s')", clean_name, SHA1ThisPass(pass), g_entities[client_id].client->sess.myip);
                trap_SendServerCommand(client_id, va("print \"^3User %s ^3is now registered.\n\"", g_entities[client_id].client->pers.netname));
            }
            else if (cm_database.integer == 2)
                parse_output(ent, va("mysqlRegisterUser curl --data \"key=%s&p=register&user=%s&pass=%s&ipaddress=%s\" %s", cm_mysql_secret.string, clean_name, SHA1ThisPass(pass), g_entities[client_id].client->sess.myip, cm_mysql_url.string));
        }
        else if (Q_stricmp(cmd, "cmleaderboard") == 0 || Q_stricmp(cmd, "cmleaders") == 0) {
            cmLeaders(ent);
        }
        else if (Q_stricmp(cmd, "cmlogin") == 0){
            char *pass[1024];
            trap_Argv(1, pass, sizeof(pass));

            if (cm_database.integer <= 0) {
                trap_SendServerCommand(-1, va("print \"^1Database commands have been disabled on this server.\n\""));
                return;
            }

            if ((trap_Argc() < 2) || (trap_Argc() > 2))
            {
                trap_SendServerCommand(-1, va("print \"Usage: /cmlogin <password>\n\""));
                return;
            }

            cmLogin(ent,pass);
        }
        /*else if (Q_stricmp(cmd, "cmraisestats") == 0) {
          if (cm_database.integer <= 0) {
          trap_SendServerCommand(ent, va("print \"^1Database commands have been disabled on this server.\n\""));
          return;
          }
          if (cm_database.integer == 2)
          parse_output(ent, va("mysqlUpdateStats curl --data \"key=%s&p=increase&g=jedi_academy&c=%s&id=%i\" %s", cm_mysql_secret.string, "kills", ent->client->pers.userID, cm_mysql_url.string));
          }*/
          else if (Q_stricmp(cmd, "cmstats") == 0) {
              char user[MAX_STRING_CHARS];
              int client_id = ent->client->ps.clientNum;
              trap_Argv(1, user, sizeof(user));

              if (cm_database.integer <= 0) {
                  trap_SendServerCommand(client_id, va("print \"^1Database commands have been disabled on this server.\n\""));
                  return;
              }

              if (trap_Argc() > 1)
                  cmStats(ent, user);
              else
                  cmStats(ent, "");
          }
          else if (Q_stricmp (cmd, "emotes") == 0)
          {
              trap_SendServerCommand( ent-g_entities, "print \"^5===^7EMOTES^5===\n\n/dance, /dance2, /dance3, /taunt, /cower, /smack, /swirl\n/kneel, /kneel2, /point, /breakdance, /laydown, /myhead, /cheer\n/sit, /sit2, /slash, /intimidate, /punch, /surrender, /enraged\n/victory, /victory2, /victory3, /headnod, /headshake, /comeon, /kiss\n/hug, /meditate, /amharlem, /amwait, /amhello, /amheal, /amhips\n\n\"" );
          }
          else if (Q_stricmp (cmd, "admincommands") == 0)
          {
              trap_SendServerCommand( ent-g_entities, "print \"^5===^7ADMIN COMMANDS^5===\n\n/amBan, /DemiGod, /GrantAdmin, /Protect, /Terminator, /Slay, /Rename\n/Silence, /InsultSilence, /Splat, /Empower, /Freeze, /NPC, /Scale\n/ChangeMap, /AdminTele, /amKick, /WhoIP, /LockName, /CSPrint\n/ForceTeam, /Monk, /Sleep, /Punish, /Slap, /LockTeam\n/AddEffect, /AddModel, /MyAdminCommands, /ChangeMode, /AdminCP, /AdminBan\n/AdminKick, /AmVSTR\n\n\"" );
          }
          else if (Q_stricmp (cmd, "commands") == 0)
          {
              trap_SendServerCommand( ent-g_entities, "print \"^5===^7COMMANDS^5===\n\n/jetpack <--- put on a jetpack\n/knockmedown <--- knock yourself down\n/drop <--- drop your saber or current weapon\n/showmotd <--- see the MOTD\n/freezemotd <--- show MOTD for infinite time\n/HideMOTD <--- hide the MOTD\n/engage_forceduel <--- duel with force powers\n/engage_meleeduel <--- duel with only melee moves (requires plugin)\n/engage_trainingduel <--- engage in a private training session\n/endduel <--- end training duel session\n/who <--- show all clients + their status\n/chatcolor <--- SAY_ALL in a different color\n/togglechat <--- toggle teamchat mode\n+button12 <--- grappling hook\n/ignore (client) <--- ignore a clients chat text\n/clansay <--- speak to clan members\n/adminsay <--- speak to admins\n/report <--- report something to an admin\n/refuseduels <--- toggle refusing duels on/off\n\"" );
              trap_SendServerCommand(ent-g_entities,"print \"/telemark <--- mark your origin and yaw for /teleport\n/servertime <--- view the server's time\n/saber <--- change your saber(s)\n/cmlogin <--- login to the local DB\n/cmregister <--- register your player name\n/cmstats <--- check your DB stats\n\n\"" );

          }
          //RoAR mod NOTE: Sorry RPG mode. You stunk.
          /*else if (Q_stricmp (cmd, "RPGcommands") == 0)
            {
            trap_SendServerCommand( ent-g_entities, "print \"^3===^1RPG COMMANDS^3===\n\n/BuyMenu <--- View menu of purchasable items\n/Buy (item) <--- Purchase a certain item\n/PvP <--- Toggle Player vs. Player mode on or off\n\n\"" );
            }*/
          else if (Q_stricmp(cmd, "togglechat") == 0){
              if (ent->client->pers.teamchat){
                  if (ent->client->pers.plugindetect == qtrue) {
                      trap_SendServerCommand(ent-g_entities, va("cvar c_chatmode clan"));
                  }
                  ent->client->pers.clanchat = qtrue;
                  ent->client->pers.teamchat = qfalse;
                  ent->client->pers.adminchat = qfalse;
                  trap_SendServerCommand( ent-g_entities, va("print \"^7TEAM CHAT MODE: ^1CLAN\n\"" ) );
              }
              else if (ent->client->pers.clanchat){
                  if (ent->client->pers.plugindetect == qtrue) {
                      trap_SendServerCommand(ent-g_entities, va("cvar c_chatmode admin"));
                  }
                  ent->client->pers.clanchat = qfalse;
                  ent->client->pers.teamchat = qfalse;
                  ent->client->pers.adminchat = qtrue;
                  trap_SendServerCommand( ent-g_entities, va("print \"^7TEAM CHAT MODE: ^3ADMIN\n\"" ) );
              }
              else if (ent->client->pers.adminchat){
                  if (ent->client->pers.plugindetect == qtrue) {
                      trap_SendServerCommand(ent-g_entities, va("cvar c_chatmode team"));
                  }
                  ent->client->pers.clanchat = qfalse;
                  ent->client->pers.teamchat = qtrue;
                  ent->client->pers.adminchat = qfalse;
                  trap_SendServerCommand( ent-g_entities, va("print \"^7TEAM CHAT MODE: ^5TEAM\n\"" ) );
              }
          }
          else if ((Q_stricmp(cmd, "chatcolor") == 0) || (Q_stricmp(cmd, "amchatcolor") == 0)) {
              char	cmd[1024];
              trap_Argv(1, cmd, 1024);
              if (roar_allow_chatColors.integer == 0) {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3Chat colors are disabled.\n\""));
                  return;
              }
              else if (!cmd[0])
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3COLORS = RED, GREEN, YELLOW, BLUE, CYAN, PURPLE, WHITE, and BLACK\n\""));
              }
              else if (Q_stricmp(cmd, "red") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^1RED\n\""));
                  strcpy(ent->client->pers.chatcolor, "red");
              }
              else if (Q_stricmp(cmd, "green") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^2GREEN\n\""));
                  strcpy(ent->client->pers.chatcolor, "green");
              }
              else if (Q_stricmp(cmd, "yellow") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^3YELLOW\n\""));
                  strcpy(ent->client->pers.chatcolor, "yellow");
              }
              else if (Q_stricmp(cmd, "blue") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^4BLUE\n\""));
                  strcpy(ent->client->pers.chatcolor, "blue");
              }
              else if (Q_stricmp(cmd, "cyan") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^5CYAN\n\""));
                  strcpy(ent->client->pers.chatcolor, "cyan");
              }
              else if (Q_stricmp(cmd, "purple") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^6PURPLE\n\""));
                  strcpy(ent->client->pers.chatcolor, "purple");
              }
              else if (Q_stricmp(cmd, "white") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^7WHITE\n\""));
                  strcpy(ent->client->pers.chatcolor, "white");
              }
              else if (Q_stricmp(cmd, "black") == 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"^3CHAT COLOR CHANGED TO ^0BLACK\n\""));
                  strcpy(ent->client->pers.chatcolor, "black");
              }
              //if (ent->client->ojpClientPlugIn10 == qtrue){
              trap_SendServerCommand(ent-g_entities, va("cvar c_chatcolor %s", cmd));
              //}
          }
          else if ((Q_stricmp (cmd, "engage_forceduel") == 0) || (Q_stricmp(cmd, "engage_fullforceduel") == 0))
          {
              if (g_forcePowerDisable.integer == 262143){
                  trap_SendServerCommand( ent-g_entities, va("print \"^7Force powers are disabled on the server!\n\"" ) );
                  return;
              }

              if (roar_enable_ForceDuel.integer == 1)
                  Cmd_EngageDuel_f( ent, 1 );
              else {
                  trap_SendServerCommand( ent-g_entities, va("print \"^7Force duels disabled on this server!\n\"" ) );
                  return;
              }
          }
          else if (Q_stricmp (cmd, "refuseduels") == 0 )
          {
              if (ent->client->pers.refuseduels == 1){
                  ent->client->pers.refuseduels = 0;
                  trap_SendServerCommand( ent-g_entities, va("print \"Duels are now being: ^2ACCEPTED\n\"") );
              } else {
                  ent->client->pers.refuseduels = 1;
                  trap_SendServerCommand( ent-g_entities, va("print \"Duels are now being: ^1REFUSED\n\"") );
              }
          }
          else if (Q_stricmp (cmd, "engage_trainingduel") == 0 )
          {
              if (roar_enable_TrainingDuel.integer == 1)
                  Cmd_EngageDuel_f( ent, 2 );
              else {
                  trap_SendServerCommand( ent-g_entities, va("print \"^7Training duels disabled on this server!\n\"" ) );
                  return;
              }
          }
          else if (Q_stricmp (cmd, "engage_meleeduel") == 0 )
          {
              if (roar_enable_MeleeDuel.integer == 1)
                  Cmd_EngageDuel_f( ent, 3 );
              else {
                  trap_SendServerCommand( ent-g_entities, va("print \"^7Melee duels disabled on this server!\n\"" ) );
                  return;
              }
          }
          else if (Q_stricmp(cmd, "servertime") == 0) {
              char *time;
              time = timestring ( );
              trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", time ) );
              trap_SendServerCommand( ent-g_entities, va("cp \"%s\n\"", time ) );
          }
          /*else if (Q_stricmp (cmd, "kylesmash") == 0)
            {
            TryGrapple(ent);
            }*/
          else if (Q_stricmp(cmd, "saber") == 0 )
          {
              char   saber1[MAX_STRING_CHARS];
              char   saber2[MAX_STRING_CHARS];
              char userinfo[MAX_INFO_STRING];

              trap_Argv( 1, saber1, sizeof( saber1 ) );
              trap_Argv( 2, saber2, sizeof( saber2 ) );

              if ((trap_Argc() < 2) || (trap_Argc() > 3)) 
              { 
                  trap_SendServerCommand( clientNum, va("print \"Usage: /saber <saber1>\nOR /saber <saber1> <saber2>\n\"" ) );
                  return;
              }

              trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

              if (trap_Argc() == 2) {
                  Info_SetValueForKey( userinfo, "saber1", saber1 );
                  Info_SetValueForKey( userinfo, "saber2", "none" );
                  if (ent->client->pers.plugindetect == qtrue)
                      trap_SendServerCommand(clientNum, va("cvar saber2 none"));
              }

              if (trap_Argc() == 3) {
                  Info_SetValueForKey( userinfo, "saber1", saber1 );
                  Info_SetValueForKey( userinfo, "saber2", saber2 );
                  if (ent->client->pers.plugindetect == qtrue)
                      trap_SendServerCommand(clientNum, va("cvar saber2 %s", saber2));
              }
              if (ent->client->pers.plugindetect == qtrue)
                  trap_SendServerCommand(clientNum, va("cvar saber1 %s", saber1));
              trap_SetUserinfo(clientNum, userinfo); 
              ClientUserinfoChanged(clientNum); 
          }
          else if (Q_stricmp(cmd, "clanlogin") == 0 )
          {
              char   password[MAX_STRING_CHARS];

              if (trap_Argc() != 2) 
              { 
                  trap_SendServerCommand( clientNum, va("print \"Usage: /clanlogin <password>\n\"" ) );
                  return;
              }

              if ( ent->r.svFlags & SVF_CLANSAY ) {
                  trap_SendServerCommand( clientNum, "print \"You are already logged in. Type in /clanlogout to remove clan status.\n\"" ); 
                  return; 
              }

              trap_Argv( 1, password, sizeof( password ) ); // password

              if (strcmp(password, cm_clanPassword.string) == 0) {
                  ent->client->pers.iamclan = qtrue;
                  ent->r.svFlags |= SVF_CLANSAY;
                  G_LogPrintf("%s %s\n", ent->client->pers.netname, roar_ClanLogin_saying.string);
                  trap_SendServerCommand(-1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, roar_ClanLogin_saying.string));
                  trap_SendServerCommand(ent-g_entities, va("cvar c_Clanpassword %s", password));
              }
              else
                  trap_SendServerCommand( clientNum, "print \"Incorrect password.\n\"" ); 
          }
          else if ((Q_stricmp(cmd, "adminlogin") == 0 ) || (Q_stricmp(cmd, "amlogin") == 0 ))
          {
              char   password[MAX_STRING_CHARS];

              if (ent->client->sess.adminbanned == 1){
                  trap_SendServerCommand( clientNum, va("print \"You are banned from administration.\n\"" ) );
                  return;
              }

              if (trap_Argc() != 2) 
              { 
                  trap_SendServerCommand( clientNum, va("print \"Usage: /adminlogin <password>\n\"" ) );
                  return;
              }

              if ( ent->r.svFlags & SVF_ADMIN ) {
                  trap_SendServerCommand( clientNum, "print \"You are already logged in. Type in /adminlogout to remove admin status.\n\"" ); 
                  return; 
              }

              trap_Argv( 1, password, sizeof( password ) ); // password

              if (strcmp(password, cm_adminPassword1.string) == 0){
                  ent->client->pers.bitvalue = cm_adminControl1.integer;
                  strcpy(ent->client->pers.login, cm_adminlogin1_saying.string);
                  strcpy(ent->client->pers.logout, cm_adminlogout1_saying.string);
                  ent->client->pers.iamanadmin = 1;
                  ent->r.svFlags |= SVF_ADMIN;
                  G_LogPrintf("%s %s\n", ent->client->pers.netname, ent->client->pers.login);
                  trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, ent->client->pers.login ));
              } else if (strcmp(password, cm_adminPassword2.string) == 0){
                  ent->client->pers.bitvalue = cm_adminControl2.integer;
                  strcpy(ent->client->pers.login, cm_adminlogin2_saying.string);
                  strcpy(ent->client->pers.logout, cm_adminlogout2_saying.string);
                  ent->client->pers.iamanadmin = 2;
                  ent->r.svFlags |= SVF_ADMIN;
                  G_LogPrintf("%s %s\n", ent->client->pers.netname, ent->client->pers.login);
                  trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, ent->client->pers.login ));
              } else if (strcmp(password, cm_adminPassword3.string) == 0){
                  ent->client->pers.bitvalue = cm_adminControl3.integer;
                  strcpy(ent->client->pers.login, cm_adminlogin3_saying.string);
                  strcpy(ent->client->pers.logout, cm_adminlogout3_saying.string);
                  ent->client->pers.iamanadmin = 3;
                  ent->r.svFlags |= SVF_ADMIN;
                  G_LogPrintf("%s %s\n", ent->client->pers.netname, ent->client->pers.login);
                  trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, ent->client->pers.login ));
              } else if (strcmp(password, cm_adminPassword4.string) == 0){
                  ent->client->pers.bitvalue = cm_adminControl4.integer;
                  strcpy(ent->client->pers.login, cm_adminlogin4_saying.string);
                  strcpy(ent->client->pers.logout, cm_adminlogout4_saying.string);
                  ent->client->pers.iamanadmin = 4;
                  ent->r.svFlags |= SVF_ADMIN;
                  G_LogPrintf("%s %s\n", ent->client->pers.netname, ent->client->pers.login);
                  trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, ent->client->pers.login ));
              } else if (strcmp(password, cm_adminPassword5.string) == 0){
                  ent->client->pers.bitvalue = cm_adminControl5.integer;
                  strcpy(ent->client->pers.login, cm_adminlogin5_saying.string);
                  strcpy(ent->client->pers.logout, cm_adminlogout5_saying.string);
                  ent->client->pers.iamanadmin = 5;
                  ent->r.svFlags |= SVF_ADMIN;
                  G_LogPrintf("%s %s\n", ent->client->pers.netname, ent->client->pers.login);
                  trap_SendServerCommand( -1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, ent->client->pers.login ));
              } else
                  trap_SendServerCommand( ent-g_entities, va("print \"Incorrect password.\n\"" ));
              if (ent->client->pers.plugindetect == qtrue)
                  trap_SendServerCommand(ent-g_entities, va("cvar c_Adminpassword %s", password));
          }

          else if ((Q_stricmp(cmd, "telemark") == 0) || (Q_stricmp(cmd, "amtelemark") == 0))
          { // mark the teleportation point with a mark.
              ent->client->ps.viewangles[0] = 0.0f;
              ent->client->ps.viewangles[2] = 0.0f;
              ent->client->pers.amtelemark1 = (int) ent->client->ps.origin[0];
              ent->client->pers.amtelemark2 = (int) ent->client->ps.origin[1];
              ent->client->pers.amtelemark3 = (int) ent->client->ps.origin[2];
              ent->client->pers.amtelemarkyaw = ent->client->ps.viewangles[1];
              ent->client->pers.amtelemarkset = qtrue;
              trap_SendServerCommand( ent-g_entities, va("print \"TELEMARK PLACED AT: ^1X:^7%d, ^1Y:^7%d, ^1Z:^7%d, ^1YAW:^7%d\n\"", (int) ent->client->ps.origin[0], (int) ent->client->ps.origin[1], (int) ent->client->ps.origin[2], ent->client->pers.amtelemarkyaw));
          }
          else if ((Q_stricmp(cmd, "origin") == 0) || (Q_stricmp(cmd, "amorigin") == 0))
          { // teleport to specific location
              //
              int	client_id = -1; 
              char	arg1[MAX_STRING_CHARS];
              trap_Argv( 1, arg1, sizeof( arg1 ) );
              client_id = G_ClientNumberFromArg( arg1 );
              //

              //cm - Dom
              //BugFix: If you gave an ambigious name (e.g. The letter 'a' appears both in XharocK and Alora)
              //to this command it would crash the server.
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

              if (client_id)
              {
                  trap_SendServerCommand( ent-g_entities, va("print \"^1X:^7%d, ^1Y:^7%d, ^1Z:^7%d\n\"", (int) g_entities[client_id].client->ps.origin[0], (int) g_entities[client_id].client->ps.origin[1], (int) g_entities[client_id].client->ps.origin[2]));
                  return;
              }
              else
              {
                  trap_SendServerCommand( ent-g_entities, va("print \"^1X:^7%d, ^1Y:^7%d, ^1Z:^7%d\n\"", (int) ent->client->ps.origin[0], (int) ent->client->ps.origin[1], (int) ent->client->ps.origin[2]));
              }
          }
          else if ((Q_stricmp(cmd, "KnockMeDown") == 0) || (Q_stricmp(cmd, "amKnockMeDown") == 0))
          {
              if (ent->client->ps.weaponTime > 0)
              { //weapon busy
                  return;
              }
              if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
              { //force power or knockdown or something
                  return;
              }
              if (roar_allow_KnockMeDown_command.integer == 0)
              {
                  trap_SendServerCommand( ent-g_entities, va("print \"^7KnockMeDown is disabled on this server!\n\"" ) );
                  return;
              }
              if (ent->client->pers.amdemigod == 1 || ent->client->pers.amhuman == 1
                      || ent->client->pers.ampunish == 1 || ent->client->pers.amsleep == 1 || ent->client->pers.amfreeze == 1){
                  trap_SendServerCommand( ent-g_entities, va("print \"Cannot knock yourself down in your current state.\n\"" ) );
                  return;
              }
              else if (ent->health < 1 || (ent->client->ps.eFlags & EF_DEAD))
              {
              }
              else
              {
                  ent->client->ps.velocity[2] = 375;
                  ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
                  ent->client->ps.forceDodgeAnim = 0;
                  ent->client->ps.weaponTime = 1000;
                  ent->client->ps.forceHandExtendTime = level.time + 700;
              }
          }
          else if (Q_stricmp(cmd, "endduel") == 0)
          {
              endDuel(ent);
          }
          else if (Q_stricmp(cmd, "weapondamages") == 0 ){
              trap_SendServerCommand( ent-g_entities, va("print \"^7E11 BLASTER DAMAGE = %s (20)\nDISRUPTOR MAIN DAMAGE = %s (30)\nDISRUPTOR ALT DAMAGE = %s (100)\nBOWCASTER DAMAGE = %s (50)\nREPEATER DAMAGE = %s (14)\nREPEATER ALT DAMAGE = %s (60)\nREPEATER SPLASH DAMAGE = %s (60)\nDEMP2 DAMAGE = %s (35)\nDEMP2 ALT DAMAGE = %s (8)\nFLECHETTE DAMAGE = %s (12)\nFLECHETTE ALT DAMAGE = %s (60)\nROCKET DAMAGE = %s (100)\nROCKET SPLASH DAMAGE = %s (100)\nCONCUSION DAMAGE = %s (75)\nCONCUSION SPLASH DAMAGE = %s (40)\nCONCUSION ALT DAMAGE = %s (25)\nSTUN BATON DAMAGE = %s (20)\nMELEE SWING1 DAMAGE = %s (10)\nMELEE SWING2 DAMAGE = %s (12)\nTHERMAL DETONATOR DAMAGE = %s (70)\nTHERMAL DETONATOR ALT DAMAGE = %s (60)\nTRIP MINE DAMAGE = %s (100)\nDET PACK DAMAGE = %s (100)\nDET PACK SPLASH DAMAGE = %s (200)\nBRYAR PISTOL DAMAGE = %s (10)\n\"", cm_E11_BLASTER_DAMAGE.string, cm_DISRUPTOR_MAIN_DAMAGE.string, cm_DISRUPTOR_ALT_DAMAGE.string, cm_BOWCASTER_DAMAGE.string, cm_REPEATER_DAMAGE.string, cm_REPEATER_ALT_DAMAGE.string, cm_REPEATER_SPLASH_DAMAGE.string, cm_DEMP2_DAMAGE.string, cm_DEMP2_ALT_DAMAGE.string, cm_FLECHETTE_DAMAGE.string, cm_FLECHETTE_ALT_DAMAGE.string, cm_ROCKET_DAMAGE.string, cm_ROCKET_SPLASH_DAMAGE.string, cm_CONCUSION_DAMAGE.string, cm_CONCUSION_SPLASH_DAMAGE.string, cm_CONCUSION_ALT_DAMAGE.string, cm_STUN_BATON_DAMAGE.string, cm_MELEE_SWING1_DAMAGE.string, cm_MELEE_SWING2_DAMAGE.string, cm_THERMAL_DETONATOR_DAMAGE.string, cm_THERMAL_DETONATOR_ALT_DAMAGE.string, cm_TRIP_MINE_DAMAGE.string, cm_DET_PACK_DAMAGE.string, cm_DET_PACK_SPLASH_DAMAGE.string, cm_BRYAR_PISTOL_DAMAGE.string ) );
          }
          else if (Q_stricmp(cmd, "weaponvelocities") == 0 ){
              trap_SendServerCommand( ent-g_entities, va("print \"^7E11 BLASTER VELOCITY = %s (2300)\nBOWCASTER VELOCITY = %s (1300)\nREPEATER VELOCITY = %s (1600)\nDEMP2 VELOCITY = %s (1800)\nFLECHETTE VELOCITY = %s (3500)\nROCKET VELOCITY = %s (900)\nCONCUSION VELOCITY = %s (3000)\nTHERMAL DETONATOR VELOCITY = %s (900)\nBRYAR PISTOL VELOCITY = %s (1600)\n\"", cm_E11_BLASTER_VELOCITY.string, cm_BOWCASTER_VELOCITY.string, cm_REPEATER_VELOCITY.string, cm_DEMP2_VELOCITY.string, cm_FLECHETTE_VELOCITY.string, cm_ROCKET_VELOCITY.string, cm_CONCUSION_VELOCITY.string, cm_THERMAL_DETONATOR_VELOCITY.string, cm_BRYAR_PISTOL_VELOCITY.string ) );
          }
          else if (Q_stricmp(cmd, "saberdamages") == 0 ){
              trap_SendServerCommand( ent-g_entities, va("print \"^7Saber Twirl = %s (10)\nSaber Kick = %s (2)\nSaber Dual Kata = %s (90)\nStaff Kata Min. = %s (60)\nSaber Staff Kata Max. = %s (70)\nSaber Multi Min. = %s (2)\nSaber Multi Max. = %s (70)\nSaber Special Min. = %s (2)\nSaber Special Max. = %s (90)\nSaber Red Normal = %s (100)\nSaber Red Normal Min. = %s (2)\nSaber Red Normal Max. = %s (120)\nSaber Red DFA Min. = %s (2)\nSaber Red DFA Max. = %s (180)\nSaber Red Back Min. = %s (2)\nSaber Red Back Max. = %s (30)\nSaber Yellow Normal = %s (60)\nSaber Yellow Overhead Min. = %s (2)\nSaber Yellow Overhead Max. = %s (80)\nSaber Yellow Back Min. = %s (2)\nSaber Yellow Back Max. = %s (25)\nSaber Blue Normal = %s (35)\nSaber Blue Lunge Min. = %s (2)\nSaber Blue Lunge Max. = %s (30)\nSaber Blue Back Min. = %s (2)\nSaber Blue Back Max. = %s (30)\n\"", g_mSaberDMGTwirl.string, g_mSaberDMGKick.string, g_mSaberDMGDualKata.string, g_mSaberDMGStaffKataMin.string, g_mSaberDMGStaffKataMax.string, g_mSaberDMGMultiMin.string, g_mSaberDMGMultiMax.string, g_mSaberDMGSpecialMin.string, g_mSaberDMGSpecialMax.string, g_mSaberDMGRedNormal.string, g_mSaberDMGRedNormalMin.string, g_mSaberDMGRedNormalMax.string, g_mSaberDMGRedDFAMin.string, g_mSaberDMGRedDFAMax.string, g_mSaberDMGRedBackMin.string, g_mSaberDMGRedBackMax.string, g_mSaberDMGYellowNormal.string, g_mSaberDMGYellowOverheadMin.string, g_mSaberDMGYellowOverheadMax.string, g_mSaberDMGYellowBackMin.string, g_mSaberDMGYellowBackMax.string, g_mSaberDMGBlueNormal.string, g_mSaberDMGBlueLungeMin.string, g_mSaberDMGBlueLungeMax.string, g_mSaberDMGBlueBackMin.string, g_mSaberDMGBlueBackMax.string ) );
          }
          else if (Q_stricmp(cmd, "hidemotd") == 0)
          {
              hideMOTD(ent);
          }
          else if (Q_stricmp(cmd, "showmotd") == 0)
          {
              showMOTD(ent);
          }
          else if (Q_stricmp(cmd, "freezemotd") == 0)
          {
              cmfreezeMOTD(ent);
          }
          // MJN - Ignore // FULL CREDIT TO MJN!
          else if (Q_stricmp(cmd, "ignore") == 0) {
              int ignoree = -1;
              qboolean ignore;
              char   name[MAX_STRING_CHARS];
              if (trap_Argc() != 2) {
                  trap_SendServerCommand(ent-g_entities, "print \"Usage: ignore <client>\n\"");
                  return;
              }
              trap_Argv(1, name, sizeof(name));
              ignoree = G_ClientNumberFromArg(name);
              if (ignoree == -1)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"Can't find client ID for %s\n\"", name));
                  return;
              }
              if (ignoree == -2)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"Ambiguous client ID for %s\n\"", name));
                  return;
              }
              if (ignoree >= MAX_CLIENTS || ignoree < 0)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"Bad client ID for %s\n\"", name));
                  return;
              }
              if (!g_entities[ignoree].inuse)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"Client %s is not active\n\"", name));
                  return;
              }
              ignore = G_IsClientChatIgnored(ent->client->ps.clientNum, ignoree) ? qfalse : qtrue;
              if (ignoree == ent->client->ps.clientNum)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"You cant ignore yourself.\n\""));
                  return;
              }
              G_IgnoreClientChat(ent->client->ps.clientNum, ignoree, ignore);
              if (ignore)
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"%s ^7is now being ignored.\n\"", g_entities[ignoree].client->pers.netname));
                  trap_SendServerCommand(ignoree, va("print \"%s ^7is now ignoring you.\n\"", ent->client->pers.netname));
              }
              else
              {
                  trap_SendServerCommand(ent-g_entities, va("print \"%s ^7is now unignored.\n\"", g_entities[ignoree].client->pers.netname));
                  trap_SendServerCommand(ignoree, va("print \"%s ^7has unignored you.\n\"", ent->client->pers.netname));
              }
          }

          else if (Q_stricmp(cmd, "ordernpc") == 0)
          {
              Cmd_OrderNPC_f(ent);
          }
          else if (Q_stricmp(cmd, "clansay") == 0)
          {
              char		*p;

              p = ConcatArgs( 1 );

              if (strlen( p ) > MAX_SAY_TEXT) {
                  return;
              }

              G_Say( ent, NULL, SAY_CLAN, p );
              G_LogPrintf("ClanSay(%s): %s\n", ent->client->pers.netname, p);
          }
          else if ((Q_stricmp(cmd, "adminsay") == 0) || (Q_stricmp(cmd, "amsay") == 0))
          {
              char		*p;

              p = ConcatArgs( 1 );

              if (strlen( p ) > MAX_SAY_TEXT) {
                  return;
              }

              G_Say( ent, NULL, SAY_ADMIN, p );
              G_LogPrintf("AdminSay(%s): %s\n", ent->client->pers.netname, p);
          }
          else if (Q_stricmp(cmd, "report") == 0)
          {
              char		*p;

              p = ConcatArgs( 1 );

              if (strlen( p ) > MAX_SAY_TEXT) {
                  return;
              }

              if (cm_allow_report_command.integer == 0){
                  trap_SendServerCommand( ent-g_entities, va("print \"The report command is disabled on this server.\n\"") );
                  return;
              }

              G_Say( ent, NULL, SAY_REPORT, p );
              G_LogPrintf("(%s)Reported: %s\n", ent->client->pers.netname, p);
          }

          else if ((Q_stricmp(cmd, "jetpack") == 0) || (Q_stricmp(cmd, "amjetpack") == 0))
          {
              cmJetpack(ent);
          }
          //[HolocronFiles]
          else if (Q_stricmp(cmd, "addholocron") == 0 && bot_wp_edit.integer >= 1)
          {// Add a new holocron point. Unique1 added.
              AOTCTC_Holocron_Add ( ent );
          }
          else if (Q_stricmp(cmd, "saveholocrons") == 0 && bot_wp_edit.integer >= 1)
          {// Save holocron position table. Unique1 added.
              AOTCTC_Holocron_Savepositions();		
          }
          else if (Q_stricmp(cmd, "spawnholocron") == 0 && bot_wp_edit.integer >= 1)
          {// Spawn a holocron... Unique1 added.
              AOTCTC_Create_Holocron( rand()%18, ent->r.currentOrigin );
          }
          //[/HolocronFiles]
          else if ((Q_stricmp(cmd, "amdrop") == 0) || (Q_stricmp(cmd, "Drop") == 0) || (Q_stricmp(cmd, "DropSaber") == 0) || (Q_stricmp(cmd, "amDropSaber") == 0))
          {
              vec3_t vecnorm;
              if (roar_allow_dropsaber_command.integer == 0){
                  trap_SendServerCommand( ent-g_entities, va("print \"Drop is disabled on this server!\n\"" ) );
                  return;
              }
              if (ent->client->pers.amdemigod == 1 || ent->client->pers.amhuman == 1
                      || ent->client->pers.ampunish == 1 || ent->client->pers.amsleep == 1 || ent->client->pers.amfreeze == 1){
                  trap_SendServerCommand( ent-g_entities, va("print \"Cannot drop weapon in your current state.\n\"" ) );
                  return;
              }
              if (ent->client->ps.weapon == WP_SABER &&
                      ent->client->ps.saberEntityNum &&
                      ent->client->ps.saberInFlight)
              {
                  return;
              }
              if (ent->client->ps.weapon == WP_SABER &&
                      ent->client->ps.saberEntityNum &&
                      !ent->client->ps.saberInFlight)
              {
                  saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
              }
              else {
                  TossClientWeapon(ent, vecnorm, 500);
              }
          }
          //for convenient powerduel testing in release
          else if (Q_stricmp(cmd, "killother") == 0 && CheatsOk( ent ))
          {
              if (trap_Argc() > 1)
              {
                  char sArg[MAX_STRING_CHARS];
                  int entNum = 0;

                  trap_Argv( 1, sArg, sizeof( sArg ) );

                  entNum = G_ClientNumFromNetname(sArg);

                  if (entNum >= 0 && entNum < MAX_GENTITIES)
                  {
                      gentity_t *kEnt = &g_entities[entNum];

                      if (kEnt->inuse && kEnt->client)
                      {
                          kEnt->flags &= ~FL_GODMODE;
                          kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
                          player_die (kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);
                      }
                  }
              }
          }
#ifdef _DEBUG
          else if (Q_stricmp(cmd, "relax") == 0 && CheatsOk( ent ))
          {
              if (ent->client->ps.eFlags & EF_RAG)
              {
                  ent->client->ps.eFlags &= ~EF_RAG;
              }
              else
              {
                  ent->client->ps.eFlags |= EF_RAG;
              }
          }
          else if (Q_stricmp(cmd, "holdme") == 0 && CheatsOk( ent ))
          {
              if (trap_Argc() > 1)
              {
                  char sArg[MAX_STRING_CHARS];
                  int entNum = 0;

                  trap_Argv( 1, sArg, sizeof( sArg ) );

                  entNum = atoi(sArg);

                  if (entNum >= 0 &&
                          entNum < MAX_GENTITIES)
                  {
                      gentity_t *grabber = &g_entities[entNum];

                      if (grabber->inuse && grabber->client && grabber->ghoul2)
                      {
                          if (!grabber->s.number)
                          { //switch cl 0 and entitynum_none, so we can operate on the "if non-0" concept
                              ent->client->ps.ragAttach = ENTITYNUM_NONE;
                          }
                          else
                          {
                              ent->client->ps.ragAttach = grabber->s.number;
                          }
                      }
                  }
              }
              else
              {
                  ent->client->ps.ragAttach = 0;
              }
          }
          else if (Q_stricmp(cmd, "limb_break") == 0 && CheatsOk( ent ))
          {
              if (trap_Argc() > 1)
              {
                  char sArg[MAX_STRING_CHARS];
                  int breakLimb = 0;

                  trap_Argv( 1, sArg, sizeof( sArg ) );
                  if (!Q_stricmp(sArg, "right"))
                  {
                      breakLimb = BROKENLIMB_RARM;
                  }
                  else if (!Q_stricmp(sArg, "left"))
                  {
                      breakLimb = BROKENLIMB_LARM;
                  }

                  G_BreakArm(ent, breakLimb);
              }
          }
          else if (Q_stricmp(cmd, "headexplodey") == 0 && CheatsOk( ent ))
          {
              Cmd_Kill_f (ent);
              if (ent->health < 1)
              {
                  DismembermentTest(ent);
              }
          }
          else if (Q_stricmp(cmd, "debugstupidthing") == 0 && CheatsOk( ent ))
          {
              int i = 0;
              gentity_t *blah;
              while (i < MAX_GENTITIES)
              {
                  blah = &g_entities[i];
                  if (blah->inuse && blah->classname && blah->classname[0] && !Q_stricmp(blah->classname, "NPC_Vehicle"))
                  {
                      Com_Printf("Found it.\n");
                  }
                  i++;
              }
          }
          else if (Q_stricmp(cmd, "arbitraryprint") == 0 && CheatsOk( ent ))
          {
              trap_SendServerCommand( -1, va("cp \"Blah blah blah\n\""));
          }
          else if (Q_stricmp(cmd, "handcut") == 0 && CheatsOk( ent ))
          {
              int bCl = 0;
              char sarg[MAX_STRING_CHARS];

              if (trap_Argc() > 1)
              {
                  trap_Argv( 1, sarg, sizeof( sarg ) );

                  if (sarg[0])
                  {
                      bCl = atoi(sarg);

                      if (bCl >= 0 && bCl < MAX_GENTITIES)
                      {
                          gentity_t *hEnt = &g_entities[bCl];

                          if (hEnt->client)
                          {
                              if (hEnt->health > 0)
                              {
                                  gGAvoidDismember = 1;
                                  hEnt->flags &= ~FL_GODMODE;
                                  hEnt->client->ps.stats[STAT_HEALTH] = hEnt->health = -999;
                                  player_die (hEnt, hEnt, hEnt, 100000, MOD_SUICIDE);
                              }
                              gGAvoidDismember = 2;
                              G_CheckForDismemberment(hEnt, ent, hEnt->client->ps.origin, 999, hEnt->client->ps.legsAnim, qfalse);
                              gGAvoidDismember = 0;
                          }
                      }
                  }
              }
          }
          else if (Q_stricmp(cmd, "loveandpeace") == 0 && CheatsOk(ent))
          {
              trace_t tr;
              vec3_t fPos;

              AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

              fPos[0] = ent->client->ps.origin[0] + fPos[0] * 40;
              fPos[1] = ent->client->ps.origin[1] + fPos[1] * 40;
              fPos[2] = ent->client->ps.origin[2] + fPos[2] * 40;

              trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

              if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
              {
                  gentity_t *other = &g_entities[tr.entityNum];

                  if (other && other->inuse && other->client)
                  {
                      vec3_t entDir;
                      vec3_t otherDir;
                      vec3_t entAngles;
                      vec3_t otherAngles;

                      if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
                      {
                          Cmd_ToggleSaber_f(ent);
                      }

                      if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered)
                      {
                          Cmd_ToggleSaber_f(other);
                      }

                      if ((ent->client->ps.weapon != WP_SABER || ent->client->ps.saberHolstered) &&
                              (other->client->ps.weapon != WP_SABER || other->client->ps.saberHolstered))
                      {
                          VectorSubtract(other->client->ps.origin, ent->client->ps.origin, otherDir);
                          VectorCopy(ent->client->ps.viewangles, entAngles);
                          entAngles[YAW] = vectoyaw(otherDir);
                          SetClientViewAngle(ent, entAngles);

                          StandardSetBodyAnim(ent, /*BOTH_KISSER1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
                          ent->client->ps.saberMove = LS_NONE;
                          ent->client->ps.saberBlocked = 0;
                          ent->client->ps.saberBlocking = 0;

                          VectorSubtract(ent->client->ps.origin, other->client->ps.origin, entDir);
                          VectorCopy(other->client->ps.viewangles, otherAngles);
                          otherAngles[YAW] = vectoyaw(entDir);
                          SetClientViewAngle(other, otherAngles);

                          StandardSetBodyAnim(other, /*BOTH_KISSEE1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
                          other->client->ps.saberMove = LS_NONE;
                          other->client->ps.saberBlocked = 0;
                          other->client->ps.saberBlocking = 0;
                      }
                  }
              }
          }
#endif
          else if (Q_stricmp(cmd, "thedestroyer") == 0 && CheatsOk( ent ) && ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
          {
              Cmd_ToggleSaber_f(ent);

              if (!ent->client->ps.saberHolstered)
              {
              }
          }
          //begin bot debug cmds
          else if (Q_stricmp(cmd, "debugBMove_Forward") == 0 && CheatsOk(ent))
          {
              int arg = 4000;
              int bCl = 0;
              char sarg[MAX_STRING_CHARS];

              assert(trap_Argc() > 1);
              trap_Argv( 1, sarg, sizeof( sarg ) );

              assert(sarg[0]);
              bCl = atoi(sarg);
              Bot_SetForcedMovement(bCl, arg, -1, -1);
          }
          else if (Q_stricmp(cmd, "debugBMove_Back") == 0 && CheatsOk(ent))
          {
              int arg = -4000;
              int bCl = 0;
              char sarg[MAX_STRING_CHARS];

              assert(trap_Argc() > 1);
              trap_Argv( 1, sarg, sizeof( sarg ) );

              assert(sarg[0]);
              bCl = atoi(sarg);
              Bot_SetForcedMovement(bCl, arg, -1, -1);
          }
          else if (Q_stricmp(cmd, "debugBMove_Right") == 0 && CheatsOk(ent))
          {
              int arg = 4000;
              int bCl = 0;
              char sarg[MAX_STRING_CHARS];

              assert(trap_Argc() > 1);
              trap_Argv( 1, sarg, sizeof( sarg ) );

              assert(sarg[0]);
              bCl = atoi(sarg);
              Bot_SetForcedMovement(bCl, -1, arg, -1);
          }
          else if (Q_stricmp(cmd, "debugBMove_Left") == 0 && CheatsOk(ent))
          {
              int arg = -4000;
              int bCl = 0;
              char sarg[MAX_STRING_CHARS];

              assert(trap_Argc() > 1);
              trap_Argv( 1, sarg, sizeof( sarg ) );

              assert(sarg[0]);
              bCl = atoi(sarg);
              Bot_SetForcedMovement(bCl, -1, arg, -1);
          }
          else if (Q_stricmp(cmd, "debugBMove_Up") == 0 && CheatsOk(ent))
          {
              int arg = 4000;
              int bCl = 0;
              char sarg[MAX_STRING_CHARS];

              assert(trap_Argc() > 1);
              trap_Argv( 1, sarg, sizeof( sarg ) );

              assert(sarg[0]);
              bCl = atoi(sarg);
              Bot_SetForcedMovement(bCl, -1, -1, arg);
          }
          //end bot debug cmds
#ifndef FINAL_BUILD
          else if (Q_stricmp(cmd, "debugSetSaberMove") == 0 && CheatsOk( ent ))
          {
              Cmd_DebugSetSaberMove_f(ent);
          }
          else if (Q_stricmp(cmd, "debugSetBodyAnim") == 0 && CheatsOk( ent ))
          {
              Cmd_DebugSetBodyAnim_f(ent, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
          }
          else if (Q_stricmp(cmd, "debugDismemberment") == 0 && CheatsOk( ent ))
          {
              Cmd_Kill_f (ent);
              if (ent->health < 1)
              {
                  char	arg[MAX_STRING_CHARS];
                  int		iArg = 0;

                  if (trap_Argc() > 1)
                  {
                      trap_Argv( 1, arg, sizeof( arg ) );

                      if (arg[0])
                      {
                          iArg = atoi(arg);
                      }
                  }

                  DismembermentByNum(ent, iArg);
              }
          }
          else if (Q_stricmp(cmd, "debugDropSaber") == 0 && CheatsOk( ent ))
          {
              if (ent->client->ps.weapon == WP_SABER &&
                      ent->client->ps.saberEntityNum &&
                      !ent->client->ps.saberInFlight)
              {
                  saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
              }
          }
          else if (Q_stricmp(cmd, "debugKnockMeDown") == 0 && CheatsOk( ent ))
          {
              if (BG_KnockDownable(&ent->client->ps))
              {
                  ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
                  ent->client->ps.forceDodgeAnim = 0;
                  if (trap_Argc() > 1)
                  {
                      ent->client->ps.forceHandExtendTime = level.time + 1100;
                      ent->client->ps.quickerGetup = qfalse;
                  }
                  else
                  {
                      ent->client->ps.forceHandExtendTime = level.time + 700;
                      ent->client->ps.quickerGetup = qtrue;
                  }
              }
          }
          else if (Q_stricmp(cmd, "debugSaberSwitch") == 0 && CheatsOk( ent ))
          {
              gentity_t *targ = NULL;

              if (trap_Argc() > 1)
              {
                  char	arg[MAX_STRING_CHARS];

                  trap_Argv( 1, arg, sizeof( arg ) );

                  if (arg[0])
                  {
                      int x = atoi(arg);

                      if (x >= 0 && x < MAX_CLIENTS)
                      {
                          targ = &g_entities[x];
                      }
                  }
              }

              if (targ && targ->inuse && targ->client)
              {
                  Cmd_ToggleSaber_f(targ);
              }
          }
          else if (Q_stricmp(cmd, "debugIKGrab") == 0 && CheatsOk( ent ))
          {
              gentity_t *targ = NULL;

              if (trap_Argc() > 1)
              {
                  char	arg[MAX_STRING_CHARS];

                  trap_Argv( 1, arg, sizeof( arg ) );

                  if (arg[0])
                  {
                      int x = atoi(arg);

                      if (x >= 0 && x < MAX_CLIENTS)
                      {
                          targ = &g_entities[x];
                      }
                  }
              }

              if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
              {
                  targ->client->ps.heldByClient = ent->s.number+1;
              }
          }
          else if (Q_stricmp(cmd, "debugIKBeGrabbedBy") == 0 && CheatsOk( ent ))
          {
              gentity_t *targ = NULL;

              if (trap_Argc() > 1)
              {
                  char	arg[MAX_STRING_CHARS];

                  trap_Argv( 1, arg, sizeof( arg ) );

                  if (arg[0])
                  {
                      int x = atoi(arg);

                      if (x >= 0 && x < MAX_CLIENTS)
                      {
                          targ = &g_entities[x];
                      }
                  }
              }

              if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
              {
                  ent->client->ps.heldByClient = targ->s.number+1;
              }
          }
          else if (Q_stricmp(cmd, "debugIKRelease") == 0 && CheatsOk( ent ))
          {
              gentity_t *targ = NULL;

              if (trap_Argc() > 1)
              {
                  char	arg[MAX_STRING_CHARS];

                  trap_Argv( 1, arg, sizeof( arg ) );

                  if (arg[0])
                  {
                      int x = atoi(arg);

                      if (x >= 0 && x < MAX_CLIENTS)
                      {
                          targ = &g_entities[x];
                      }
                  }
              }

              if (targ && targ->inuse && targ->client)
              {
                  targ->client->ps.heldByClient = 0;
              }
          }
          else if (Q_stricmp(cmd, "debugThrow") == 0 && CheatsOk( ent ))
          {
              trace_t tr;
              vec3_t tTo, fwd;

              if (ent->client->ps.weaponTime > 0 || ent->client->ps.forceHandExtend != HANDEXTEND_NONE ||
                      ent->client->ps.groundEntityNum == ENTITYNUM_NONE || ent->health < 1)
              {
                  return;
              }

              AngleVectors(ent->client->ps.viewangles, fwd, 0, 0);
              tTo[0] = ent->client->ps.origin[0] + fwd[0]*32;
              tTo[1] = ent->client->ps.origin[1] + fwd[1]*32;
              tTo[2] = ent->client->ps.origin[2] + fwd[2]*32;

              trap_Trace(&tr, ent->client->ps.origin, 0, 0, tTo, ent->s.number, MASK_PLAYERSOLID);

              if (tr.fraction != 1)
              {
                  gentity_t *other = &g_entities[tr.entityNum];

                  if (other->inuse && other->client && other->client->ps.forceHandExtend == HANDEXTEND_NONE &&
                          other->client->ps.groundEntityNum != ENTITYNUM_NONE && other->health > 0 &&
                          (int)ent->client->ps.origin[2] == (int)other->client->ps.origin[2])
                  {
                      float pDif = 40.0f;
                      vec3_t entAngles, entDir;
                      vec3_t otherAngles, otherDir;
                      vec3_t intendedOrigin;
                      vec3_t boltOrg, pBoltOrg;
                      vec3_t tAngles, vDif;
                      vec3_t fwd, right;
                      trace_t tr;
                      trace_t tr2;

                      VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
                      VectorCopy( ent->client->ps.viewangles, entAngles );
                      entAngles[YAW] = vectoyaw( otherDir );
                      SetClientViewAngle( ent, entAngles );

                      ent->client->ps.forceHandExtend = HANDEXTEND_PRETHROW;
                      ent->client->ps.forceHandExtendTime = level.time + 5000;

                      ent->client->throwingIndex = other->s.number;
                      ent->client->doingThrow = level.time + 5000;
                      ent->client->beingThrown = 0;

                      VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
                      VectorCopy( other->client->ps.viewangles, otherAngles );
                      otherAngles[YAW] = vectoyaw( entDir );
                      SetClientViewAngle( other, otherAngles );

                      other->client->ps.forceHandExtend = HANDEXTEND_PRETHROWN;
                      other->client->ps.forceHandExtendTime = level.time + 5000;

                      other->client->throwingIndex = ent->s.number;
                      other->client->beingThrown = level.time + 5000;
                      other->client->doingThrow = 0;

                      //Doing this now at a stage in the throw, isntead of initially.
                      //other->client->ps.heldByClient = ent->s.number+1;

                      G_EntitySound( other, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
                      G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*jump1.wav") );
                      G_Sound(other, CHAN_AUTO, G_SoundIndex( "sound/movers/objects/objectHit.wav" ));

                      //see if we can move to be next to the hand.. if it's not clear, break the throw.
                      VectorClear(tAngles);
                      tAngles[YAW] = ent->client->ps.viewangles[YAW];
                      VectorCopy(ent->client->ps.origin, pBoltOrg);
                      AngleVectors(tAngles, fwd, right, 0);
                      boltOrg[0] = pBoltOrg[0] + fwd[0]*8 + right[0]*pDif;
                      boltOrg[1] = pBoltOrg[1] + fwd[1]*8 + right[1]*pDif;
                      boltOrg[2] = pBoltOrg[2];

                      VectorSubtract(boltOrg, pBoltOrg, vDif);
                      VectorNormalize(vDif);

                      VectorClear(other->client->ps.velocity);
                      intendedOrigin[0] = pBoltOrg[0] + vDif[0]*pDif;
                      intendedOrigin[1] = pBoltOrg[1] + vDif[1]*pDif;
                      intendedOrigin[2] = other->client->ps.origin[2];

                      trap_Trace(&tr, intendedOrigin, other->r.mins, other->r.maxs, intendedOrigin, other->s.number, other->clipmask);
                      trap_Trace(&tr2, ent->client->ps.origin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, CONTENTS_SOLID);

                      if (tr.fraction == 1.0 && !tr.startsolid && tr2.fraction == 1.0 && !tr2.startsolid)
                      {
                          VectorCopy(intendedOrigin, other->client->ps.origin);
                      }
                      else
                      { //if the guy can't be put here then it's time to break the throw off.
                          vec3_t oppDir;
                          int strength = 4;

                          other->client->ps.heldByClient = 0;
                          other->client->beingThrown = 0;
                          ent->client->doingThrow = 0;

                          ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
                          G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

                          other->client->ps.forceHandExtend = HANDEXTEND_NONE;
                          VectorSubtract(other->client->ps.origin, ent->client->ps.origin, oppDir);
                          VectorNormalize(oppDir);
                          other->client->ps.velocity[0] = oppDir[0]*(strength*40);
                          other->client->ps.velocity[1] = oppDir[1]*(strength*40);
                          other->client->ps.velocity[2] = 150;

                          VectorSubtract(ent->client->ps.origin, other->client->ps.origin, oppDir);
                          VectorNormalize(oppDir);
                          ent->client->ps.velocity[0] = oppDir[0]*(strength*40);
                          ent->client->ps.velocity[1] = oppDir[1]*(strength*40);
                          ent->client->ps.velocity[2] = 150;
                      }
                  }
              }
          }
#endif
#ifdef VM_MEMALLOC_DEBUG
          else if (Q_stricmp(cmd, "debugTestAlloc") == 0 && CheatsOk( ent ))
          { //rww - small routine to stress the malloc trap stuff and make sure nothing bad is happening.
              char *blah;
              int i = 1;
              int x;

              //stress it. Yes, this will take a while. If it doesn't explode miserably in the process.
              while (i < 32768)
              {
                  x = 0;

                  trap_TrueMalloc((void **)&blah, i);
                  if (!blah)
                  { //pointer is returned null if allocation failed
                      trap_SendServerCommand(-1, va("print \"Failed to alloc at %i!\n\"", i));
                      break;
                  }
                  while (x < i)
                  { //fill the allocated memory up to the edge
                      if (x + 1 == i)
                      {
                          blah[x] = 0;
                      }
                      else
                      {
                          blah[x] = 'A';
                      }
                      x++;
                  }
                  trap_TrueFree((void **)&blah);
                  if (blah)
                  { //should be nullified in the engine after being freed
                      trap_SendServerCommand(-1, va("print \"Failed to free at %i!\n\"", i));
                      break;
                  }

                  i++;
              }

              trap_SendServerCommand(-1, "print \"Finished allocation test\n\"");
          }
#endif
#ifndef FINAL_BUILD
          else if (Q_stricmp(cmd, "debugShipDamage") == 0 && CheatsOk(ent))
          {
              char	arg[MAX_STRING_CHARS];
              char	arg2[MAX_STRING_CHARS];
              int		shipSurf, damageLevel;

              trap_Argv(1, arg, sizeof(arg));
              trap_Argv(2, arg2, sizeof(arg2));
              shipSurf = SHIPSURF_FRONT + atoi(arg);
              damageLevel = atoi(arg2);

              G_SetVehDamageFlags(&g_entities[ent->s.m_iVehicleNum], shipSurf, damageLevel);
          }
#endif
          else if (Q_stricmp(cmd, "adminlogout") == 0)
          {
              if (ent->r.svFlags & SVF_ADMIN)
              {
                  if (ent->r.svFlags & SVF_ADMIN) {
                      G_LogPrintf("%s %s\n", ent->client->pers.netname, ent->client->pers.logout);
                      trap_SendServerCommand(-1, va("print \"%s ^7%s\n\"", ent->client->pers.netname, ent->client->pers.logout));
                      ent->client->pers.iamanadmin = 0;
                      ent->r.svFlags &= ~SVF_ADMIN;
                      ent->client->pers.bitvalue = 0;
                      trap_SendServerCommand(ent-g_entities, va("cvar c_Adminpassword %s", " "));
                  }
              }
          }
          else if (Q_stricmp(cmd, "clanlogout") == 0)
          {
              if (ent->r.svFlags & SVF_CLANSAY)
              {
                  if (ent->r.svFlags & SVF_CLANSAY) {
                      G_LogPrintf("%s %s\n", ent->client->pers.netname, roar_ClanLogout_saying.string);
                      trap_SendServerCommand(-1, va("print \"%s %s\n\"", ent->client->pers.netname, roar_ClanLogout_saying.string));
                      ent->client->pers.iamclan = qfalse;
                      ent->r.svFlags &= ~SVF_CLANSAY;
                  }
              }
          }
          else
          {
              if (Q_stricmp(cmd, "addbot") == 0)
              { //because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
                  //			trap_SendServerCommand( clientNum, va("print \"You can only add bots as the server.\n\"" ) );
                  trap_SendServerCommand(clientNum, va("print \"%s.\n\"", G_GetStringEdString("MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER")));
              }
              else
              {
                  G_PerformEmote(cmd, ent);
              }
          }
    }
