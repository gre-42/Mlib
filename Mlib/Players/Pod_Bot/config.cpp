#include "bot_globals.h"

class Initializer {
public:
    Initializer() {
        g_rgcvarPointer[PBCVAR_AIM_DAMPER_COEFFICIENT_X]       = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_DAMPER_COEFFICIENT_X],      "0.22", FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_DAMPER_COEFFICIENT_Y]       = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_DAMPER_COEFFICIENT_Y],      "0.22", FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_DEVIATION_X]                = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_DEVIATION_X],               "2.0",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_DEVIATION_Y]                = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_DEVIATION_Y],               "1.0",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_INFLUENCE_X_ON_Y]           = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_INFLUENCE_X_ON_Y],          "0.25", FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_INFLUENCE_Y_ON_X]           = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_INFLUENCE_Y_ON_X],          "0.17", FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO]    = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO],   "0.5",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_OFFSET_DELAY]               = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_OFFSET_DELAY],              "1.2",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_SPRING_STIFFNESS_X]         = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_SPRING_STIFFNESS_X],        "13.0", FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_SPRING_STIFFNESS_Y]         = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_SPRING_STIFFNESS_Y],        "13.0", FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AIM_TARGET_ANTICIPATION_RATIO]  = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_TARGET_ANTICIPATION_RATIO], "2.2",  FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 04.03.2006
        g_rgcvarPointer[PBCVAR_AIM_TYPE]                       = new cvar_t{g_rgpszPbCvars[PBCVAR_AIM_TYPE],                      "4",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_AUTOKILL]                       = new cvar_t{g_rgpszPbCvars[PBCVAR_AUTOKILL],                      "0.0",  FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 02.05.2006
        g_rgcvarPointer[PBCVAR_AUTOKILLDELAY]                  = new cvar_t{g_rgpszPbCvars[PBCVAR_AUTOKILLDELAY],                 "45.0", FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 02.05.2006
        g_rgcvarPointer[PBCVAR_BOTJOINTEAM]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_BOTJOINTEAM],                   "ANY",  FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 16.09.2006
        g_rgcvarPointer[PBCVAR_BOTQUOTAMATCH]                  = new cvar_t{g_rgpszPbCvars[PBCVAR_BOTQUOTAMATCH],                 "0.0",  FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 16.09.2006
        g_rgcvarPointer[PBCVAR_CHAT]                           = new cvar_t{g_rgpszPbCvars[PBCVAR_CHAT],                          "1",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_DANGERFACTOR]                   = new cvar_t{g_rgpszPbCvars[PBCVAR_DANGERFACTOR],                  "800",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_DETAILNAMES]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_DETAILNAMES],                   "0",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_FFA]                            = new cvar_t{g_rgpszPbCvars[PBCVAR_FFA],                           "0",    FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 04.10.2006
        g_rgcvarPointer[PBCVAR_FIRSTHUMANRESTART]              = new cvar_t{g_rgpszPbCvars[PBCVAR_FIRSTHUMANRESTART],             "0",    FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 04.10.2010
        g_rgcvarPointer[PBCVAR_JASONMODE]                      = new cvar_t{g_rgpszPbCvars[PBCVAR_JASONMODE],                     "0",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_LATENCYBOT]                     = new cvar_t{g_rgpszPbCvars[PBCVAR_LATENCYBOT],                    "0",    FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 16.05.2008
        g_rgcvarPointer[PBCVAR_MAPSTARTBOTJOINDELAY]           = new cvar_t{g_rgpszPbCvars[PBCVAR_MAPSTARTBOTJOINDELAY],          "5",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_MAXBOTS]                        = new cvar_t{g_rgpszPbCvars[PBCVAR_MAXBOTS],                       "0",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_MAXBOTSKILL]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_MAXBOTSKILL],                   "100",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_MAXCAMPTIME]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_MAXCAMPTIME],                   "30",   FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 30.23.2008
        g_rgcvarPointer[PBCVAR_MAXWEAPONPICKUP]                = new cvar_t{g_rgpszPbCvars[PBCVAR_MAXWEAPONPICKUP],               "3",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_MINBOTS]                        = new cvar_t{g_rgpszPbCvars[PBCVAR_MINBOTS],                       "0",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_MINBOTSKILL]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_MINBOTSKILL],                   "60",   FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_NUMFOLLOWUSER]                  = new cvar_t{g_rgpszPbCvars[PBCVAR_NUMFOLLOWUSER],                 "3",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_PASSWORD]                       = new cvar_t{g_rgpszPbCvars[PBCVAR_PASSWORD],                      "",     FCVAR_EXTDLL | FCVAR_PROTECTED};
        g_rgcvarPointer[PBCVAR_PASSWORDKEY]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_PASSWORDKEY],                   "_pbadminpw", FCVAR_EXTDLL | FCVAR_PROTECTED};
        g_rgcvarPointer[PBCVAR_RADIO]                          = new cvar_t{g_rgpszPbCvars[PBCVAR_RADIO],                         "1",    FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 03.02.2007
        g_rgcvarPointer[PBCVAR_RESTREQUIPAMMO]                 = new cvar_t{g_rgpszPbCvars[PBCVAR_RESTREQUIPAMMO],                "000000000",    FCVAR_SERVER | FCVAR_EXTDLL};  // KWo - 09.03.2006
        g_rgcvarPointer[PBCVAR_RESTRWEAPONS]                   = new cvar_t{g_rgpszPbCvars[PBCVAR_RESTRWEAPONS],                  "00000000000000000000000000",    FCVAR_SERVER | FCVAR_EXTDLL};  // KWo - 09.03.2006
        g_rgcvarPointer[PBCVAR_SHOOTTHRUWALLS]                 = new cvar_t{g_rgpszPbCvars[PBCVAR_SHOOTTHRUWALLS],                "1",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_SKIN]                           = new cvar_t{g_rgpszPbCvars[PBCVAR_SKIN],                          "5",    FCVAR_SERVER | FCVAR_EXTDLL};      // KWo - 18.11.2006
        g_rgcvarPointer[PBCVAR_SPRAY]                          = new cvar_t{g_rgpszPbCvars[PBCVAR_SPRAY],                         "1",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_TIMER_GRENADE]                  = new cvar_t{g_rgpszPbCvars[PBCVAR_TIMER_GRENADE],                 "0.5",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_TIMER_PICKUP]                   = new cvar_t{g_rgpszPbCvars[PBCVAR_TIMER_PICKUP],                  "0.3",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_TIMER_SOUND]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_TIMER_SOUND],                   "1.0",  FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_USESPEECH]                      = new cvar_t{g_rgpszPbCvars[PBCVAR_USESPEECH],                     "1",    FCVAR_SERVER | FCVAR_EXTDLL};
        g_rgcvarPointer[PBCVAR_VERSION]                        = new cvar_t{g_rgpszPbCvars[PBCVAR_VERSION],                       "",     FCVAR_SERVER | FCVAR_SPONLY};
        g_rgcvarPointer[PBCVAR_WELCOMEMSGS]                    = new cvar_t{g_rgpszPbCvars[PBCVAR_WELCOMEMSGS],                   "1",    FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY};
        g_rgcvarPointer[PBCVAR_WPTFOLDER]                      = new cvar_t{g_rgpszPbCvars[PBCVAR_WPTFOLDER],                     "wptdefault", FCVAR_SERVER | FCVAR_EXTDLL};
    }
} initializer;
