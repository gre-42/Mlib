// ####################################
// #                                  #
// #       Ping of Death - Bot        #
// #                by                #
// #    Markus Klinge aka Count Floyd #
// #                                  #
// ####################################
//
// Started from the HPB-Bot Alpha Source
// by Botman so Credits for a lot of the basic
// HL Server/Client Stuff goes to him
//
// util.cpp
//
// Misc utility Functions. Really not optional after all.

#include "bot_globals.h"
#include "bot_weapons.h"  // KWo - 10.03.2006

Vector UTIL_VecToAngles (const Vector &vec)
{
/*
   float rgflVecOut[3];

   VEC_TO_ANGLES (vec, rgflVecOut);

   return (Vector(rgflVecOut));
*/
   // the purpose of this function is to convert a spatial location determined by the Vector
   // passed in into absolute angles from the origin of the world. - from YapB

   float fYaw, fPitch;

   if ((vec(0) == 0) && (vec(1) == 0))
   {
      fYaw = 0;
      fPitch = (vec(2) > 0) ? 90.0 : 270.0;
   }
   else
   {
      fYaw = atan2f (vec(1), vec(0)) * (180.0 / M_PI);
      fPitch = atan2f (vec(2), Length2D(vec)) * (180.0 / M_PI);
   }
   return Vector (fPitch, fYaw, 0);
}

unsigned short FixedUnsigned16 (float value, float scale)
{
   int output;

   output = (int) (value * scale);   // KWo - to remove warning
   if (output < 0)
      output = 0;
   if (output > 0xFFFF)
      output = 0xFFFF;

   return ((unsigned short) output);
}


short FixedSigned16 (float value, float scale)
{
   int output;

   output = (int) (value * scale);   // KWo - to remove warning

   if (output > 32767)
      output = 32767;

   if (output < -32768)
      output = -32768;

   return ((short) output);
}


int UTIL_GetTeam (edict_t *pEntity)
{
   // return team number 1 through 2 based what MOD uses for team numbers
   int clientIndex = ENTINDEX (pEntity) - 1;
   if (clientIndex < 0 || clientIndex >= 32) {
      throw std::runtime_error("Client index out of bounds");
   }
   return (clients[clientIndex].iTeam);
}


bot_t *UTIL_GetBotPointer (edict_t *pEdict)
{
   if (FNullEnt (pEdict))
      return (NULL); // reliability check

   int index = ENTINDEX (pEdict) - 1;

   if ((index >= 0) && (index < gpGlobals->maxClients) && (bots[index].pEdict == pEdict))
      return (&bots[index]);

   return (NULL); // return NULL if edict is not a bot
}


bool IsAlive (edict_t *pEdict)
{
   return ((pEdict->v.deadflag == DEAD_NO)
           && (pEdict->v.health > 0)
           && !(pEdict->v.flags & FL_NOTARGET)
//           && (pEdict->v.takedamage != 0)  doesn't work with godmode  KWo - 05.04.2006
       /*    && (pEdict->v.movetype != MOVETYPE_NOCLIP) */);  // KWo - 02.03.2010
}


bool FInViewCone (Vector *pOrigin, edict_t *pEdict)
{
   Vector2D vec2LOS;
   float flDot;
   float fov;

   MAKE_VECTORS (pEdict->v.v_angle);

   vec2LOS = Normalize(Make2D(*pOrigin - pEdict->v.origin));

   flDot = DotProduct (vec2LOS, Make2D(gpGlobals->v_forward));

   if (pEdict->v.fov > 0)
      fov = pEdict->v.fov;
   else
      fov = 90;

   if (flDot >= cos ((fov / 2) * M_PI / 180))
      return (TRUE);

   return (FALSE);
}


float GetShootingConeDeviation (edict_t *pEdict, Vector *pvecPosition)
{
   Vector vecDir = Normalize(*pvecPosition - GetGunPosition (pEdict));
   Vector vecAngle = pEdict->v.v_angle;

   MAKE_VECTORS (vecAngle);

   // He's facing it, he meant it
   return (DotProduct (gpGlobals->v_forward, vecDir));
}


bool IsShootableBreakable (edict_t *pent)  // KWo - 08.02.2006
{
   if (pent == NULL)
      return (false);

   return ( ( ((FStrEq ("func_breakable", STRING (pent->v.classname))
           && (    (pent->v.playerclass == 1)
                || (pent->v.health == 0)
                || ( (pent->v.health > 1) && (pent->v.health < 1000))
                || (pent->v.rendermode == 4)
               )) // KWo - 21.02.2006 - br. crates has rendermode 4
            || (FStrEq ("func_pushable", STRING (pent->v.classname))
               && (pent->v.health < 1000) && (pent->v.spawnflags & SF_PUSH_BREAKABLE))))  // KWo - 03.02.2007
            && (pent->v.impulse == 0)
            && (pent->v.takedamage > 0)
            && !(pent->v.spawnflags & SF_BREAK_TRIGGER_ONLY) );
}

/*
bool is_breakable(edict_t* pBreakable) {
   char *mat_type;
   mat_type = INFOKEY_VALUE(GET_INFOKEYBUFFER(pBreakable), "material");
   return (!strcmp(mat_type, "7"));
}
*/

bool FBoxVisible (bot_t *pBot, edict_t *pTargetEdict, Vector *pvHit, unsigned char *ucBodyPart) // KWo - 23.03.2012 - rewritten...
{
// KWo - 05.09.2009 - changed variables to static ones
   static int i;
   static int RenderFx;             // KWo - 22.03.2008
   static int RenderMode;           // KWo - 22.03.2008
   static int TargetEntIndex;       // KWo - 17.01.2011
   static int TargetWeapon;         // KWo - 17.01.2011
   static Vector RenderColor;       // KWo - 22.03.2008
   static float RenderAmount;       // KWo - 22.03.2008
   static float LightLevel;         // KWo - 23.03.2008
   static bool SemiTransparent;     // KWo - 22.03.2008
   static bool WeaponIsGun;         // KWo - 17.01.2011
   static TraceResult tr;
   static Vector vecLookerOrigin;
   static Vector vecTarget;
   static Vector vecEnDirection;
   static edict_t *pEdict;

   *ucBodyPart = 0;
   *pvHit = g_vecZero;              // KWo - 04.07.2008
   pEdict = pBot->pEdict;           // KWo - 27.05.2008

   if (FNullEnt(pTargetEdict))      // KWo - 17.01.2011
      return (FALSE);

   TargetEntIndex = ENTINDEX (pTargetEdict) - 1;                                        // 17.01.2011
   if ((TargetEntIndex >= 0) && (TargetEntIndex < gpGlobals->maxClients))
   {
      TargetWeapon = clients[TargetEntIndex].iCurrentWeaponId;                          // 17.01.2011
      WeaponIsGun = (WeaponIsPistol(TargetWeapon) || WeaponIsPrimaryGun(TargetWeapon)); // 17.01.2011
   }
   else
      WeaponIsGun = FALSE;

// Can't see the target entity if blinded or smoked...
   if (Length(pEdict->v.origin - pTargetEdict->v.origin) > pBot->f_view_distance)   // KWo - 14.09.2008
      return (FALSE);

   // don't look through water
   if (((pEdict->v.waterlevel != 3) && (pTargetEdict->v.waterlevel == 3))
       || ((pEdict->v.waterlevel == 3) && (pTargetEdict->v.waterlevel == 0)))
      return (FALSE);

// KWo - 22.03.2008 - added invisibility check
   RenderFx = pTargetEdict->v.renderfx;
   RenderMode = pTargetEdict->v.rendermode;
   RenderColor = pTargetEdict->v.rendercolor;
   RenderAmount = pTargetEdict->v.renderamt;
   SemiTransparent = false;    // KWo (moved) - 05.09.2009

   if (((RenderFx == kRenderFxExplode) || (pTargetEdict->v.effects & EF_NODRAW))
      && (!(WeaponIsGun) || !(pTargetEdict->v.oldbuttons & IN_ATTACK))) // kRenderFxExplode is always invisible even for mode kRenderNormal
   {
      return (FALSE);
   }
   else if (((RenderFx == kRenderFxExplode) || (pTargetEdict->v.effects & EF_NODRAW))
      && (pTargetEdict->v.oldbuttons & IN_ATTACK) && (WeaponIsGun))  // KWo - 17.01.2011
   {
      SemiTransparent = true;
   }
   else if ((RenderFx != kRenderFxHologram) && (RenderMode != kRenderNormal)) // kRenderFxHologram is always visible no matter what is the mode
   {
      if (RenderFx == kRenderFxGlowShell)
      {
         if ((RenderAmount <= 20.0) && (RenderColor(0) <= 20)
            && (RenderColor(1) <= 20) && (RenderColor(2) <= 20))
         {
            if (!(pTargetEdict->v.oldbuttons & IN_ATTACK) || !(WeaponIsGun))
            {
               return (FALSE);
            }
            else
            {
               SemiTransparent = true;
            }
         }
         else if ((RenderAmount <= 60.0) && (RenderColor(0) <= 60)
            && (RenderColor(1) <= 60) && (RenderColor(2) <= 60))
         {
            SemiTransparent = true;
         }
      }
      else
      {
         if (RenderAmount <= 20)
         {
            if (!(pTargetEdict->v.oldbuttons & IN_ATTACK) || !(WeaponIsGun))
            {
               return (FALSE);
            }
            else
            {
               SemiTransparent = true;
            }
         }
         else if (RenderAmount <= 60)
         {
            SemiTransparent = true;
         }
      }
   }


// KWo - 26.03.2008 - added darkness check
   LightLevel = UTIL_IlluminationOf(pTargetEdict);
//   ALERT(at_logged,"[DEBUG] Bot %s checks the illumination of %s. It's = %f.\n",
//      STRING(pEdict->v.netname), STRING(pTargetEdict->v.netname), LightLevel);

   if ((!pBot->bUsesNVG) && (((LightLevel < 3.0) && (g_f_cv_skycolor > 50.0)) || ((LightLevel < 25.0) && (g_f_cv_skycolor <= 50.0)))
      && (!(pTargetEdict->v.effects & EF_DIMLIGHT)) && (!(pTargetEdict->v.oldbuttons & IN_ATTACK) || !(WeaponIsGun))) // KWo - 17.01.2011
   {
      return (FALSE);
   }
   else if (((((LightLevel < 10.0) && (g_f_cv_skycolor > 50.0)) || ((LightLevel < 30.0) && (g_f_cv_skycolor <= 50.0)))
      && (pTargetEdict->v.oldbuttons & IN_ATTACK) && (WeaponIsGun)) && (!(pTargetEdict->v.effects & EF_DIMLIGHT)) && (!pBot->bUsesNVG)) // KWo - 17.01.2011
   {
      SemiTransparent = true; // in this case we can notice the enemy, but not so good...
   }

//   if (SemiTransparent)
//      ALERT(at_logged, "[DEBUG] FBoxVisible - Bot's %s target SemiTransparent = true, LightLevel = %.2f \n", pBot->name, LightLevel);

   vecLookerOrigin = GetGunPosition (pEdict);
   vecTarget = pTargetEdict->v.origin;

   // Check direct Line to waist
   TRACE_LINE (vecLookerOrigin, vecTarget, dont_ignore_monsters | ignore_glass, pEdict, &tr);

   if ((tr.flFraction >= 1.0) || (tr.pHit == pTargetEdict))
   {
      *pvHit = tr.vecEndPos  + Vector (0.0, 0.0, 3.0); // KWo - 13.07.2008
      *ucBodyPart |= WAIST_VISIBLE;
   }

   vecEnDirection = pTargetEdict->v.angles; // KWo - 05.09.2009 (moved)
   MAKE_VECTORS (vecEnDirection);

   // Check direct Line to head
   vecTarget = vecTarget + pTargetEdict->v.view_ofs + Vector(0.0, 0.0, 2.0); // KWo - 02.04.2010
   if (!(pTargetEdict->v.oldbuttons & IN_DUCK))       // KWo - 13.07.2008
   {
      vecTarget = vecTarget + Vector(0.0, 0.0, 1.0);  // KWo - 02.04.2010
   }

   TRACE_LINE (vecLookerOrigin, vecTarget, dont_ignore_monsters | ignore_glass, pEdict, &tr);

   // if the player is rendered, his head cannot be good seen...
   if (((tr.flFraction >= 1.0) || (tr.pHit == pTargetEdict)) && (!SemiTransparent))
   {
      *pvHit = tr.vecEndPos;
      *ucBodyPart |= HEAD_VISIBLE;
   }

   if (*ucBodyPart != 0)
      return (TRUE);

   // Nothing visible - check randomly other Parts of Body
   for (i = 0; i < 6; i++)
   {
      vecTarget = pTargetEdict->v.origin;
      switch(i)
      {
         case 0: // left arm
         {
            vecTarget(0) -= 10.0 * gpGlobals->v_right(0);
            vecTarget(1) -= 10.0 * gpGlobals->v_right(1);
            vecTarget(2) += 8.0;
            break;
         }
         case 1: // right arm
         {
            vecTarget(0) += 10.0 * gpGlobals->v_right(0);
            vecTarget(1) += 10.0 * gpGlobals->v_right(1);
            vecTarget(2) += 8.0;
            break;
         }
         case 2: // left leg
         {
            vecTarget(0) -= 10.0 * gpGlobals->v_right(0);
            vecTarget(1) -= 10.0 * gpGlobals->v_right(1);
            vecTarget(2) -= 12.0;
            break;
         }
         case 3: // right leg
         {
            vecTarget(0) += 10.0 * gpGlobals->v_right(0);
            vecTarget(1) += 10.0 * gpGlobals->v_right(1);
            vecTarget(2) -= 12.0;
            break;
         }
         case 4: // left foot
         {
            vecTarget(0) -= 10.0 * gpGlobals->v_right(0);
            vecTarget(1) -= 10.0 * gpGlobals->v_right(1);
            vecTarget(2) -= 24.0;
            break;
         }
         case 5: // right foot
         {
            vecTarget(0) += 10.0 * gpGlobals->v_right(0);
            vecTarget(1) += 10.0 * gpGlobals->v_right(1);
            vecTarget(2) -= 24.0;
            break;
         }
      }
/*
      vecTarget(0) += RANDOM_FLOAT (pTargetEdict->v.mins(0), pTargetEdict->v.maxs(0));
      vecTarget(1) += RANDOM_FLOAT (pTargetEdict->v.mins(1), pTargetEdict->v.maxs(1));
      vecTarget(2) += RANDOM_FLOAT (pTargetEdict->v.mins(2), pTargetEdict->v.maxs(2));
*/

      TRACE_LINE (vecLookerOrigin, vecTarget, dont_ignore_monsters | ignore_glass, pEdict, &tr);

      if ((tr.flFraction >= 1.0) && (tr.pHit == pTargetEdict))
      {
         // Return seen position
         *pvHit = tr.vecEndPos;
         *ucBodyPart |= CUSTOM_VISIBLE;
         return (TRUE);
      }
   }

   return (FALSE);
}


bool FVisible (const Vector &vecOrigin, edict_t *pEdict)
{
   TraceResult tr;
   Vector vecLookerOrigin;

   // look through caller's eyes
   vecLookerOrigin = GetGunPosition (pEdict);

   // don't look through water
   if ((POINT_CONTENTS (vecOrigin) == CONTENTS_WATER)
       != (POINT_CONTENTS (vecLookerOrigin) == CONTENTS_WATER))
      return (FALSE);

   TRACE_LINE (vecLookerOrigin, vecOrigin, ignore_monsters | ignore_glass, pEdict, &tr);

   if (tr.flFraction != 1.0)
      return (FALSE);  // Line of sight is not established

   return (TRUE);  // line of sight is valid.
}


Vector GetGunPosition (edict_t *pEdict)
{
   return (pEdict->v.origin + pEdict->v.view_ofs);
}


int UTIL_GetNearestPlayerIndex (Vector vecOrigin)
{
   float fDistance;
   float fMinDistance = 9999.0;
   int index = 0;
   int i;

   for (i = 0; i < gpGlobals->maxClients; i++)
   {
      if (!(clients[i].iFlags & CLIENT_USED)
          || !(clients[i].iFlags & CLIENT_ALIVE))
         continue;

      fDistance = Length(clients[i].pEdict->v.origin - vecOrigin);

      if (fDistance < fMinDistance)
      {
         index = i;
         fMinDistance = fDistance;
      }
   }

   return (index);
}


Vector VecBModelOrigin (edict_t *pEdict)
{
   return (pEdict->v.absmin + (pEdict->v.size * 0.5f));
}

void UTIL_DecalTrace (TraceResult *pTrace, char *pszDecalName)
{
   short entityIndex;
   int index;
   int message;

   index = DECAL_INDEX(pszDecalName);
   if (index < 0)
      index = 0;

   if (pTrace->flFraction == 1.0)
      return;

   // Only decal BSP models
   if (!FNullEnt (pTrace->pHit))
   {
      edict_t *pHit = pTrace->pHit;

      if ((pHit->v.solid == SOLID_BSP) || (pHit->v.movetype == MOVETYPE_PUSHSTEP))
         entityIndex = ENTINDEX (pHit);
      else
         return;
   }
   else
      entityIndex = 0;

   message = TE_DECAL;
   if (entityIndex != 0)
   {
      if (index > 255)
      {
         message = TE_DECALHIGH;
         index -= 256;
      }
   }
   else
   {
      message = TE_WORLDDECAL;
      if (index > 255)
      {
         message = TE_WORLDDECALHIGH;
         index -= 256;
      }
   }

   MESSAGE_BEGIN (MSG_BROADCAST, SVC_TEMPENTITY);
   WRITE_BYTE (message);
   WRITE_COORD (pTrace->vecEndPos(0));
   WRITE_COORD (pTrace->vecEndPos(1));
   WRITE_COORD (pTrace->vecEndPos(2));
   WRITE_BYTE (index);
   if (entityIndex)
      WRITE_SHORT (entityIndex);
   MESSAGE_END ();
   return;
}

void UTIL_RoundStart (void)
{
   // function to be called each time a round starts in CS 1.5 or 1.6

   int i;

   for (i = 0; i < 32; i++)
   {
      iRadioSelect[i] = 0;
   }

   g_bBombPlanted = FALSE;
   g_iDefuser = -1; // KWo - 13.07.2007
   g_bBombDefusing = false; // KWo - 13.07.2007

   g_bBombSayString = FALSE;
   g_fTimeBombPlanted = 0.0;
   g_vecBomb = g_vecZero;

   // Clear Waypoint Indices of visited Bomb Spots
   for (i = 0; i < MAXNUMBOMBSPOTS; i++)
      g_rgiBombSpotsVisited[i] = -1;

   g_iLastBombPoint = -1;
   g_fTimeNextBombUpdate = 0.0;

   g_bLeaderChosenT = FALSE;
   g_bLeaderChosenCT = FALSE;

   g_bHostageRescued = FALSE;
   g_rgfLastRadioTime[0] = 0.0;
   g_rgfLastRadioTime[1] = 0.0;
   g_bBotsCanPause = FALSE;

   g_fAutoKillTime = 0.0; // KWo - 02.05.2006
   // Clear Array of Player Stats
   for (i = 0; i < gpGlobals->maxClients; i++)
   {
      clients[i].vecSoundPosition = g_vecZero;
      clients[i].fHearingDistance = 0.0;
      clients[i].fTimeSoundLasting = 0.0;
      clients[i].fMaxTimeSoundLasting = 0.5;    // KWo - 01.08.2006

      if (clients[i].welcome_time == -1.0)      // KWo - 19.04.2010
         clients[i].welcome_time = -2.0;
      if (clients[i].wptmessage_time == -1.0)   // KWo - 19.04.2010
         clients[i].wptmessage_time = -2.0;
   }

   // Update Experience Data on Round Start
   g_iUpdGlExpState = 1;
   g_bRecalcKills = FALSE;

   // Calculate the Round Mid/End in World Time
   g_fTimeRoundStart = gpGlobals->time + CVAR_GET_FLOAT ("mp_freezetime");
   g_fTimeRoundMid = g_fTimeRoundStart + CVAR_GET_FLOAT ("mp_roundtime") * 60 / 2;
   g_fTimeRoundEnd = g_fTimeRoundStart + CVAR_GET_FLOAT ("mp_roundtime") * 60;

   if (!gmsgFlashlight)
   {
      gmsgFlashlight = GET_USER_MSG_ID (PLID, "Flashlight", NULL);
   }
   if (!gmsgNVGToggle)
   {
      gmsgNVGToggle = GET_USER_MSG_ID (PLID, "NVGToggle", NULL);
   }

   // Show the Waypoint copyright Message right at round start
   g_bMapInitialised = TRUE;
   g_bRoundEnded = FALSE; // KWo - 30.09.2010
//   ALERT(at_logged, "[DEBUG] Info map parameters - buying = %d.\n", g_i_MapBuying);
   return;
}

void UTIL_RoundEnd (void)    // KWo - 02.05.2006
{
//   UTIL_ServerPrint ("[Debug] Round End event caught.\n");
   if (g_bSaveVisTable)
      SaveVisTab();
   g_fTimeRoundEnd = gpGlobals->time;  // KWo - 28.05.2010
   g_bRoundEnded = TRUE; // KWo - 30.09.2010
   return;
}

void UTIL_GameStarted (void)    // KWo - 10.02.2006
{
   UTIL_SaveButtonsData();
   UTIL_SaveBreakableData();
   UTIL_SaveHostagesData();  // KWo - 16.05.2006
   return;
}

void UTIL_SaveButtonsData (void)    // KWo - 10.02.2006
{
   edict_t *pButton = NULL;
   const char *button_name = "func_button";
   vec3_t v_button_origin;
   int i;
   g_iNumButtons = 0;

   for (i=0; i < MAX_BUTTONS; i++)
   {
      ButtonsData[i].EntIndex = -1;
      ButtonsData[i].classname[0] = '\0';
      ButtonsData[i].origin = g_vecZero;
      ButtonsData[i].target[0] = '\0';
   }

   for (i = 0; i < 4; i++)
   {
      switch (i)
      {
         case 0:
            button_name = "func_button";
            break;
         case 1:
            button_name = "func_pushable";
            break;
         case 2:
            button_name = "trigger_once";
            break;
/*
         case 3:
            button_name = "trigger_multiple";
            break;
*/
         case 3:
            button_name = "func_door";
            break;
      }

      pButton = FIND_ENTITY_BY_CLASSNAME (NULL, button_name);
      while (!FNullEnt (pButton))
      {
         const char* vtarget = STRING(pButton->v.target);
         if ((g_iNumButtons < MAX_BUTTONS) && ((i != 1) || !(pButton->v.spawnflags & SF_PUSH_BREAKABLE)) && (strcmp(vtarget, "") != 0))
         {
            ButtonsData[g_iNumButtons].EntIndex = ENTINDEX(pButton);
            v_button_origin = VecBModelOrigin (pButton);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
            snprintf (ButtonsData[g_iNumButtons].classname, sizeof (ButtonsData[g_iNumButtons].classname), STRING (pButton->v.classname));
            ButtonsData[g_iNumButtons].origin = v_button_origin;
            snprintf (ButtonsData[g_iNumButtons].target, sizeof (ButtonsData[g_iNumButtons].target), STRING (pButton->v.target));
#pragma GCC pop
            g_iNumButtons++;
         }
         pButton = FIND_ENTITY_BY_CLASSNAME (pButton, button_name);
      }
   }
/*
   UTIL_ServerPrint ("[Debug] Found %i buttons on the map.\n", g_iNumButtons);
   for (i=0; i < g_iNumButtons; i++)
   {
      UTIL_ServerPrint ("[Debug] Button %i , EntIndex = %i, classname - %s , target - %s .\n", i+1, ButtonsData[i].EntIndex, ButtonsData[i].classname, ButtonsData[i].target);
   }
*/
   return;
}

void UTIL_SaveBreakableData (void)    // KWo - 04.03.2006
{
   edict_t *pEnt = NULL;
   int i;
   int j;  // KWo - 06.03.2006
   int Ent_Ind_Tr;  // KWo - 06.03.2006
   Vector vecPath_S;  // KWo - 06.03.2006
   Vector vecPath_E;  // KWo - 06.03.2006
   TraceResult tr;  // KWo - 06.03.2006

   for (i = 0; i < g_iNumWaypoints; i++)  // KWo - 06.03.2006
   {
      vecPath_S = paths[i]->origin;
      vecPath_E = vecPath_S - Vector (0,0,100);
      TRACE_LINE (vecPath_S, vecPath_E, ignore_monsters, NULL, &tr);
      Ent_Ind_Tr = ENTINDEX(tr.pHit);

      if (FStrEq(STRING(tr.pHit->v.classname),"func_breakable") || FStrEq(STRING(tr.pHit->v.classname),"func_pushable"))
      {
         for (j = 0; j < g_iNumBreakables; j++)
         {
            if (Ent_Ind_Tr == BreakablesData[j].EntIndex)
            {
               BreakablesData[j].ignored = true;
            }
         }
      }
   }

   pEnt = NULL;
   j = 0;
   while (!FNullEnt (pEnt = FIND_ENTITY_BY_CLASSNAME (pEnt, "env_explosion")))
   {
      for (i = 0; i < g_iNumBreakables; i++)
      {
         if (FStrEq(STRING(pEnt->v.targetname),BreakablesData[i].target) && !(FStrEq(STRING(pEnt->v.targetname),"")))
            BreakablesData[i].ignored = true;
      }
      j++;
   }
/*
   UTIL_ServerPrint ("[Debug] Found %i shootable breakables on the map and %d explosive .\n", g_iNumBreakables, j);
   for (i=0; i < g_iNumBreakables; i++)
   {
      UTIL_ServerPrint ("[Debug] Breakable %i , EntIndex = %i, classname - %s , target - %s , %s.\n", i+1, BreakablesData[i].EntIndex, BreakablesData[i].classname, BreakablesData[i].target, (BreakablesData[i].ignored)? "ignored" : "accepted");
   }

   pEnt = NULL;
   while (!FNullEnt (pEnt = FIND_ENTITY_BY_CLASSNAME (pEnt, "func_breakable")))
   {
      if (atoi(INFOKEY_VALUE (GET_INFOKEYBUFFER (pEnt), "material")) == 7) // unbreakable glass
      {
         UTIL_ServerPrint ("[Debug] Found a breakable which is based on unbreakable glass.\n");
      }
   }
*/
   return;
}

void UTIL_CheckCvars (void) // KWo - 06.04.2006
{
   unsigned int i;
   // int pl_index;
   // edict_t *pPlayer;  // KWo - 02.05.2006
   // char *infobuffer;  // KWo - 02.05.2006

// Check restrictions
// Thanks to Bailopan (AMX MOD X) for the solution
   char str1[27];
   if (g_rgcvarPointer[PBCVAR_RESTRWEAPONS])  // KWo - 13.10.2006
   {
      strncpy (str1, g_rgcvarPointer[PBCVAR_RESTRWEAPONS]->string, 26);
   }
   else
   {
      strncpy (str1, CVAR_GET_STRING(g_rgpszPbCvars[PBCVAR_RESTRWEAPONS]), 26);
   }
   str1[26] = '\0';
   size_t len1 = strlen(str1);
   for (i = 0; i < len1; i++)
   {
      if (i < NUM_WEAPONS)
      {
         g_iWeaponRestricted[i] = (int)(str1[i] - '0');
      }
   }
   char str2[10];
   if (g_rgcvarPointer[PBCVAR_RESTREQUIPAMMO])  // KWo - 13.10.2006
      strncpy (str2, g_rgcvarPointer[PBCVAR_RESTREQUIPAMMO]->string, 9);
   else
      strncpy (str2, CVAR_GET_STRING(g_rgpszPbCvars[PBCVAR_RESTREQUIPAMMO]),9);
   str2[9] = '\0';
   size_t len2 = strlen(str2);
   for (i = 0; i < len2; i++)
   {
      if (i < NUM_EQUIPMENTS)
      {
         g_iEquipAmmoRestricted[i] =  (int)(str2[i] - '0');
      }
   }

//   UTIL_ServerPrint ("[Debug] Cvars tested. Min_bots = %d, Max_bots = %d .\n", (int)g_rgcvarPointer[PBCVAR_MINBOTS]->value, (int)g_rgcvarPointer[PBCVAR_MAXBOTS]->value);

// Check some other cvars
   g_b_cv_spray = false;
   if (g_rgcvarPointer[PBCVAR_SPRAY])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_SPRAY]->value > 0.f)
         g_b_cv_spray = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_SPRAY]) > 0.f)
         g_b_cv_spray = true;
   }

   g_b_cv_chat = false;
   if (g_rgcvarPointer[PBCVAR_CHAT])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_CHAT]->value > 0.f)
         g_b_cv_chat = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_CHAT]) > 0.f)
         g_b_cv_chat = true;
   }

   g_i_cv_latencybot = 0;         // KWo - 02.03.2010
   if (g_rgcvarPointer[PBCVAR_LATENCYBOT])
   {
      g_i_cv_latencybot = (int)g_rgcvarPointer[PBCVAR_LATENCYBOT]->value;
   }
   else
   {
      g_i_cv_latencybot = (int)CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_LATENCYBOT]);
   }
   if (g_i_cv_latencybot < 0)
      g_i_cv_latencybot = 0;
   else if (g_i_cv_latencybot > 2)
      g_i_cv_latencybot = 2;

   if (g_rgcvarPointer[PBCVAR_NUMFOLLOWUSER])  // KWo - 13.10.2006
      g_i_cv_numfollowuser = (int) g_rgcvarPointer[PBCVAR_NUMFOLLOWUSER]->value;
   else
      g_i_cv_numfollowuser = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_NUMFOLLOWUSER]);

   if (g_rgcvarPointer[PBCVAR_MAXWEAPONPICKUP])  // KWo - 13.10.2006
      g_i_cv_maxweaponpickup = (int) g_rgcvarPointer[PBCVAR_MAXWEAPONPICKUP]->value;
   else
      g_i_cv_maxweaponpickup = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_MAXWEAPONPICKUP]);
   if (g_i_cv_maxweaponpickup < -1) // KWo - 26.12.2006
      g_i_cv_maxweaponpickup = -1;

   g_b_cv_shootthruwalls = false;
   if (g_rgcvarPointer[PBCVAR_SHOOTTHRUWALLS])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_SHOOTTHRUWALLS]->value > 0.f)
         g_b_cv_shootthruwalls = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_SHOOTTHRUWALLS]) > 0.f)
         g_b_cv_shootthruwalls = true;
   }

   if (g_rgcvarPointer[PBCVAR_SKIN])  // KWo - 18.11.2006
      g_i_cv_skin = (int) g_rgcvarPointer[PBCVAR_SKIN]->value;
   else
      g_i_cv_skin = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_SKIN]);
   if ((g_i_cv_skin < 1) || (g_i_cv_skin > 5))
      g_i_cv_skin = 5;

   if (g_rgcvarPointer[PBCVAR_DETAILNAMES])  // KWo - 22.03.2008
      g_i_cv_detailnames = (int) g_rgcvarPointer[PBCVAR_DETAILNAMES]->value;
   else
      g_i_cv_detailnames = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_DETAILNAMES]);

   if ((g_i_cv_detailnames < 0) || (g_i_cv_detailnames > 3))
      g_i_cv_detailnames = 0;

   g_b_cv_UseSpeech = false; // KWo - 07.10.2006
   if (g_rgcvarPointer[PBCVAR_USESPEECH])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_USESPEECH]->value > 0.f)
         g_b_cv_UseSpeech = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_USESPEECH]) > 0.f)
         g_b_cv_UseSpeech = true;
   }

   g_b_cv_firsthumanrestart = false;        // KWo - 04.10.2010
   if (g_rgcvarPointer[PBCVAR_FIRSTHUMANRESTART])
   {
      if (g_rgcvarPointer[PBCVAR_FIRSTHUMANRESTART]->value > 0.f)
         g_b_cv_firsthumanrestart = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_FIRSTHUMANRESTART]) > 0.f)
         g_b_cv_firsthumanrestart = true;
   }

   g_b_cv_jasonmode = false;
   if (g_rgcvarPointer[PBCVAR_JASONMODE])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_JASONMODE]->value > 0.f)
         g_b_cv_jasonmode = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_JASONMODE]) > 0.f)
         g_b_cv_jasonmode = true;
   }

   if (g_rgcvarPointer[PBCVAR_MINBOTS])  // KWo - 13.10.2006
      g_i_cv_MinBots = (int) g_rgcvarPointer[PBCVAR_MINBOTS]->value;
   else
      g_i_cv_MinBots = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_MINBOTS]);

   if (g_rgcvarPointer[PBCVAR_MAXBOTS])  // KWo - 13.10.2006
      g_i_cv_MaxBots = (int) g_rgcvarPointer[PBCVAR_MAXBOTS]->value;
   else
      g_i_cv_MaxBots = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_MAXBOTS]);

   if (g_rgcvarPointer[PBCVAR_TIMER_SOUND])  // KWo - 13.10.2006
      g_f_cv_timer_sound = g_rgcvarPointer[PBCVAR_TIMER_SOUND]->value;
   else
      g_f_cv_timer_sound = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_TIMER_SOUND]);

   if (g_rgcvarPointer[PBCVAR_TIMER_PICKUP])  // KWo - 13.10.2006
      g_f_cv_timer_pickup = g_rgcvarPointer[PBCVAR_TIMER_PICKUP]->value;
   else
      g_f_cv_timer_pickup = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_TIMER_PICKUP]);

   if (g_rgcvarPointer[PBCVAR_TIMER_GRENADE])  // KWo - 13.10.2006
      g_f_cv_timer_grenade = g_rgcvarPointer[PBCVAR_TIMER_GRENADE]->value;
   else
      g_f_cv_timer_grenade = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_TIMER_GRENADE]);


   g_b_cv_autokill = false;  // KWo - 02.05.2006
   if (g_rgcvarPointer[PBCVAR_AUTOKILL])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_AUTOKILL]->value > 0.f)
         g_b_cv_autokill = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AUTOKILL]) > 0.f)
         g_b_cv_autokill = true;
   }

   if (g_rgcvarPointer[PBCVAR_AUTOKILLDELAY])  // KWo - 13.10.2006
      g_f_cv_autokilldelay = g_rgcvarPointer[PBCVAR_AUTOKILLDELAY]->value;
   else
      g_f_cv_autokilldelay = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AUTOKILLDELAY]);  // KWo - 02.05.2006
   if (g_f_cv_autokilldelay < 0.0)
      g_f_cv_autokilldelay = 0.0;
   else if (g_f_cv_autokilldelay > 300.0)
      g_f_cv_autokilldelay = 300.0;

   if (g_rgcvarPointer[PBCVAR_MAPSTARTBOTJOINDELAY])  // KWo - 17.05.2008
      g_f_cv_MapStartBotJoinDelay = g_rgcvarPointer[PBCVAR_MAPSTARTBOTJOINDELAY]->value;
   else
      g_f_cv_MapStartBotJoinDelay = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_MAPSTARTBOTJOINDELAY]);
   if (g_f_cv_MapStartBotJoinDelay < 0.0)
      g_f_cv_MapStartBotJoinDelay = 0.0;
   else if (g_f_cv_MapStartBotJoinDelay > 3600.0)
      g_f_cv_MapStartBotJoinDelay = 3600.0;

   if (g_rgcvarPointer[PBCVAR_MAXCAMPTIME])  // KWo - 23.03.2008
      g_f_cv_maxcamptime = g_rgcvarPointer[PBCVAR_MAXCAMPTIME]->value;
   else
      g_f_cv_maxcamptime = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_MAXCAMPTIME]);
   if (g_f_cv_maxcamptime < 0.0)
      g_f_cv_maxcamptime = 0.0;
   else if (g_f_cv_maxcamptime > 120.0)
      g_f_cv_maxcamptime = 120.0;

   g_b_cv_ffa = false;  // KWo - 04.10.2006
   if (g_rgcvarPointer[PBCVAR_FFA])  // KWo - 13.10.2006
   {
      if (g_rgcvarPointer[PBCVAR_FFA]->value > 0.f)
         g_b_cv_ffa = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_FFA]) > 0.f)
         g_b_cv_ffa = true;
   }

   g_b_cv_radio = false;  // KWo - 03.02.2007
   if (g_rgcvarPointer[PBCVAR_RADIO])
   {
      if (g_rgcvarPointer[PBCVAR_RADIO]->value > 0.f)
         g_b_cv_radio = true;
   }
   else
   {
      if (CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_RADIO]) > 0.f)
         g_b_cv_radio = true;
   }


   const char *str3;
   if (g_rgcvarPointer[PBCVAR_BOTJOINTEAM])  // KWo - 13.10.2006
      str3 = g_rgcvarPointer[PBCVAR_BOTJOINTEAM]->string; // KWo - 16.09.2006
   else
      str3 = CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_BOTJOINTEAM]); // KWo - 16.09.2006
   if ((FStrEq(str3,"T")) || (FStrEq(str3,"t")))  // KWo - 16.09.2006
      g_i_cv_BotsJoinTeam = 1;
   else if ((FStrEq(str3,"CT")) || (FStrEq(str3,"ct")))
      g_i_cv_BotsJoinTeam = 2;
   else
      g_i_cv_BotsJoinTeam = 0;

   if (g_rgcvarPointer[PBCVAR_BOTQUOTAMATCH])  // KWo - 13.10.2006
      g_i_cv_BotsQuotaMatch = (int) g_rgcvarPointer[PBCVAR_BOTQUOTAMATCH]->value;
   else
      g_i_cv_BotsQuotaMatch = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_BOTQUOTAMATCH]); // KWo - 16.09.2006
   if (g_i_cv_BotsQuotaMatch < 0) // KWo - 16.09.2006
      g_i_cv_BotsQuotaMatch = 0;
   else if (g_i_cv_BotsQuotaMatch > gpGlobals->maxClients - 1)
      g_i_cv_BotsQuotaMatch = gpGlobals->maxClients - 1;

   // botaim2 parameters
   if (g_rgcvarPointer[PBCVAR_AIM_TYPE])  // KWo - 13.10.2006
      g_i_cv_aim_type = (int) g_rgcvarPointer[PBCVAR_AIM_TYPE]->value;
   else
      g_i_cv_aim_type = (int) CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_TYPE]);

   if (g_rgcvarPointer[PBCVAR_AIM_SPRING_STIFFNESS_X])  // KWo - 13.10.2006
      g_f_cv_aim_spring_stiffness_x = g_rgcvarPointer[PBCVAR_AIM_SPRING_STIFFNESS_X]->value;
   else
      g_f_cv_aim_spring_stiffness_x = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_SPRING_STIFFNESS_X]);

   if (g_rgcvarPointer[PBCVAR_AIM_SPRING_STIFFNESS_Y])  // KWo - 13.10.2006
      g_f_cv_aim_spring_stiffness_y = g_rgcvarPointer[PBCVAR_AIM_SPRING_STIFFNESS_Y]->value;
   else
      g_f_cv_aim_spring_stiffness_y = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_SPRING_STIFFNESS_Y]);

   if (g_rgcvarPointer[PBCVAR_AIM_DAMPER_COEFFICIENT_X])  // KWo - 13.10.2006
      g_f_cv_aim_damper_coefficient_x = g_rgcvarPointer[PBCVAR_AIM_DAMPER_COEFFICIENT_X]->value;
   else
      g_f_cv_aim_damper_coefficient_x = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_DAMPER_COEFFICIENT_X]);

   if (g_rgcvarPointer[PBCVAR_AIM_DAMPER_COEFFICIENT_Y])  // KWo - 13.10.2006
      g_f_cv_aim_damper_coefficient_y = g_rgcvarPointer[PBCVAR_AIM_DAMPER_COEFFICIENT_Y]->value;
   else
      g_f_cv_aim_damper_coefficient_y = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_DAMPER_COEFFICIENT_Y]);

   if (g_rgcvarPointer[PBCVAR_AIM_DEVIATION_X])  // KWo - 13.10.2006
      g_f_cv_aim_deviation_x = g_rgcvarPointer[PBCVAR_AIM_DEVIATION_X]->value;
   else
      g_f_cv_aim_deviation_x = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_DEVIATION_X]);

   if (g_rgcvarPointer[PBCVAR_AIM_DEVIATION_Y])  // KWo - 13.10.2006
      g_f_cv_aim_deviation_y = g_rgcvarPointer[PBCVAR_AIM_DEVIATION_Y]->value;
   else
      g_f_cv_aim_deviation_y = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_DEVIATION_Y]);

   if (g_rgcvarPointer[PBCVAR_AIM_INFLUENCE_X_ON_Y])  // KWo - 13.10.2006
      g_f_cv_aim_influence_x_on_y = g_rgcvarPointer[PBCVAR_AIM_INFLUENCE_X_ON_Y]->value;
   else
      g_f_cv_aim_influence_x_on_y = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_INFLUENCE_X_ON_Y]);

   if (g_rgcvarPointer[PBCVAR_AIM_INFLUENCE_Y_ON_X])  // KWo - 13.10.2006
      g_f_cv_aim_influence_y_on_x = g_rgcvarPointer[PBCVAR_AIM_INFLUENCE_Y_ON_X]->value;
   else
      g_f_cv_aim_influence_y_on_x = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_INFLUENCE_Y_ON_X]);

   if (g_rgcvarPointer[PBCVAR_AIM_OFFSET_DELAY])  // KWo - 13.10.2006
      g_f_cv_aim_offset_delay = g_rgcvarPointer[PBCVAR_AIM_OFFSET_DELAY]->value;
   else
      g_f_cv_aim_offset_delay = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_OFFSET_DELAY]);

   if (g_rgcvarPointer[PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO])  // KWo - 13.10.2006
      g_f_cv_aim_notarget_slowdown_ratio = g_rgcvarPointer[PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO]->value;
   else
      g_f_cv_aim_notarget_slowdown_ratio = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO]);

   if (g_rgcvarPointer[PBCVAR_AIM_TARGET_ANTICIPATION_RATIO])  // KWo - 13.10.2006
      g_f_cv_aim_target_anticipation_ratio = g_rgcvarPointer[PBCVAR_AIM_TARGET_ANTICIPATION_RATIO]->value;
   else
      g_f_cv_aim_target_anticipation_ratio = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_AIM_TARGET_ANTICIPATION_RATIO]);

   if (g_rgcvarPointer[PBCVAR_DANGERFACTOR])  // KWo - 13.10.2006
      g_f_cv_dangerfactor = g_rgcvarPointer[PBCVAR_DANGERFACTOR]->value;
   else
      g_f_cv_dangerfactor = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_DANGERFACTOR]);

   if (g_f_cv_dangerfactor < 0)
      g_f_cv_dangerfactor = 0;
   if (g_f_cv_dangerfactor > 5000) // KWo - 25.01.2010
      g_f_cv_dangerfactor = 5000; // KWo - 25.01.2010

   if (g_rgcvarPointer[PBCVAR_PASSWORDKEY])  // KWo - 13.10.2006
      g_sz_cv_PasswordField = g_rgcvarPointer[PBCVAR_PASSWORDKEY]->string;
   else
      g_sz_cv_PasswordField = CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_PASSWORDKEY]);  // KWo - 02.05.2006

   if (g_rgcvarPointer[PBCVAR_PASSWORD])  // KWo - 13.10.2006
      g_sz_cv_Password = g_rgcvarPointer[PBCVAR_PASSWORD]->string;
   else
      g_sz_cv_Password = CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_PASSWORD]);  // KWo - 02.05.2006

   if (g_rgcvarPointer[PBCVAR_WPTFOLDER])  // KWo - 17.11.2006
      g_sz_cv_WPT_Folder = g_rgcvarPointer[PBCVAR_WPTFOLDER]->string;
   else
      g_sz_cv_WPT_Folder = CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_WPTFOLDER]);

/*
   if (!g_p_cv_csdm_active)
      g_p_cv_csdm_active = CVAR_GET_POINTER("csdm_active");
   if (g_p_cv_csdm_active)
   {
      g_b_cv_csdm_active = (g_p_cv_csdm_active->value > 0.f);
   }
   else if (CVAR_GET_FLOAT ("csdm_active") > 0.f)
      g_b_cv_csdm_active = true;
*/
   g_b_cv_csdm_active = false; // KWo - 15.04.2008

   g_i_cv_FpsMax = (int)CVAR_GET_FLOAT ("fps_max");

   g_b_cv_FriendlyFire = false;
   if (CVAR_GET_FLOAT ("mp_friendlyfire") > 0.f)
      g_b_cv_FriendlyFire = true;

   g_b_cv_FootSteps = false;
   if (CVAR_GET_FLOAT ("mp_footsteps") > 0.f)
      g_b_cv_FootSteps = true;

   g_f_cv_c4timer = CVAR_GET_FLOAT ("mp_c4timer");  // KWo - 17.11.2006
   g_f_cv_Gravity = CVAR_GET_FLOAT ("sv_gravity"); // KWo - 16.11.2006
   g_f_cv_FreezeTime = CVAR_GET_FLOAT ("mp_freezetime"); // KWo - 17.11.2006

   g_f_cv_skycolor = CVAR_GET_FLOAT("sv_skycolor_r") + CVAR_GET_FLOAT("sv_skycolor_g") + CVAR_GET_FLOAT("sv_skycolor_b");

   g_b_cv_flash_allowed = false;
   if (CVAR_GET_FLOAT ("mp_flashlight") > 0.f)
      g_b_cv_flash_allowed = true;

   g_b_cv_Parachute = false; // KWo - 07.03.2010
   if (CVAR_GET_FLOAT ("sv_parachute") > 0.f)
      g_b_cv_Parachute = true;

   // // KWo - 02.03.2010 - added ping for bots calculation
   //    int ping, loss, humans;
   //    double ping_av, loss_av;
   //    ping = 0;
   //    loss = 0;
   //    humans = 0;
   //    ping_av = 0;
   //    loss_av = 0;

   // Disabled block of code  // Your Name 27.12.2021
   //    for (pl_index = 0; pl_index < gpGlobals->maxClients; pl_index++)
   //    {
   //       pPlayer = INDEXENT (pl_index + 1);
   // 
   //       if (!FNullEnt (pPlayer) && (pPlayer->v.flags & FL_CLIENT) && (!(pPlayer->v.flags & FL_FAKECLIENT)))
   //       {
   //          infobuffer = GET_INFOKEYBUFFER (clients[pl_index].pEdict);
   //          if (clients[pl_index].iFlags & CLIENT_ADMIN)
   //          {
   //             if ((*g_sz_cv_PasswordField == 0) && (*g_sz_cv_Password == 0))
   //                clients[pl_index].iFlags &= ~CLIENT_ADMIN;
   //             else
   //             {
   //                if (!FStrEq (g_sz_cv_Password, INFOKEY_VALUE (infobuffer, const_cast<char *>(g_sz_cv_PasswordField))))
   //                {
   //                   clients[pl_index].iFlags &= ~CLIENT_ADMIN;
   //                   UTIL_ServerPrint ("Player %s is no longer an admin for podbot mm.\n", STRING(pPlayer->v.netname));
   //                }
   //             }
   //          }
   //          else if ((*g_sz_cv_PasswordField != 0) && (*g_sz_cv_Password != 0))
   //          {
   //             if (FStrEq (g_sz_cv_Password, INFOKEY_VALUE (infobuffer, const_cast<char *>(g_sz_cv_PasswordField))))
   //             {
   //                clients[pl_index].iFlags |= CLIENT_ADMIN;
   //                UTIL_ServerPrint ("Player %s became an admin for podbot mm.\n", STRING(pPlayer->v.netname));
   //             }
   //          }
   //          humans++;
   //          PLAYER_CNX_STATS(pPlayer, &ping, &loss);
   //          if ((ping < 0) || (ping > 4095))
   //             ping = 100;
   //          ping_av += ping;
   //          loss_av += loss;
   //       }
   //    }
   //    if (humans > 0)
   //    {
   //       ping_av = ping_av / humans;
   //       loss_av = loss_av / humans;
   //    }
   //    else
   //    {
   //       ping_av = 100;
   //       loss_av = 0;
   //    }
   //    for (pl_index = 0; pl_index < gpGlobals->maxClients; pl_index++) // thanks to MeRcyLeZZ
   //    {
   //       if (bots[pl_index].is_used)
   //       {
   // //         ping = (int)RANDOM_LONG((int)ping_av - int(0.2 * ping_av), (int)ping_av + int(0.2 * ping_av));
   //          ping = (int)RANDOM_LONG(30, 50);
   //          if (ping < 10)
   //             ping = 10;
   //          else if (ping > 4095)
   //             ping = 4095;
   // 
   //       // First argument's ping
   //          for (bots[pl_index].iOffsetPing[0] = 0; bots[pl_index].iOffsetPing[0] < 4; bots[pl_index].iOffsetPing[0]++)
   //          {
   //             if ((ping - bots[pl_index].iOffsetPing[0]) % 4 == 0)
   //             {
   //                bots[pl_index].iArgPing[0] = (ping - bots[pl_index].iOffsetPing[0]) / 4;
   //                break;
   //             }
   //          }
   //          // Second argument's ping
   //          for (bots[pl_index].iOffsetPing[1] = 0; bots[pl_index].iOffsetPing[1] < 2; bots[pl_index].iOffsetPing[1]++)
   //          {
   //             if ((ping - bots[pl_index].iOffsetPing[1]) % 2 == 0)
   //             {
   //                bots[pl_index].iArgPing[1] = (ping - bots[pl_index].iOffsetPing[1]) / 2;
   //                break;
   //             }
   //          }
   //          // Third argument's ping
   //          bots[pl_index].iArgPing[2] = ping;
   //       }
   //    }
   // 
   // 
   // //   UTIL_ServerPrint ("[Debug] Cvars tested. Min_bots = %d, Max_bots = %d .\n", g_i_cv_MinBots, g_i_cv_MaxBots);
   return;
}

void UTIL_SaveHostagesData (void)    // KWo 16.05.2006
{
   edict_t *pEnt = NULL;
   int i;
   for (i=0; i < MAX_HOSTAGES; i++)
   {
      HostagesData[i].EntIndex = 0;
      HostagesData[i].OldOrigin = g_vecZero;
      HostagesData[i].IsMoving = false;
      HostagesData[i].Alive = false;
      HostagesData[i].UserEntIndex = 0;
   }

   g_iNumHostages = 0;
   pEnt = FIND_ENTITY_BY_CLASSNAME (NULL, "hostage_entity");
   while (!FNullEnt (pEnt))
   {
      if (g_iNumHostages < MAX_HOSTAGES)
      {
         HostagesData[g_iNumHostages].EntIndex = ENTINDEX(pEnt);
         HostagesData[g_iNumHostages].OldOrigin = pEnt->v.origin; // KWo - 16.06.2006
         HostagesData[g_iNumHostages].IsMoving = false;
         if (pEnt->v.health > 0.0)
            HostagesData[g_iNumHostages].Alive = true;
         HostagesData[g_iNumHostages].UserEntIndex = 0;
         g_iNumHostages++;
      }
      pEnt = FIND_ENTITY_BY_CLASSNAME (pEnt, "hostage_entity");
   }
/*
   if (g_iNumHostages > 0)
   {
      UTIL_ServerPrint ("[Debug] Found %i hostages on the map.\n", g_iNumHostages);
      for (i=0; i < g_iNumHostages; i++)
      {
         UTIL_ServerPrint ("[Debug] Hostage %i , EntIndex = %i, origin = [%f, %f, %f].\n", i+1, HostagesData[i].EntIndex,
            HostagesData[i].OldOrigin(0), HostagesData[i].OldOrigin(1), HostagesData[i].OldOrigin(2));
      }
   }

   else
   {
      UTIL_ServerPrint ("[Debug] No hostage found.\n");
   }
*/
   return;
}

void UTIL_CheckHostages (void)    // KWo 17.05.2006
{
   edict_t *pHostage = NULL;
   edict_t *pEntity = NULL; // KWo - 16.07.2006
   int i, j;
   short HostUser;
   Vector vecHostPos = g_vecZero; // KWo - 26.08.2006
   bool bTakeHostage = FALSE; // KWo - 26.08.2006
   bool bBotHasHost = FALSE; // KWo - 26.08.2006

//   ALERT(at_logged, "[DEBUG] Calling UTIL_CheckHostages function; number of hostages = %i.\n", g_iNumHostages);
   for (i=0; i < g_iNumHostages; i++)
   {
      pHostage = INDEXENT(HostagesData[i].EntIndex);
      if (!FNullEnt(pHostage))
      {
         if ((pHostage->v.health > 0.0) && (pHostage->v.deadflag == 0))
            HostagesData[i].Alive = true;
         else
            HostagesData[i].Alive = false;

         if (Length(pHostage->v.origin - HostagesData[i].OldOrigin) > 20.0)
            HostagesData[i].IsMoving = true;
         else
            HostagesData[i].IsMoving = false;

         HostagesData[i].OldOrigin = pHostage->v.origin;

#if !defined __amd64__
         uint32_t following = *((uint32_t *)pHostage->pvPrivateData + OFFSET_HOSTAGEFOLLOW);
#else
         uint64_t following = *((uint64_t *)pHostage->pvPrivateData + OFFSET_HOSTAGEFOLLOW);
#endif

         if (following == 0)
         {
            if ((HostagesData[i].UserEntIndex > 0) && (HostagesData[i].Alive))
            {
               vecHostPos = pHostage->v.origin;  // KWo - 26.08.2006
               bTakeHostage = true;  // KWo - 26.08.2006
//               ALERT(at_logged, "[DEBUG] Hostage %d lost his user(1).\n", i);
            }
            HostagesData[i].UserEntIndex = 0;
         }
         else
        // Else this is probably a pointer to an entity's edict.
         {
            pEntity = (edict_t*)following;
            if (FNullEnt(pEntity))
            {
//               ALERT(at_logged, "[DEBUG] Unknown error finding hostage parameter.\n");
               HostagesData[i].UserEntIndex = 0;
            }
            else
            {
               HostUser = ENTINDEX(pEntity);
               if ((clients[HostUser-1].iFlags & CLIENT_ALIVE) && (clients[HostUser-1].iFlags & CLIENT_USED)
                    && (Length(clients[HostUser-1].vOrigin - pHostage->v.origin) <= 600.0))
               {
                  HostagesData[i].UserEntIndex = HostUser;
//                  ALERT(at_logged, "[DEBUG] Found hostage's %d user - %s.\n", i, STRING(pEntity->v.netname));
               }
               else
               {
/*
#if !defined __amd64__
                  *((int *)pHostage->pvPrivateData + OFFSET_HOSTAGEFOLLOW) = 0;
#else
                  *((long *)pHostage->pvPrivateData + OFFSET_HOSTAGEFOLLOW) = 0;
#endif
*/
               }
            }
         }

         HostUser = HostagesData[i].UserEntIndex;
         if ((HostUser > 0) && (HostUser <= gpGlobals->maxClients))
         {
            if ((bots[HostUser-1].is_used) && !FNullEnt(pHostage))  // KWo - 16.07.2006
            {
               if (!HostagesData[i].Alive || (Length(pEntity->v.origin - pHostage->v.origin) > 600.0))
               {
                  for (j = 0; j < MAX_HOSTAGES; j++)
                  {
                     if (bots[HostUser-1].pHostages[j] == pHostage)
                     {
//                        ALERT(at_logged, "[DEBUG] Bot %s lost a hostage %d.\n", bots[HostUser-1].name, i);
                        bots[HostUser-1].pHostages[j] = NULL;
                     }
                  }
               }
            }

            if ((!(clients[HostUser-1].iFlags & CLIENT_USED) || !(clients[HostUser-1].iFlags & CLIENT_ALIVE)
               || (Length(clients[HostUser-1].vOrigin - pHostage->v.origin) > 600.0)) && (HostagesData[i].Alive))
            {
/*
#if !defined __amd64__
               *((int *)pHostage->pvPrivateData + OFFSET_HOSTAGEFOLLOW) = 0;
#else
               *((long *)pHostage->pvPrivateData + OFFSET_HOSTAGEFOLLOW) = 0;
#endif
*/
               HostagesData[i].UserEntIndex = 0;
               if (HostagesData[i].Alive)  // KWo - 26.08.2006
               {
                  vecHostPos = pHostage->v.origin;  // KWo - 26.08.2006
                  bTakeHostage = true;  // KWo - 26.08.2006
//                  ALERT(at_logged, "[DEBUG] Hostage %d lost his user(2).\n", i);
               }
            }
         }
      }
   }
// some hostage lost his "user" - need pickup him again :)
   if (bTakeHostage) // KWo - 26.08.2006
   {
      for (i=0; i < gpGlobals->maxClients; i++)
      {
         if ((bots[i].is_used) && !(bots[i].bDead) && (bots[i].bot_team == TEAM_CS_COUNTER))
         {
            bBotHasHost = FALSE;
            for (j = 0; j < g_iNumHostages; j++)
            {
               if (!FNullEnt (bots[i].pHostages[j]))
               {
                  if ((bots[i].pHostages[j]->v.health > 0) || (Length(bots[i].pHostages[j]->v.origin - bots[i].pEdict->v.origin) < 600))
                  {
                     bBotHasHost = true;
                  }
               }
            }
            if (!bBotHasHost)
            {
               bots[i].vecPosition = vecHostPos;
               int iIndex = WaypointFindNearestToMove (pHostage, vecHostPos); // KWo - 28.08.2006 - if it will be working correctly , I can randomize the position a bit...
               bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION, iIndex, 0.0, TRUE};
               BotPushTask (&bots[i], &TempTask);
//               ALERT(at_logged, "[DEBUG] Bot %s goes to take a lost hostage.\n", bots[i].name);
            }
         }
      }
   }
   return;
}


bool UTIL_CanUseWeapon (int iId)    // KWo 11.03.2006 - to use in future release to prevent pickup restricted weapons
{
   switch (iId)
   {
      case CS_WEAPON_P228:
         return(g_iWeaponRestricted[PB_WEAPON_P228]==0);
         break;
      case CS_WEAPON_SHIELDGUN:
         return(g_iWeaponRestricted[PB_WEAPON_SHIELDGUN]==0);
         break;
      case CS_WEAPON_SCOUT:
         return(g_iWeaponRestricted[PB_WEAPON_SCOUT]==0);
         break;
      case CS_WEAPON_HEGRENADE:
         return(g_iEquipAmmoRestricted[PB_WEAPON_HEGRENADE]==0);
         break;
      case CS_WEAPON_XM1014:
         return(g_iWeaponRestricted[PB_WEAPON_XM1014]==0);
         break;
      case CS_WEAPON_C4:
         return(true);
         break;
      case CS_WEAPON_MAC10:
         return(g_iWeaponRestricted[PB_WEAPON_MAC10]==0);
         break;
      case CS_WEAPON_AUG:
         return(g_iWeaponRestricted[PB_WEAPON_AUG]==0);
         break;
      case CS_WEAPON_SMOKEGRENADE:
         return(g_iEquipAmmoRestricted[PB_WEAPON_SMOKEGRENADE]==0);
         break;
      case CS_WEAPON_ELITE:
         return(g_iWeaponRestricted[PB_WEAPON_ELITE]==0);
         break;
      case CS_WEAPON_FIVESEVEN:
         return(g_iWeaponRestricted[PB_WEAPON_FIVESEVEN]==0);
         break;
      case CS_WEAPON_UMP45:
         return(g_iWeaponRestricted[PB_WEAPON_UMP45]==0);
         break;
      case CS_WEAPON_SG550:
         return(g_iWeaponRestricted[PB_WEAPON_SG550]==0);
         break;
      case CS_WEAPON_GALIL:
         return(g_iWeaponRestricted[PB_WEAPON_GALIL]==0);
         break;
      case CS_WEAPON_FAMAS:
         return(g_iWeaponRestricted[PB_WEAPON_FAMAS]==0);
         break;
      case CS_WEAPON_USP:
         return(g_iWeaponRestricted[PB_WEAPON_USP]==0);
         break;
      case CS_WEAPON_GLOCK18:
         return(g_iWeaponRestricted[PB_WEAPON_GLOCK18]==0);
         break;
      case CS_WEAPON_AWP:
         return(g_iWeaponRestricted[PB_WEAPON_AWP]==0);
         break;
      case CS_WEAPON_MP5NAVY:
         return(g_iWeaponRestricted[PB_WEAPON_MP5NAVY]==0);
         break;
      case CS_WEAPON_M249:
         return(g_iWeaponRestricted[PB_WEAPON_M249]==0);
         break;
      case CS_WEAPON_M3:
         return(g_iWeaponRestricted[PB_WEAPON_M3]==0);
         break;
      case CS_WEAPON_M4A1:
         return(g_iWeaponRestricted[PB_WEAPON_M4A1]==0);
         break;
      case CS_WEAPON_TMP:
         return(g_iWeaponRestricted[PB_WEAPON_TMP]==0);
         break;
      case CS_WEAPON_G3SG1:
         return(g_iWeaponRestricted[PB_WEAPON_G3SG1]==0);
         break;
      case CS_WEAPON_FLASHBANG:
         return(g_iEquipAmmoRestricted[PB_WEAPON_FLASHBANG]==0);
         break;
      case CS_WEAPON_DEAGLE:
         return(g_iWeaponRestricted[PB_WEAPON_DEAGLE]==0);
         break;
      case CS_WEAPON_SG552:
         return(g_iWeaponRestricted[PB_WEAPON_SG552]==0);
         break;
      case CS_WEAPON_AK47:
         return(g_iWeaponRestricted[PB_WEAPON_AK47]==0);
         break;
      case CS_WEAPON_KNIFE:
         return(true);
         break;
      case CS_WEAPON_P90:
         return(g_iWeaponRestricted[PB_WEAPON_P90]==0);
         break;
   }
   return(false);
}

void UTIL_FindButtonInSphere (signed char &cButtonIndex, const Vector &vecCenter, float flRadius)    // KWo 09.02.2006
{
   signed char cIndex;
   int i;
   float fDistance;

   if (g_iNumButtons <= 0)
   {
      cButtonIndex = -1;
      return;
   }

   if (cButtonIndex > -1)
      cIndex = cButtonIndex;
   else
      cIndex = 0;

   for (i = cIndex; i < g_iNumButtons; i++)
   {
      fDistance = Length(vecCenter - ButtonsData[i].origin);
      if (fDistance < flRadius)
      {
         cButtonIndex = i;
         return;
      }
   }
   cButtonIndex = -1;
   return;
}

float UTIL_GetVectorsCone (Vector vec1_start, Vector vec1_end, Vector vec2_start, Vector vec2_end)  // KWo - 13.02.2006
{
   float fCone;
   Vector vec1Dir = Normalize(vec1_start - vec1_end);
   Vector vec2Dir = Normalize(vec2_start - vec2_end);

   fCone = sqrt((vec1Dir(0)-vec2Dir(0))*(vec1Dir(0)-vec2Dir(0))+(vec1Dir(1)-vec2Dir(1))*(vec1Dir(1)-vec2Dir(1))
           +(vec1Dir(2)-vec2Dir(2))*(vec1Dir(2)-vec2Dir(2)));
   return (fCone);
}

void UTIL_DrawBeam (Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed)  // KWo - 21.03.2006 to see how bots test corridors
{
   if (FNullEnt (pHostEdict))
      return; // reliability check
   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pHostEdict);
   WRITE_BYTE (TE_BEAMPOINTS);
   WRITE_COORD (start(0));
   WRITE_COORD (start(1));
   WRITE_COORD (start(2));
   WRITE_COORD (end(0));
   WRITE_COORD (end(1));
   WRITE_COORD (end(2));
   WRITE_SHORT (g_pSpriteTexture);
   WRITE_BYTE (1); // framestart
   WRITE_BYTE (10); // framerate
   WRITE_BYTE (life); // life in 0.1's
   WRITE_BYTE (width); // width
   WRITE_BYTE (noise); // noise
   WRITE_BYTE (red); // r, g, b
   WRITE_BYTE (green); // r, g, b
   WRITE_BYTE (blue); // r, g, b
   WRITE_BYTE (brightness); // brightness
   WRITE_BYTE (speed); // speed
   MESSAGE_END ();
}

void UTIL_CheckSmokeGrenades(void) // KWo - 29.01.2008
{
   edict_t *pEdict;
   bot_t *pBot = NULL;
   edict_t *pent;
   Vector vecView;
   float fDistance;
   float fDistanceMoved;
   int bot_index;
   bool bSmoke; // KWo - 13.09.2008

   for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
   {
      if (bots[bot_index].is_used
         && !FNullEnt (bots[bot_index].pEdict))
      {
         pBot = &bots[bot_index];
         pBot->pAvoidGrenade = NULL;
         pBot->pSmokeGrenade = NULL;
      }
   }

   pent = NULL;
   // Find all Grenades on the map
   while (!FNullEnt (pent = FIND_ENTITY_BY_STRING (pent, "classname", "grenade")))
   {
      bSmoke = FALSE; // KWo - 13.09.2008
      // If Grenade is invisible don't care for it
      if (pent->v.effects & EF_NODRAW)
         continue;

      if (FStrEq (STRING (pent->v.model), "models/w_smokegrenade.mdl")
                && (pent->v.flags & FL_ONGROUND) && (pent->v.movetype == MOVETYPE_BOUNCE))  // KWo - 13.09.2008
         bSmoke = TRUE;

      for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
      {
         if (bots[bot_index].is_used
            && !FNullEnt (bots[bot_index].pEdict)
            && !pBot->bDead)
         {
            pBot = &bots[bot_index];
            pEdict = pBot->pEdict;

            if (bSmoke) // KWo - 13.09.2008
            {
               // Is this a SmokeGrenade and on Ground (smoking) ?
               if (BotEntityIsVisible (pBot, pent->v.origin))
               {
                  pBot->pSmokeGrenade = pent;
                  continue;
               }
            }

            // Check if visible to the Bot
            vecView = GetGunPosition (pEdict);

            if (BotInFieldOfView (pBot, pent->v.origin - vecView) > (pEdict->v.fov * 0.5 - 5))
               continue;
            if (!BotEntityIsVisible (pBot, pent->v.origin))
               continue;

            if (FNullEnt (pBot->pAvoidGrenade))
            {
               // Is this a flying Grenade ?
               if (!(pent->v.flags & FL_ONGROUND) /* && (pent->v.velocity.Length() > 240.0) */) // KWo - 13.09.2008
               {
                  fDistance = Length(pent->v.origin - pEdict->v.origin);
                  fDistanceMoved = Length((pent->v.origin + pent->v.velocity * pBot->fTimeFrameInterval) - pEdict->v.origin); // KWo - 17.10.2006 - reverted back

                  // Is the Grenade approaching this Bot ?
                  if ((fDistanceMoved < fDistance) && (fDistance < 512))
                  {
                     pBot->pAvoidGrenade = pent;
                  }
               }
            }
         }
      }
   }
   return;
}

float UTIL_IlluminationOf (edict_t *pEdict) // KWo - 23.03.2012 - rewritten - thanks to Immortal_BLG
{
   // this function returns a value between 0 and 100 corresponding to the entity's illumination.
   // Thanks to William van der Sterren for the human-like illumination filter computation. We
   // only consider noticeable the illuminations between 0 and 30 percent of the maximal value,
   // else it's too bright to be taken in account and we return the full illumination. The HL
   // engine allows entities to have illuminations up to 300 (hence the 75 as 30% of 300). - from RACC


   int entity_index;

   entity_index = ENTINDEX (pEdict) - 1; // get entity index

   // PMB - if pEdict is a bot, we had to create an invisible entity to correctly retrieve the
   // fakeclient's illumination (thanks to Tom Simpson from FoxBot for this engine bug fix)
   // KWo - added it for all clients - maybe there are also other bots, heh?
   if (!FNullEnt(pEdict))
   {
      if ((entity_index >= 0) && (entity_index < 32) && (pEdict->v.flags & FL_FAKECLIENT))
         return (100 * sqrt (std::min (75.f, (float) pEdict->v.light_level) / 75.0));
      else
         return (100 * sqrt (std::min (75.f, (float) GETENTITYILLUM (pEdict)) / 75.0));
   }
   return (0.0);
}

void SetBotNvg(bot_t *pBot, bool setnv)
{
   // Give/take nvgoogles..
   // setnv => true => give, false => remove

   // Make into edict pointer
   edict_t *pEdict = pBot->pEdict;

   int* nightgoogles = ((int *)pEdict->pvPrivateData + OFFSET_NVGOOGLES);

   if (setnv)
   {
      if (!(*nightgoogles & HAS_NVGOOGLES))
         *nightgoogles |= HAS_NVGOOGLES;
   }
   else
      *nightgoogles &= ~HAS_NVGOOGLES;

   return;
}


bool BotHasNvg(bot_t *pBot)
{
   // Does the bot have night vision googles?
   // Make into edict pointer
   edict_t *pEdict = pBot->pEdict;

   if ((int)*((int *)pEdict->pvPrivateData + OFFSET_NVGOOGLES) & HAS_NVGOOGLES)
      return (true);

   return (false);
}

// void UTIL_HudMessage(edict_t *pEntity, const hudtextparms_t &textparms, char *pMessage) // KWo - 16.01.2010 from AMX X
// {
//    if (pEntity)
//       MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pEntity);
//    else
//       MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

//    WRITE_BYTE(TE_TEXTMESSAGE);
//    WRITE_BYTE(textparms.channel & 0xFF);                          // channel
//    WRITE_SHORT(FixedSigned16(textparms(0), (1<<13)));              // x coordinates * 8192
//    WRITE_SHORT(FixedSigned16(textparms(1), (1<<13)));              // y coordinates * 8192
//    WRITE_BYTE(textparms.effect);                                  // effect (fade in/out)
//    WRITE_BYTE(textparms.r1);                                      // initial RED
//    WRITE_BYTE(textparms.g1);                                      // initial GREEN
//    WRITE_BYTE(textparms.b1);                                      // initial BLUE
//    WRITE_BYTE(textparms.a1);                                      // initial ALPHA
//    WRITE_BYTE(255);                                               // effect RED
//    WRITE_BYTE(255);                                               // effect GREEN
//    WRITE_BYTE(255);                                               // effect BLUE
//    WRITE_BYTE(1);                                                 // effect ALPHA
//    WRITE_SHORT(FixedUnsigned16(textparms.fadeinTime, (1<<8)));    // fade-in time in seconds * 256
//    WRITE_SHORT(FixedUnsigned16(textparms.fadeoutTime, (1<<8)));   // fade-out time in seconds * 256
//    WRITE_SHORT(FixedUnsigned16(textparms.holdTime, (1<<8)));      // hold time in seconds * 256

//    if (textparms.effect == 2)
//       WRITE_SHORT(FixedUnsigned16(textparms.fxTime, (1<<8)));     // effect time in seconds * 256

//    WRITE_STRING(pMessage);
//    MESSAGE_END();

//    g_hudset.x = -1.0;
//    g_hudset.y = -1.0;
//    g_hudset.effect = 0;
//    g_hudset.r1 = 255;
//    g_hudset.g1 = 255;
//    g_hudset.b1 = 255;
//    g_hudset.a1 = 1;
//    g_hudset.fadeinTime = 0.0;
//    g_hudset.fadeoutTime = 0.0;
//    g_hudset.holdTime = 1.0;
//    g_hudset.fxTime = 0.0;
//    g_hudset.channel = 1;
// }

char* UTIL_SplitHudMessage(const char *src)  // KWo - 16.01.2010 from AMX X
{
   static char message[512];
   short b = 0, d = 0, e = 0, c = -1;

   while (src[d] && e < 480)
   {
      if (src[d] == ' ')
      {
         c = e;
      }
      else if (src[d] == '\n')
      {
         c = -1;
         b = 0;
      }

      message[e++] = src[d++];

      if (++b == 69)
      {
         if (c == -1)
         {
            message[e++] = '\n';
            b = 0;
         } else {
            message[c] = '\n';
            b = e - c - 1;
            c = -1;
         }
      }
   }

   message[e] = 0;
   return message;
}

/*
typedef struct hudtextparms_s
{
   float      x;
   float      y;
   int        effect;
   byte       r1, g1, b1, a1;
   byte       r2, g2, b2, a2;
   float      fadeinTime;
   float      fadeoutTime;
   float      holdTime;
   float      fxTime;
   int        channel;
} hudtextparms_t;
*/
