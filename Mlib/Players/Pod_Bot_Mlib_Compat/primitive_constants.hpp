#pragma once
#include <cstddef>
#include <cstdint>

static const size_t MAXLIGHTMAPS = 4;
static const float VOL_NORM = 1.f;
static const float PITCH_NORM = 1.f;
static const float ATTN_NORM = 0.8f;

static const uint8_t TE_BEAMPOINTS = 0;
static const uint8_t TE_SHOWLINE = 102;
static const uint8_t TE_DECAL = 104;
static const uint8_t TE_WORLDDECAL = 116;
static const uint8_t TE_WORLDDECALHIGH = 117;
static const uint8_t TE_DECALHIGH = 118;

enum InputButton {
    IN_ATTACK    = (1 << 0),
    IN_JUMP      = (1 << 1),
    IN_DUCK      = (1 << 2),
    IN_FORWARD   = (1 << 3),
    IN_BACK      = (1 << 4),
    IN_USE       = (1 << 5),
    IN_CANCEL    = (1 << 6),
    IN_LEFT      = (1 << 7),
    IN_RIGHT     = (1 << 8),
    IN_MOVELEFT  = (1 << 9),
    IN_MOVERIGHT = (1 << 10),
    IN_ATTACK2   = (1 << 11),
    IN_RUN       = (1 << 12),
    IN_RELOAD    = (1 << 13),
    IN_ALT1      = (1 << 14),
    IN_SCORE     = (1 << 15),
};

enum Contents {
    CONTENTS_EMPTY = -1,
    CONTENTS_SOLID = -2,
    CONTENTS_WATER = -3,
    CONTENTS_SLIME = -4,
    CONTENTS_LAVA = -5,
    CONTENTS_SKY = -6,
};

enum EdictFlags {
    FL_FLY =            (1<<0),
    FL_SWIM =           (1<<1),
    FL_CONVEYOR =       (1<<2),
    FL_CLIENT =         (1<<3),
    FL_INWATER =        (1<<4),
    FL_MONSTER =        (1<<5),
    FL_GODMODE =        (1<<6),
    FL_NOTARGET =       (1<<7),
    FL_SKIPLOCALHOST =  (1<<8),
    FL_ONGROUND =       (1<<9),
    FL_PARTIALGROUND =  (1<<10),
    FL_WATERJUMP =      (1<<11),
    FL_FROZEN =         (1<<12),
    FL_FAKECLIENT =     (1<<13),
    FL_DUCKING =        (1<<14),
    FL_FLOAT =          (1<<15),
    FL_GRAPHED =        (1<<16),
    FL_IMMUNE_WATER =   (1<<17),
    FL_IMMUNE_SLIME =   (1<<18),
    FL_IMMUNE_LAVA =    (1<<19),
    FL_PROXY =          (1<<20),
    FL_ALWAYSTHINK =    (1<<21),
    FL_BASEVELOCITY =   (1<<22),
    FL_MONSTERCLIP =    (1<<23),
    FL_ONTRAIN =        (1<<24),
    FL_WORLDBRUSH =     (1<<25),
    FL_SPECTATOR =      (1<<26),
    FL_CUSTOMENTITY =   (1<<29),
    FL_KILLME =         (1<<30),
    FL_DORMANT =        (1<<31),
};

enum MoveType {
    MOVETYPE_NONE          = 0,
    MOVETYPE_WALK          = 3,
    MOVETYPE_STEP          = 4,
    MOVETYPE_FLY           = 5,
    MOVETYPE_TOSS          = 6,
    MOVETYPE_PUSH          = 7,
    MOVETYPE_NOCLIP        = 8,
    MOVETYPE_FLYMISSILE    = 9,
    MOVETYPE_BOUNCE        = 10,
    MOVETYPE_BOUNCEMISSILE = 11,
    MOVETYPE_FOLLOW        = 12,
    MOVETYPE_PUSHSTEP      = 13,
};

enum MessageType {
    MSG_BROADCAST =      0,
    MSG_ONE =            1,
    MSG_ALL =            2,
    MSG_INIT =           3,
    MSG_PVS =            4,
    MSG_PAS =            5,
    MSG_PVS_R =          6,
    MSG_PAS_R =          7,
    MSG_ONE_UNRELIABLE = 8,
    MSG_SPEC =           9,
};

enum RenderFx {	
    kRenderFxNone,
    kRenderFxPulseSlow,
    kRenderFxPulseFast,
    kRenderFxPulseSlowWide,
    kRenderFxPulseFastWide,
    kRenderFxFadeSlow,
    kRenderFxFadeFast,
    kRenderFxSolidSlow,
    kRenderFxSolidFast,
    kRenderFxStrobeSlow,
    kRenderFxStrobeFast,
    kRenderFxStrobeFaster,
    kRenderFxFlickerSlow,
    kRenderFxFlickerFast,
    kRenderFxNoDissipation,
    kRenderFxDistort,
    kRenderFxHologram,
    kRenderFxDeadPlayer,
    kRenderFxExplode,
    kRenderFxGlowShell,
    kRenderFxClampMinScale,
    kRenderFxLightMultiplier,
};

enum EntityEffects {
    EF_BRIGHTFIELD = (1 << 0),
    EF_MUZZLEFLASH = (1 << 1),
    EF_BRIGHTLIGHT = (1 << 2),
    EF_DIMLIGHT    = (1 << 3),
    EF_INVLIGHT    = (1 << 4),
    EF_NOINTERP    = (1 << 5),
    EF_LIGHT       = (1 << 6),
    EF_NODRAW      = (1 << 7),
    EF_NIGHTVISION = (1 << 8),
    EF_SNIPERLASER = (1 << 9),
    EF_FIBERCAMERA = (1 << 10),
};

enum Channels {
    CHAN_AUTO,
    CHAN_WEAPON,
    CHAN_VOICE,
    CHAN_ITEM,
    CHAN_BODY,
    CHAN_STREAM,
    CHAN_STATIC,
    CHAN_NETWORKVOICE_BASE,
    CHAN_NETWORKVOICE_END,
    CHAN_BOT,
};

enum RenderingConstants
{	
	kRenderNormal,
	kRenderTransColor,
	kRenderTransTexture,
	kRenderGlow,
	kRenderTransAlpha,
	kRenderTransAdd,
};

enum DeadFlags {
    DEAD_NO =           0,
    DEAD_DYING =        1,
    DEAD_DEAD =         2,
    DEAD_RESPAWNABLE =  3,
    DEAD_DISCARDBODY =  4,
};

enum Solid {
    SOLID_NOT      = 0,
    SOLID_TRIGGER  = 1,
    SOLID_BBOX     = 2,
    SOLID_SLIDEBOX = 3,
    SOLID_BSP      = 4,
};

enum MESSAGE {
    SVC_TEMPENTITY = 23,
    SVC_INTERMISSION = 30,
    SVC_CDTRACK = 32,
    SVC_WEAPONANIM = 35,
    SVC_ROOMTYPE = 37,
    SVC_HLTV = 50
};

typedef enum {
    MRES_UNSET,
    MRES_IGNORED,
    MRES_HANDLED,
    MRES_OVERRIDE,
    MRES_SUPERCEDE,
} META_RES;

static const int FCVAR_ARCHIVE = 1;
static const int FCVAR_USERINFO = 2;
static const int FCVAR_SERVER = 4;
static const int FCVAR_EXTDLL = 8;
static const int FCVAR_CLIENTDLL = 16;
static const int FCVAR_PROTECTED = 32;
static const int FCVAR_SPONLY = 64;
static const int FCVAR_PRINTABLEONLY = 128;
static const int FCVAR_UNLOGGED = 256;

static const int MAX_AMMO_SLOTS = 32;
static const int MAX_WEAPONS = 32;

static const bool TRUE = true;
static const bool FALSE = false;
