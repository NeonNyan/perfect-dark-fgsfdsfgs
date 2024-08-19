//
// Base (MP)
//

#include "stagesetup.h"

extern s32 intro[];
extern u32 props[];
extern struct path paths[];
extern struct ailist ailists[];

struct stagesetup setup = {
	NULL,
	NULL,
	NULL,
	intro,
	props,
	paths,
	ailists,
	NULL,
};

u32 props[] = {
	weapon(0x0100, 0x0000, PAD_MP1_002C, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION00)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0036, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0037, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_002D, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION01)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0038, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0039, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_002E, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION02)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_003A, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_003B, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_002F, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION03)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_003C, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_003D, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_0030, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION04)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_003E, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_003F, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_0031, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION05)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0040, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0041, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_0032, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION06)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0042, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0043, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_0033, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION07)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0044, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0045, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_0034, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION08)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0046, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0047, OBJFLAG_FALL, 0, 0, 1000)
	weapon(0x0100, 0x0000, PAD_MP1_0035, OBJFLAG_FALL, 0, 0, WEAPON_MPLOCATION09)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0048, OBJFLAG_FALL, 0, 0, 1000)
	ammocratemulti(0x0100, MODEL_MULTI_AMMO_CRATE, PAD_MP1_0049, OBJFLAG_FALL, 0, 0, 1000)
	tag(0x01, 1)
	lift(0x0100, MODEL_A51_LIFT_THINWALL, PAD_MP1_004C, OBJFLAG_00000008 | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_IGNOREFLOORCOLOUR | OBJFLAG_INVINCIBLE | OBJFLAG_UNCOLLECTABLE | OBJFLAG_FORCENOBOUNCE | OBJFLAG_01000000 | OBJFLAG_CANNOT_ACTIVATE | OBJFLAG_LIFT_CHECKCEILING, OBJFLAG2_NOFALL | OBJFLAG2_FALLWITHOUTROTATION | OBJFLAG2_IMMUNETOGUNFIRE | OBJFLAG2_BULLETPROOF | OBJFLAG2_IMMUNETOEXPLOSIONS, 0, 1000, 0x004c, 0x004d, -1, -1, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001555, 0x00320000, 0x00000000)
	tag(0x02, 1)
	lift(0x0100, MODEL_A51_LIFT_THINWALL, PAD_MP1_004B, OBJFLAG_00000008 | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_IGNOREFLOORCOLOUR | OBJFLAG_INVINCIBLE | OBJFLAG_UNCOLLECTABLE | OBJFLAG_FORCENOBOUNCE | OBJFLAG_01000000 | OBJFLAG_CANNOT_ACTIVATE | OBJFLAG_LIFT_CHECKCEILING, OBJFLAG2_NOFALL | OBJFLAG2_FALLWITHOUTROTATION | OBJFLAG2_IMMUNETOGUNFIRE | OBJFLAG2_BULLETPROOF | OBJFLAG2_IMMUNETOEXPLOSIONS, 0, 1000, 0x004b, 0x004a, -1, -1, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001555, 0x00320000, 0x00000000)
	door(0x0100, MODEL_DD_LABDOOR, PAD_MP1_004E, OBJFLAG_DEACTIVATED, 0, 0, 1000, 0x0000f851, 0x0000f333, 0x00035555, 0x00035555, 0x00000666, 0, DOORTYPE_VERTICAL, 0x00000000, 0x0000012c, 0x00000000, 0x00000000, 0x00000100)
	door(0x0100, MODEL_DD_LABDOOR, PAD_MP1_004F, OBJFLAG_DEACTIVATED, 0, 0, 1000, 0x0000f851, 0x0000f333, 0x00035555, 0x00035555, 0x00000666, 0, DOORTYPE_VERTICAL, 0x00000000, 0x0000012c, 0x00000000, 0x00000000, 0x00000100)
	door(0x0100, MODEL_DD_LABDOOR, PAD_MP1_0051, OBJFLAG_DEACTIVATED, 0, 0, 1000, 0x0000f851, 0x0000f333, 0x00035555, 0x00035555, 0x00000666, 0, DOORTYPE_VERTICAL, 0x00000000, 0x0000012c, 0x00000000, 0x00000000, 0x00000100)
	door(0x0100, MODEL_DD_LABDOOR, PAD_MP1_0050, OBJFLAG_DEACTIVATED, 0, 0, 1000, 0x0000f851, 0x0000f333, 0x00035555, 0x00035555, 0x00000666, 0, DOORTYPE_VERTICAL, 0x00000000, 0x0000012c, 0x00000000, 0x00000000, 0x00000100)
	door(0x0100, MODEL_DD_LABDOOR, PAD_MP1_0052, OBJFLAG_DEACTIVATED, 0, 0, 1000, 0x0000f851, 0x0000f333, 0x00035555, 0x00035555, 0x00000666, 0, DOORTYPE_VERTICAL, 0x00000000, 0x0000012c, 0x00000000, 0x00000000, 0x00000100)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0053, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0054, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0055, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0056, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0057, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0058, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0059, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_005A, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_005B, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_005C, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_005D, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_005E, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_005F, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0060, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0061, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0062, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0063, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0064, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0065, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0066, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0067, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0068, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_0069, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	stdobject(0x0100, MODEL_A51_CRATE2, PAD_MP1_006A, OBJFLAG_FALL | OBJFLAG_XTOPADBOUNDS | OBJFLAG_YTOPADBOUNDS | OBJFLAG_ZTOPADBOUNDS | OBJFLAG_CORE_GEO_INUSE | OBJFLAG_INVINCIBLE, 0, 0, 1000)
	endprops
};

s32 intro[] = {
	spawn(PAD_MP1_001C)
	spawn(PAD_MP1_001D)
	spawn(PAD_MP1_001E)
	spawn(PAD_MP1_001F)
	spawn(PAD_MP1_0020)
	spawn(PAD_MP1_0021)
	spawn(PAD_MP1_0022)
	spawn(PAD_MP1_0023)
	spawn(PAD_MP1_0024)
	spawn(PAD_MP1_0025)
	spawn(PAD_MP1_0026)
	spawn(PAD_MP1_0027)
	spawn(PAD_MP1_0028)
	spawn(PAD_MP1_0029)
	spawn(PAD_MP1_002A)
	spawn(PAD_MP1_002B)
	case(0, PAD_MP1_0006)
	case_respawn(0, PAD_MP1_0000)
	case_respawn(0, PAD_MP1_0001)
	case_respawn(0, PAD_MP1_0002)
	case_respawn(0, PAD_MP1_0003)
	case_respawn(0, PAD_MP1_0004)
	case_respawn(0, PAD_MP1_0005)
	case(1, PAD_MP1_000D)
	case_respawn(1, PAD_MP1_0007)
	case_respawn(1, PAD_MP1_0008)
	case_respawn(1, PAD_MP1_0009)
	case_respawn(1, PAD_MP1_000A)
	case_respawn(1, PAD_MP1_000B)
	case_respawn(1, PAD_MP1_000C)
	case(2, PAD_MP1_0014)
	case_respawn(2, PAD_MP1_000E)
	case_respawn(2, PAD_MP1_000F)
	case_respawn(2, PAD_MP1_0010)
	case_respawn(2, PAD_MP1_0011)
	case_respawn(2, PAD_MP1_0012)
	case_respawn(2, PAD_MP1_0013)
	case(3, PAD_MP1_001B)
	case_respawn(3, PAD_MP1_0015)
	case_respawn(3, PAD_MP1_0016)
	case_respawn(3, PAD_MP1_0017)
	case_respawn(3, PAD_MP1_0018)
	case_respawn(3, PAD_MP1_0019)
	case_respawn(3, PAD_MP1_001A)
	hill(PAD_MP1_00DB)
	hill(PAD_MP1_00F0)
	hill(PAD_MP1_00AA)
	hill(PAD_MP1_00B7)
	endintro
};

struct path paths[] = {
	{ NULL, 0, 0 },
};

u8 func1001_start_lifts[] = {
	activate_lift(1, 0x01)
	activate_lift(2, 0x02)
	set_ailist(CHR_SELF, GAILIST_IDLE)
	endlist
};

u8 func1000_21d4[] = {
	mp_init_simulants
	rebuild_teams
	rebuild_squadrons
	set_ailist(CHR_SELF, GAILIST_IDLE)
	endlist
};

struct ailist ailists[] = {
	{ func1000_21d4,        0x1000 },
	{ func1001_start_lifts, 0x1001 },
	{ NULL, 0 },
};
