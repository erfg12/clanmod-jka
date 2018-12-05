//g_emotes.c
//Searches for a valid emote and performs it
//Author: Dom

#include "g_local.h"
#include "bg_saga.h"
#include "g_adminshared.h"

void StandardSetBodyAnim(gentity_t *self, int anim, int flags);

//EMOTE BITRATES HAVE ARRIVED!!!!
typedef enum
{
  E_MYHEAD = 0,
  E_COWER,
  E_SMACK,
  E_ENRAGED,
  E_VICTORY1,
  E_VICTORY2,
  E_VICTORY3,
  E_SWIRL,
  E_DANCE1,
  E_DANCE2,
  E_DANCE3,
  E_SIT1, //Punch
  E_SIT2, //Intimidate
  E_SIT3, //Slash
  E_SIT4, //Sit
  E_SIT5, //Sit2
  E_KNEEL1,
  E_KNEEL2,
  E_KNEEL3,
  E_SLEEP,
  E_BREAKDANCE,
  E_CHEER,
  E_SURRENDER,
  E_HEADSHAKE, //v1.08 BEGINS
  E_HEADNOD,
  E_ATEASE,
  E_COMEON,
  E_KISS,
  E_HUG,
  E_HARLEM, // Do the harlem shake
  E_WAIT
} emote_type_t;

typedef enum
{
    E_HELLO,
    E_HEAL,
    E_HIPS,
    E_FLIP,
    E_WON
} emote2_type_t; // I don't like this, refactor eventually

//This void is a mix of requirements to meet for the emote to work, and the custom emote animation itself.
void cm_TheEmote(int requirement, int animation, gentity_t *ent, qboolean freeze){
  if (!(roar_emoteControl.integer & (1 << requirement)) || !(roar_emoteControl2.integer & (1 << requirement))){
    trap_SendServerCommand( ent-g_entities, va("print \"This emote is not allowed on this server.\n\"") );
    return;
  }
  if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE ) {
    return;
  }
  if (ent->client->ps.duelInProgress){
    trap_SendServerCommand( ent-g_entities, va("print \"^7Emotes not allowed in duel!\n\"" ) );
    return;
  }
  if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE){
    return;
  }
  if ( ent->client->ps.saberHolstered < 2 ){
    ent->client->ps.saberHolstered = 2;
  }
  if ( BG_SaberInAttack(ent->client->ps.saberMove) || BG_SaberInSpecialAttack(ent->client->ps.saberMove) || ent->client->ps.saberLockTime ){
    return;
  }
  if (freeze == qtrue){
      VectorClear(ent->client->ps.velocity);
    if (ent->client->ps.forceDodgeAnim == animation){
      ent->client->emote_freeze=0;
      ent->client->ps.saberCanThrow = qtrue;
      ent->client->ps.forceDodgeAnim = 0;
      ent->client->ps.forceHandExtendTime = 0;
    }
    else
    {
        if(ent->client->ps.weapon == WP_SABER && ent->client->ps.saberHolstered < 2) {
            ent->client->ps.saberCanThrow = qfalse;
            ent->client->ps.forceRestricted = qtrue;
            ent->client->ps.saberMove = LS_NONE;
            ent->client->ps.saberBlocked = 0;
            ent->client->ps.saberBlocking = 0;
            ent->client->ps.saberHolstered = 2;
            if (ent->client->saber[0].soundOff)
                G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
            if (ent->client->saber[1].model[0] && ent->client->saber[1].soundOff)
                G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
        }
        ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
        ent->client->ps.forceHandExtendTime = level.time + Q3_INFINITE;
        ent->client->ps.forceDodgeAnim = animation;
      // ent->client->ps.persistant[PERS_REGEN] = 1;
        ent->client->emote_freeze=1;
        
    }
  } else {
    StandardSetBodyAnim(ent, animation, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
  }
}

void G_PerformEmote(char *emote, gentity_t *ent)
{
  if (Q_stricmp(emote, "myhead") == 0){
    cm_TheEmote (E_MYHEAD, BOTH_SONICPAIN_HOLD, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "cower") == 0) || (Q_stricmp(emote, "amcower") == 0)){
    cm_TheEmote (E_COWER, BOTH_COWER1, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "smack") == 0 ) || (Q_stricmp(emote, "amsmack") == 0 )){
    cm_TheEmote (E_SMACK, BOTH_TOSS1, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "enraged") == 0 ) || (Q_stricmp(emote, "amenraged") == 0 )){
    cm_TheEmote (E_ENRAGED, BOTH_FORCE_RAGE, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "victory") == 0 ) || (Q_stricmp(emote, "amvictory") == 0 )){
    cm_TheEmote (E_VICTORY1, BOTH_TAVION_SWORDPOWER, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "victory2") == 0 ) || (Q_stricmp(emote, "amvictory2") == 0 )){
    cm_TheEmote (E_VICTORY2, BOTH_TAVION_SCEPTERGROUND, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "victory3") == 0 ) || (Q_stricmp(emote, "amvictory3") == 0 )){
    cm_TheEmote (E_VICTORY3, BOTH_ALORA_TAUNT, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "swirl") == 0 ) || (Q_stricmp(emote, "amswirl") == 0 )){
    cm_TheEmote (E_SWIRL, BOTH_CWCIRCLELOCK, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "dance") == 0 ) || (Q_stricmp(emote, "amdance") == 0 )){
    cm_TheEmote (E_DANCE1, BOTH_BUTTERFLY_LEFT, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "dance2") == 0 ) || (Q_stricmp(emote, "amdance2") == 0 )){
    cm_TheEmote (E_DANCE2, BOTH_BUTTERFLY_RIGHT, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "dance3") == 0 ) || (Q_stricmp(emote, "amdance3") == 0 )){
    cm_TheEmote (E_DANCE3, BOTH_FJSS_TR_BL, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "sit2") == 0 ) || (Q_stricmp(emote, "amsit2") == 0 )){
    cm_TheEmote (E_SIT5, BOTH_SLEEP6START, ent, qtrue);
  }
  else if (Q_stricmp(emote, "point") == 0){
    cm_TheEmote (E_KNEEL1, BOTH_SCEPTER_HOLD, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "kneel2") == 0) || (Q_stricmp(emote, "amkneel2") == 0)){
    cm_TheEmote (E_KNEEL2, BOTH_ROSH_PAIN, ent, qtrue);
  }
  else if ((Q_stricmp(emote, "kneel") == 0) || (Q_stricmp(emote, "amkneel") == 0)){
    cm_TheEmote (E_KNEEL3, BOTH_CROUCH3, ent, qtrue);
  }
  else if ((Q_stricmp(emote, "laydown") == 0) || (Q_stricmp(emote, "amlaydown") == 0)){
    cm_TheEmote (E_SLEEP, BOTH_SLEEP1, ent, qtrue);
  }
  else if ((Q_stricmp(emote, "breakdance") == 0 ) || (Q_stricmp(emote, "ambreakdance") == 0 )){
    cm_TheEmote (E_BREAKDANCE, BOTH_BACK_FLIP_UP, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "cheer") == 0 ) || (Q_stricmp(emote, "amcheer") == 0 )){
    cm_TheEmote (E_CHEER, BOTH_TUSKENTAUNT1, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "comeon") == 0 ) || (Q_stricmp(emote, "amcomeon") == 0 )){
    cm_TheEmote (E_COMEON, BOTH_COME_ON1, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "headshake") == 0 ) || (Q_stricmp(emote, "amheadshake") == 0 )){
    cm_TheEmote (E_HEADSHAKE, BOTH_HEADSHAKE, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "headnod") == 0 ) || (Q_stricmp(emote, "amheadnod") == 0 )){
    cm_TheEmote (E_HEADNOD, BOTH_HEADNOD, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "surrender") == 0 ) || (Q_stricmp(emote, "amsurrender") == 0 )){
    cm_TheEmote (E_SURRENDER, TORSO_SURRENDER_START, ent, qtrue);
  }
  else if ((Q_stricmp(emote, "atease") == 0 ) || (Q_stricmp(emote, "amatease") == 0 )){
    cm_TheEmote (E_ATEASE, BOTH_STAND4, ent, qtrue);
  }
  else if (Q_stricmp(emote, "punch") == 0 ){
    cm_TheEmote (E_SIT1, BOTH_LOSE_SABER, ent, qfalse);
  }
  else if (Q_stricmp(emote, "intimidate") == 0){
    cm_TheEmote (E_SIT2, BOTH_ROSH_HEAL, ent, qfalse);
  }
  else if (Q_stricmp(emote, "slash") == 0){
    cm_TheEmote (E_SIT3, BOTH_ALORA_SPIN_SLASH, ent, qfalse);
  }
  else if ((Q_stricmp(emote, "sit") == 0) || (Q_stricmp(emote, "amsit") == 0)){
    cm_TheEmote (E_SIT4, BOTH_SIT6, ent, qtrue);
  }
  else if((Q_stricmp(emote, "harlem") == 0) || (Q_stricmp(emote, "amharlem") == 0)) {
    cm_TheEmote(E_HARLEM, BOTH_FORCE_DRAIN_GRABBED, ent, qtrue); 
  }
  else if(Q_stricmp(emote, "amwait") == 0) { // unable to have wait, already exists.
    cm_TheEmote(E_WAIT, BOTH_STAND10, ent, qtrue);
  }
  else if(Q_stricmp(emote, "amhello") == 0) {
    cm_TheEmote(E_HELLO, BOTH_SILENCEGESTURE1, ent, qfalse);
  }
  else if(Q_stricmp(emote, "amheal") == 0) {
    cm_TheEmote(E_HEAL, BOTH_FORCEHEAL_START, ent, qtrue);
  }
  else if(Q_stricmp(emote, "amhips") == 0) {
    cm_TheEmote(E_HIPS, BOTH_STAND5TOSTAND8, ent, qtrue);
  }
  else if(Q_stricmp(emote, "amwon") == 0)
  {
      cm_TheEmote(E_WON, TORSO_HANDSIGNAL1, ent, qfalse);
  }
  //Someday we're going to have to make this look nicer...
  else if (Q_stricmp(emote, "kiss") == 0){
    trace_t tr;
    vec3_t fPos;

    AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

    fPos[0] = ent->client->ps.origin[0] + fPos[0]*40.0f;
    fPos[1] = ent->client->ps.origin[1] + fPos[1]*40.0f;
    fPos[2] = ent->client->ps.origin[2] + fPos[2]*40.0f;

    trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

    if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE) {
      return;
    }
    if (!(roar_emoteControl.integer & (1 << E_KISS)))
    {
      trap_SendServerCommand( ent-g_entities, va("print \"This emote is not allowed on this server.\n\"") );
      return;
    }
    if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
    {
      return;
    }
    if (ent->client->ps.duelInProgress)
    {
      trap_SendServerCommand( ent-g_entities, va("print \"^7Emotes not allowed in duel!\n\"" ) );
      return;
    }

    if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
    {
      gentity_t *other = &g_entities[tr.entityNum];

      vec3_t entDir;
      vec3_t otherDir;
      vec3_t entAngles;
      vec3_t otherAngles;

      if (other && other->inuse && other->client)
      {
        if ( other->client->sess.spectatorState == SPECTATOR_FOLLOW || other->client->sess.spectatorState == SPECTATOR_FREE) {
          return;
        }
        if ( ent->client->ps.saberHolstered < 2 ){
          ent->client->ps.saberHolstered = 2;
        }
        if ( other->client->ps.saberHolstered < 2 ){
          other->client->ps.saberHolstered = 2;
        }
        
        VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
        VectorCopy( ent->client->ps.viewangles, entAngles );
        entAngles[YAW] = vectoyaw( otherDir );
        SetClientViewAngle( ent, entAngles );
        StandardSetBodyAnim(ent, BOTH_KISSER, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
          VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
          VectorCopy( other->client->ps.viewangles, otherAngles );
          otherAngles[YAW] = vectoyaw( entDir );
          SetClientViewAngle( other, otherAngles );
          
          other->client->ps.saberMove = LS_NONE;
          other->client->ps.saberBlocked = 0;
          other->client->ps.saberBlocking = 0;
          StandardSetBodyAnim(other, BOTH_KISSEE, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);

      }
    }
  }
  else if (Q_stricmp(emote, "hug") == 0){
    trace_t tr;
    vec3_t fPos;

    AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

    fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
    fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
    fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

    trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

    if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW || ent->client->sess.spectatorState == SPECTATOR_FREE) {
      return;
    }
    if (!(roar_emoteControl.integer & (1 << E_HUG)))
    {
      trap_SendServerCommand( ent-g_entities, va("print \"This emote is not allowed on this server.\n\"") );
      return;
    }
    if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
    {
      return;
    }
    if (ent->client->ps.duelInProgress)
    {
      trap_SendServerCommand( ent-g_entities, va("print \"^7Emotes not allowed in duel!\n\"" ) );
      return;
    }

    if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
    {
      gentity_t *other = &g_entities[tr.entityNum];

      vec3_t entDir;
      vec3_t otherDir;
      vec3_t entAngles;
      vec3_t otherAngles;

      if (other && other->inuse && other->client)
      {
        if ( other->client->sess.spectatorState == SPECTATOR_FOLLOW || other->client->sess.spectatorState == SPECTATOR_FREE) {
          return;
        }
        if ( ent->client->ps.saberHolstered < 2 ){
          ent->client->ps.saberHolstered = 2;
        }
        if ( other->client->ps.saberHolstered < 2 ){
          other->client->ps.saberHolstered = 2;
        }

        StandardSetBodyAnim(ent, BOTH_HUGGER1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
        StandardSetBodyAnim(other, BOTH_HUGGEE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
        VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
        VectorCopy( other->client->ps.viewangles, otherAngles );
        otherAngles[YAW] = vectoyaw( entDir );
        SetClientViewAngle( other, otherAngles );

        other->client->ps.saberMove = LS_NONE;
        VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
        VectorCopy( ent->client->ps.viewangles, entAngles );
        entAngles[YAW] = vectoyaw( otherDir );
        SetClientViewAngle( ent, entAngles );
      }
    }
  }
    else if (Q_stricmp(emote, "amflip") == 0)
    {
        if(!ent->client || ent->client->ps.weapon != WP_SABER) //block non sabers
            return;
        
        if(ent->client->ps.fd.saberAnimLevel == SS_STRONG || ent->client->ps.fd.saberAnimLevel == SS_MEDIUM || ent->client->ps.fd.saberAnimLevel == SS_FAST)
        {
            if(ent->client->ps.saberHolstered)
            {
                cm_TheEmote(E_FLIP, BOTH_STAND1TO2, ent, qfalse);
            }
            else
            {
                cm_TheEmote(E_FLIP, BOTH_STAND2TO1, ent, qfalse);
                Cmd_ToggleSaber_f(ent);
            }
        }
        // check if duals
        if(ent->client->ps.fd.saberAnimLevel == SS_DUAL)
        {
            if (ent->client->ps.saberHolstered)
            {
                cm_TheEmote(E_FLIP, BOTH_S1_S7, ent, qfalse);
            }
            else
            {
                cm_TheEmote(E_FLIP, BOTH_SHOWOFF_FAST, ent, qfalse);
                Cmd_ToggleSaber_f(ent);
            }
        }
        // check if staff
        // TODO: turnoff
        if(ent->client->ps.fd.saberAnimLevel == SS_STAFF)
        {
            cm_TheEmote(E_FLIP, BOTH_SHOWOFF_FAST, ent, qfalse);
        }
        
        Cmd_ToggleSaber_f(ent);
        return;
    }
  else
  {
    G_PerformAdminCMD(emote, ent);
    //trap_SendServerCommand( ent-g_entities, va("print \"^7Unknown command: %s^7!\n\"", emote ) );
    //return;
  }
}
