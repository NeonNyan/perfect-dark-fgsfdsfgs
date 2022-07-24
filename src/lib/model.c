#include <ultra64.h>
#include "constants.h"
#include "game/game_096700.h"
#include "game/acosfasinf.h"
#include "game/quaternion.h"
#include "game/camera.h"
#include "game/floor.h"
#include "game/ceil.h"
#include "game/tex.h"
#include "game/gfxmemory.h"
#include "game/bg.h"
#include "game/file.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/model.h"
#include "data.h"
#include "types.h"

#if VERSION >= VERSION_PAL_BETA
u8 var8005efb0_2 = 0;
#endif

u32 var8005efb0 = 0;

bool g_ModelDistanceDisabled = false;
f32 g_ModelDistanceScale = 1;
bool var8005efbc = false;
f32 var8005efc0 = 0;
bool (*var8005efc4)(struct model *model, struct modelnode *node) = NULL;

#if VERSION >= VERSION_PAL_BETA
bool var8005efd8_2 = false;
#endif

struct gfxvtx *(*g_ModelVtxAllocatorFunc)(s32 numvertices) = NULL;
void (*g_ModelJointPositionedFunc)(s32 mtxindex, Mtxf *mtx) = NULL;

void modelSetDistanceChecksDisabled(bool disabled)
{
	g_ModelDistanceDisabled = disabled;
}

void modelSetDistanceScale(f32 scale)
{
	g_ModelDistanceScale = scale;
}

void modelSetVtxAllocatorFunc(struct gfxvtx *(*fn)(s32 numvertices))
{
	g_ModelVtxAllocatorFunc = fn;
}

s32 model0001a524(struct modelnode *node, s32 arg1)
{
	s32 index;
	union modelrodata *rodata1;
	union modelrodata *rodata2;
	union modelrodata *rodata3;

	while (node) {
		switch (node->type & 0xff) {
		case MODELNODETYPE_CHRINFO:
			rodata1 = node->rodata;
			return rodata1->chrinfo.mtxindex;
		case MODELNODETYPE_POSITION:
			rodata2 = node->rodata;
			return rodata2->position.mtxindexes[arg1 == 0x200 ? 2 : (arg1 == 0x100 ? 1 : 0)];
		case MODELNODETYPE_POSITIONHELD:
			rodata3 = node->rodata;
			return rodata3->positionheld.mtxindex;
		}

		node = node->parent;
	}

	return -1;
}

Mtxf *model0001a5cc(struct model *model, struct modelnode *node, s32 arg2)
{
	s32 index = model0001a524(node, arg2);

	if (index >= 0) {
		return &model->matrices[index];
	}

	return NULL;
}

Mtxf *model0001a60c(struct model *model)
{
	return model0001a5cc(model, model->filedata->rootnode, 0);
}

struct modelnode *model0001a634(struct model *model, s32 mtxindex)
{
	struct modelnode *node = model->filedata->rootnode;
	union modelrodata *rodata1;
	union modelrodata *rodata2;
	union modelrodata *rodata3;

	while (node) {
		switch (node->type & 0xff) {
		case MODELNODETYPE_CHRINFO:
			rodata1 = node->rodata;
			if (mtxindex == rodata1->chrinfo.mtxindex) {
				return node;
			}
			break;
		case MODELNODETYPE_POSITION:
			rodata2 = node->rodata;
			if (mtxindex == rodata2->position.mtxindexes[0]
					|| mtxindex == rodata2->position.mtxindexes[1]
					|| mtxindex == rodata2->position.mtxindexes[2]) {
				return node;
			}
			break;
		case MODELNODETYPE_POSITIONHELD:
			rodata3 = node->rodata;
			if (mtxindex == rodata3->positionheld.mtxindex) {
				return node;
			}
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node && node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}

	return NULL;
}

struct modelnode *model0001a740(struct modelnode *node)
{
	while (node) {
		u32 type = node->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO
				|| type == MODELNODETYPE_POSITION
				|| type == MODELNODETYPE_POSITIONHELD) {
			break;
		}

		node = node->parent;
	}

	return node;
}

struct modelnode *model0001a784(struct modelnode *node)
{
	while ((node = node->parent)) {
		u32 type = node->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO
				|| type == MODELNODETYPE_POSITION
				|| type == MODELNODETYPE_POSITIONHELD) {
			break;
		}
	}

	return node;
}

struct modelnode *model0001a7cc(struct modelnode *basenode)
{
	struct modelnode *node = basenode->child;

	while (node) {
		u32 type = node->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO
				|| type == MODELNODETYPE_POSITION
				|| type == MODELNODETYPE_POSITIONHELD) {
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node == basenode) {
					node = NULL;
					break;
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}

	return node;
}

struct modelnode *model0001a85c(struct modelnode *basenode)
{
	struct modelnode *node = basenode;
	struct modelnode *next;
	u32 type;

	while (node) {
		if (node != basenode && node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node != basenode) {
					type = node->type & 0xff;

					if (type == MODELNODETYPE_CHRINFO
							|| type == MODELNODETYPE_POSITION
							|| type == MODELNODETYPE_POSITIONHELD) {
						node = NULL;
						break;
					}
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}

			if (!node) {
				break;
			}
		}

		type = node->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO
				|| type == MODELNODETYPE_POSITION
				|| type == MODELNODETYPE_POSITIONHELD) {
			break;
		}
	}

	return node;
}

struct modelnode *modelGetPart(struct modelfiledata *modelfiledata, s32 partnum)
{
	s32 upper;
	s32 lower;
	u32 i;
	s16 *partnums;

	if (modelfiledata->numparts == 0) {
		return NULL;
	}

	partnums = (s16 *)&modelfiledata->parts[modelfiledata->numparts];
	lower = 0;
	upper = modelfiledata->numparts;

	while (upper >= lower) {
		i = (lower + upper) / 2;

		if (partnum == partnums[i]) {
			return modelfiledata->parts[i];
		}

		if (partnum < partnums[i]) {
			upper = i - 1;
		} else {
			lower = i + 1;
		}
	}

	return NULL;
}

void *modelGetPartRodata(struct modelfiledata *modelfiledata, s32 partnum)
{
	struct modelnode *node = modelGetPart(modelfiledata, partnum);

	if (node) {
		return node->rodata;
	}

	return NULL;
}

f32 model0001a9e8(struct model *model)
{
	Mtxf *mtx = model0001a60c(model);

	if (mtx) {
		return -mtx->m[3][2];
	}

	return 0;
}

#if VERSION >= VERSION_NTSC_1_0
// ntsc-beta has this function in another file
void *modelGetNodeRwData(struct model *model, struct modelnode *node)
{
	u32 index = 0;
	union modelrwdata **rwdatas = model->rwdatas;

	switch (node->type & 0xff) {
	case MODELNODETYPE_CHRINFO:
		index = node->rodata->chrinfo.rwdataindex;
		break;
	case MODELNODETYPE_DL:
		index = node->rodata->dl.rwdataindex;
		break;
	case MODELNODETYPE_DISTANCE:
		index = node->rodata->distance.rwdataindex;
		break;
	case MODELNODETYPE_TOGGLE:
		index = node->rodata->toggle.rwdataindex;
		break;
	case MODELNODETYPE_REORDER:
		index = node->rodata->reorder.rwdataindex;
		break;
	case MODELNODETYPE_0B:
		index = node->rodata->type0b.rwdataindex;
		break;
	case MODELNODETYPE_CHRGUNFIRE:
		index = node->rodata->chrgunfire.rwdataindex;
		break;
	case MODELNODETYPE_HEADSPOT:
		index = node->rodata->headspot.rwdataindex;
		break;
	}

	while (node->parent) {
		node = node->parent;

		if ((node->type & 0xff) == MODELNODETYPE_HEADSPOT) {
			struct modelrwdata_headspot *tmp = modelGetNodeRwData(model, node);
			rwdatas = tmp->rwdatas;
			break;
		}
	}

	return &rwdatas[index];
}
#endif

void modelNodeGetPosition(struct model *model, struct modelnode *node, struct coord *pos)
{
	switch (node->type & 0xff) {
	case MODELNODETYPE_CHRINFO:
		{
			struct modelrwdata_chrinfo *rwdata = modelGetNodeRwData(model, node);
			pos->x = rwdata->pos.x;
			pos->y = rwdata->pos.y;
			pos->z = rwdata->pos.z;
		}
		break;
	case MODELNODETYPE_POSITION:
		{
			struct modelrodata_position *rodata = &node->rodata->position;
			pos->x = rodata->pos.x;
			pos->y = rodata->pos.y;
			pos->z = rodata->pos.z;
		}
		break;
	case MODELNODETYPE_POSITIONHELD:
		{
			struct modelrodata_positionheld *rodata = &node->rodata->positionheld;
			pos->x = rodata->pos.x;
			pos->y = rodata->pos.y;
			pos->z = rodata->pos.z;
		}
		break;
	default:
		pos->x = 0;
		pos->y = 0;
		pos->z = 0;
		break;
	}
}

void modelNodeSetPosition(struct model *model, struct modelnode *node, struct coord *pos)
{
	switch (node->type & 0xff) {
	case MODELNODETYPE_CHRINFO:
		{
			struct modelrwdata_chrinfo *rwdata = modelGetNodeRwData(model, node);
			struct coord diff[1];

			diff[0].x = pos->x - rwdata->pos.x;
			diff[0].z = pos->z - rwdata->pos.z;

			rwdata->pos.x = pos->x;
			rwdata->pos.y = pos->y;
			rwdata->pos.z = pos->z;

			rwdata->unk24.x += diff[0].x; rwdata->unk24.z += diff[0].z;
			rwdata->unk34.x += diff[0].x; rwdata->unk34.z += diff[0].z;
			rwdata->unk40.x += diff[0].x; rwdata->unk40.z += diff[0].z;
			rwdata->unk4c.x += diff[0].x; rwdata->unk4c.z += diff[0].z;
		}
		break;
	case MODELNODETYPE_POSITION:
		{
			struct modelrodata_position *rodata = &node->rodata->position;
			rodata->pos.x = pos->x;
			rodata->pos.y = pos->y;
			rodata->pos.z = pos->z;
		}
		break;
	case MODELNODETYPE_POSITIONHELD:
		{
			struct modelrodata_positionheld *rodata = &node->rodata->positionheld;
			rodata->pos.x = pos->x;
			rodata->pos.y = pos->y;
			rodata->pos.z = pos->z;
		}
		break;
	}
}

void modelGetRootPosition(struct model *model, struct coord *pos)
{
	modelNodeGetPosition(model, model->filedata->rootnode, pos);
}

void modelSetRootPosition(struct model *model, struct coord *pos)
{
	modelNodeSetPosition(model, model->filedata->rootnode, pos);
}

void modelNodeGetModelRelativePosition(struct model *model, struct modelnode *node, struct coord *pos)
{
	pos->x = 0;
	pos->y = 0;
	pos->z = 0;

	while (node) {
		struct coord nodepos;
		u32 type = node->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO
				|| type == MODELNODETYPE_POSITION
				|| type == MODELNODETYPE_POSITIONHELD) {
			modelNodeGetPosition(model, node, &nodepos);
			pos->x += nodepos.x;
			pos->y += nodepos.y;
			pos->z += nodepos.z;
		}

		node = node->parent;
	}
}

f32 model0001ae44(struct model *model)
{
	if ((model->filedata->rootnode->type & 0xff) == MODELNODETYPE_CHRINFO) {
		union modelrwdata *rwdata = modelGetNodeRwData(model, model->filedata->rootnode);
		return rwdata->chrinfo.unk14;
	}

	return 0;
}

void model0001ae90(struct model *model, f32 angle)
{
	if ((model->filedata->rootnode->type & 0xff) == MODELNODETYPE_CHRINFO) {
		struct modelrwdata_chrinfo *rwdata = modelGetNodeRwData(model, model->filedata->rootnode);
		f32 diff = angle - rwdata->unk14;

		if (diff < 0) {
			diff += M_BADTAU;
		}

		rwdata->unk30 += diff;

		if (rwdata->unk30 >= M_BADTAU) {
			rwdata->unk30 -= M_BADTAU;
		}

		rwdata->unk20 += diff;

		if (rwdata->unk20 >= M_BADTAU) {
			rwdata->unk20 -= M_BADTAU;
		}

		rwdata->unk14 = angle;
	}
}

void modelSetScale(struct model *model, f32 scale)
{
	model->scale = scale;
}

void modelSetAnimScale(struct model *model, f32 scale)
{
	if (model->anim) {
		model->anim->animscale = scale;
	}
}

f32 model0001af80(struct model *model)
{
	return model->filedata->unk10 * model->scale;
}

void model0001af98(struct coord *arg0, struct coord *arg1, f32 frac)
{
	arg0->x += (arg1->x - arg0->x) * frac;
	arg0->y += (arg1->y - arg0->y) * frac;
	arg0->z += (arg1->z - arg0->z) * frac;
}

f32 model0001afe8(f32 arg0, f32 angle, f32 mult)
{
	f32 value = angle - arg0;

	if (angle < arg0) {
		value += M_BADTAU;
	}

	if (value < M_PI) {
		arg0 += value * mult;

		if (arg0 >= M_BADTAU) {
			arg0 -= M_BADTAU;
		}
	} else {
		arg0 -= (M_BADTAU - value) * mult;

		if (arg0 < 0) {
			arg0 += M_BADTAU;
		}
	}

	return arg0;
}

void model0001b07c(struct coord *arg0, struct coord *arg1, f32 mult)
{
	arg0->x = model0001afe8(arg0->x, arg1->x, mult);
	arg0->y = model0001afe8(arg0->y, arg1->y, mult);
	arg0->z = model0001afe8(arg0->z, arg1->z, mult);
}

void model0001b0e8(struct model *model, struct modelnode *node)
{
	union modelrwdata *rwdata;
	struct anim *anim = model->anim;
	struct coord sp34;
	struct coord sp28;
	f32 frac;

	if (!anim) {
		return;
	}

	rwdata = modelGetNodeRwData(model, node);

	if (rwdata->chrinfo.unk00) {
		return;
	}

	sp34.x = rwdata->chrinfo.unk34.x;
	sp34.y = rwdata->chrinfo.unk34.y;
	sp34.z = rwdata->chrinfo.unk34.z;

	rwdata->chrinfo.unk14 = rwdata->chrinfo.unk30;

	if (g_Vars.in_cutscene && anim->speed > 0.0f) {
#if VERSION >= VERSION_PAL_BETA
		frac = floorf(anim->frac / anim->speed + 0.01f) * anim->speed;
#else
		frac = floorf(anim->frac / anim->speed) * anim->speed;
#endif
	} else {
		frac = anim->frac;
	}

	if (frac != 0.0f && rwdata->chrinfo.unk01) {
		model0001af98(&sp34, &rwdata->chrinfo.unk24, frac);

		rwdata->chrinfo.unk14 = model0001afe8(rwdata->chrinfo.unk30, rwdata->chrinfo.unk20, frac);
	}

	if (anim->animnum2 || anim->fracmerge) {
		if (rwdata->chrinfo.unk02) {
			f32 y = rwdata->chrinfo.unk4c.y;

			if (anim->frac2 != 0.0f) {
				y += (rwdata->chrinfo.unk40.y - y) * anim->frac2;
			}

			sp34.y += (y - sp34.y) * anim->fracmerge;
		}
	}

	if (anim->unk70 == NULL) {
		rwdata->chrinfo.pos.x = sp34.x;
		rwdata->chrinfo.pos.y = rwdata->chrinfo.ground + sp34.f[1];
		rwdata->chrinfo.pos.z = sp34.z;
	} else {
		sp28.x = sp34.x;
		sp28.y = sp34.y;
		sp28.z = sp34.z;

		if (anim->unk70(model, &rwdata->chrinfo.pos, &sp28, &rwdata->chrinfo.ground)) {
			rwdata->chrinfo.pos.x = sp28.x;
			rwdata->chrinfo.pos.y = rwdata->chrinfo.ground + sp28.f[1];
			rwdata->chrinfo.pos.z = sp28.z;

			sp34.x = sp28.x - sp34.x;
			sp34.z = sp28.z - sp34.z;

			rwdata->chrinfo.unk34.x += sp34.x;
			rwdata->chrinfo.unk34.z += sp34.z;

			if (rwdata->chrinfo.unk01) {
				rwdata->chrinfo.unk24.x += sp34.x;
				rwdata->chrinfo.unk24.z += sp34.z;
			}

			if (rwdata->chrinfo.unk02) {
				rwdata->chrinfo.unk4c.x += sp34.x;
				rwdata->chrinfo.unk4c.z += sp34.z;

				rwdata->chrinfo.unk40.x += sp34.x;
				rwdata->chrinfo.unk40.z += sp34.z;
			}
		}
	}
}

void model0001b3bc(struct model *model)
{
	struct modelnode *node = model->filedata->rootnode;

	if (node && (node->type & 0xff) == MODELNODETYPE_CHRINFO) {
		model0001b0e8(model, node);
	}
}

void model0001b400(struct modelrenderdata *arg0, struct model *model, struct modelnode *node)
{
	struct anim *anim = model->anim;
	union modelrodata *rodata = node->rodata;
	union modelrwdata *rwdata = modelGetNodeRwData(model, node);
	f32 scale = model->scale;
	struct coord *sp254 = &rwdata->chrinfo.pos;
	f32 sp250 = rwdata->chrinfo.unk14;
	Mtxf *sp24c;
	u32 stack1;
	Mtxf *mtx = &model->matrices[rodata->chrinfo.mtxindex];
	s32 sp240 = rodata->chrinfo.unk00;
	struct skeleton *skel = model->filedata->skel;
	struct coord sp230;
	struct coord sp224;
	struct coord sp218;
	Mtxf sp1d8;
	Mtxf sp198;
	Mtxf sp158;
	f32 sp154;
	struct coord sp148;
	struct coord sp13c;
	struct coord sp130;
	struct coord sp124;
	struct coord sp118;
	struct coord sp10c;
	f32 spfc[4];
	f32 spec[4];
	u8 stack4[0xc];
	f32 spdc;
	struct coord spd0;
	struct coord spc4;
	struct coord spb8;
	Mtxf sp78;
	Mtxf sp38;

	if (rodata->chrinfo.mtxindex);

	if (node->parent) {
		sp24c = model0001a5cc(model, node->parent, 0);
	} else {
		sp24c = arg0->unk00;
	}

	anim00024050(sp240, anim->flip, skel, anim->animnum, anim->unk04, &sp230, &sp224, &sp218);

	if (g_Vars.in_cutscene && anim->speed > 0) {
#if VERSION >= VERSION_PAL_BETA
		sp154 = floorf(anim->frac / anim->speed + 0.01f) * anim->speed;
#else
		sp154 = floorf(anim->frac / anim->speed) * anim->speed;
#endif
	} else {
		sp154 = anim->frac;
	}

	if (sp154 != 0.0f) {
		anim00024050(sp240, anim->flip, skel, anim->animnum, anim->unk05, &sp148, &sp13c, &sp130);
		model0001b07c(&sp230, &sp148, sp154);
	}

	if (anim->fracmerge != 0.0f) {
		anim00024050(sp240, anim->flip2, skel, anim->animnum2, anim->unk06, &sp124, &sp118, &sp10c);

		if (anim->frac2 != 0.0f) {
			anim00024050(sp240, anim->flip2, skel, anim->animnum2, anim->unk07, &spd0, &spc4, &spb8);
			model0001b07c(&sp124, &spd0, anim->frac2);
		}

		if ((g_Anims[anim->animnum].flags & ANIMFLAG_02) && (g_Anims[anim->animnum2].flags & ANIMFLAG_02) == 0) {
			mtx4LoadYRotation(rwdata->chrinfo.unk14, &sp78);
			mtx4LoadRotation(&sp124, &sp38);
			mtx00015be0(&sp78, &sp38);
			quaternion0f097044(&sp38, spec);
		} else {
			quaternion0f096ca0(&sp124, spec);
		}

		quaternion0f096ca0(&sp230, spfc);
		quaternion0f0976c0(spfc, spec);
		quaternionSlerp(spfc, spec, anim->fracmerge, &spdc);
		quaternionToMtx(&spdc, &sp1d8);
	} else {
		mtx4LoadRotation(&sp230, &sp1d8);
	}

	if (g_Anims[anim->animnum].flags & ANIMFLAG_02) {
		mtx4LoadTranslation(sp254, &sp198);
	} else {
		if (rwdata->chrinfo.unk18 != 0.0f) {
			sp250 = model0001afe8(sp250, rwdata->chrinfo.unk1c, rwdata->chrinfo.unk18);
		}

		mtx4LoadYRotationWithTranslation(sp254, sp250, &sp198);
	}

	mtx00015be4(&sp198, &sp1d8, &sp158);

	if (scale != 1.0f) {
		mtx00015f4c(scale, &sp158);
	}

	if (sp24c) {
		mtx00015be4(sp24c, &sp158, mtx);
	} else {
		mtx4Copy(&sp158, mtx);
	}
}

void modelPositionJointUsingVecRot(struct modelrenderdata *renderdata, struct model *model, struct modelnode *node, struct coord *rot, struct coord *pos, bool allowscale, struct coord *arg6)
{
	s32 nodetype = node->type;
	struct modelrodata_position *rodata = &node->rodata->position;
	Mtxf *rendermtx;
	u32 stack;
	Mtxf mtx68;
	s32 mtxindex0 = rodata->mtxindex0;
	s32 mtxindex1 = rodata->mtxindex1;
	s32 mtxindex2 = rodata->mtxindex2;
	Mtxf *matrices = model->matrices;

	if (node->parent != NULL) {
		rendermtx = model0001a5cc(model, node->parent, 0);
	} else {
		rendermtx = renderdata->unk00;
	}

	if (rendermtx != NULL) {
		Mtxf *nodemtx = &matrices[mtxindex0];

		mtx4LoadRotationAndTranslation(pos, rot, &mtx68);

		if (allowscale && model->scale != 1.0f) {
			mtx00015f04(model->scale, &mtx68);
		}

		if (arg6->x != 1.0f) {
			mtx00015df0(arg6->x, &mtx68);
		}

		if (arg6->y != 1.0f) {
			mtx00015e4c(arg6->y, &mtx68);
		}

		if (arg6->z != 1.0f) {
			mtx00015ea8(arg6->z, &mtx68);
		}

		mtx00015be4(rendermtx, &mtx68, nodemtx);

		if (g_ModelJointPositionedFunc != NULL) {
			g_ModelJointPositionedFunc(mtxindex0, nodemtx);
		}
	} else {
		Mtxf *nodemtx = &matrices[mtxindex0];

		mtx4LoadRotationAndTranslation(pos, rot, nodemtx);

		if (allowscale && model->scale != 1.0f) {
			mtx00015f04(model->scale, nodemtx);
		}

		if (arg6->x != 1.0f) {
			mtx00015df0(arg6->x, nodemtx);
		}

		if (arg6->y != 1.0f) {
			mtx00015e4c(arg6->y, nodemtx);
		}

		if (arg6->z != 1.0f) {
			mtx00015ea8(arg6->z, nodemtx);
		}
	}

	if (nodetype & MODELNODETYPE_0100) {
		Mtxf *nodemtx = &matrices[mtxindex1];
		f32 sp3c[4];
		f32 sp2c[4];

		quaternion0f096ca0(rot, sp3c);
		quaternion0f097518(sp3c, 0.5f, sp2c);

		if (rendermtx != NULL) {
			quaternionToTransformMtx(pos, sp2c, &mtx68);
			mtx00015be4(rendermtx, &mtx68, nodemtx);
		} else {
			quaternionToTransformMtx(pos, sp2c, nodemtx);
		}
	}

	if (nodetype & MODELNODETYPE_0200) {
		Mtxf *finalmtx = rendermtx ? &mtx68 : &matrices[mtxindex2];
		f32 roty = rot->y;

		if (roty < M_PI) {
			roty *= 0.5f;
		} else {
			roty = M_BADTAU - (M_BADTAU - roty) * 0.5f;
		}

		mtx4LoadYRotation(roty, finalmtx);

		if (roty >= M_PI) {
			roty = M_BADTAU - roty;
		}

		if (roty < 0.890118f) { // 51 degrees
			roty = func0f096700(roty);
		} else {
			roty = 1.5f;
		}

		mtx00015edc(roty, finalmtx);
		mtx4SetTranslation(pos, finalmtx);

		if (rendermtx != NULL) {
			Mtxf *nodemtx = &matrices[mtxindex2];
			mtx00015be4(rendermtx, finalmtx, nodemtx);
		}
	}
}

void modelPositionJointUsingQuatRot(struct modelrenderdata *renderdata, struct model *model, struct modelnode *node, f32 rot[4], struct coord *pos, struct coord *arg5)
{
	s32 nodetype = node->type;
	struct modelrodata_position *rodata = &node->rodata->position;
	Mtxf *rendermtx;
	u32 stack;
	Mtxf mtx58;
	s32 mtxindex0 = rodata->mtxindex0;
	s32 mtxindex1 = rodata->mtxindex1;
	s32 mtxindex2 = rodata->mtxindex2;
	Mtxf *matrices = model->matrices;

	if (node->parent != NULL) {
		rendermtx = model0001a5cc(model, node->parent, 0);
	} else {
		rendermtx = renderdata->unk00;
	}

	if (rendermtx != NULL) {
		Mtxf *nodemtx = &matrices[mtxindex0];

		quaternionToTransformMtx(pos, rot, &mtx58);

		if (arg5->x != 1.0f) {
			mtx00015df0(arg5->x, &mtx58);
		}

		if (arg5->y != 1.0f) {
			mtx00015e4c(arg5->y, &mtx58);
		}

		if (arg5->z != 1.0f) {
			mtx00015ea8(arg5->z, &mtx58);
		}

		mtx00015be4(rendermtx, &mtx58, nodemtx);

		if (g_ModelJointPositionedFunc != NULL) {
			g_ModelJointPositionedFunc(mtxindex0, nodemtx);
		}
	} else {
		Mtxf *nodemtx = &matrices[mtxindex0];

		quaternionToTransformMtx(pos, rot, nodemtx);

		if (arg5->x != 1.0f) {
			mtx00015df0(arg5->x, nodemtx);
		}

		if (arg5->y != 1.0f) {
			mtx00015e4c(arg5->y, nodemtx);
		}

		if (arg5->z != 1.0f) {
			mtx00015ea8(arg5->z, nodemtx);
		}
	}

	if (nodetype & MODELNODETYPE_0100) {
		Mtxf *nodemtx = &matrices[mtxindex1];
		f32 sp2c[4];

		quaternion0f097518(rot, 0.5f, sp2c);

		if (rendermtx != NULL) {
			quaternionToTransformMtx(pos, sp2c, &mtx58);
			mtx00015be4(rendermtx, &mtx58, nodemtx);
		} else {
			quaternionToTransformMtx(pos, sp2c, nodemtx);
		}
	}

	if (nodetype & MODELNODETYPE_0200) {
		Mtxf *finalmtx = rendermtx ? &mtx58 : &matrices[mtxindex2];
		f32 roty = 2.0f * acosf(rot[0]);

		if (roty < M_PI) {
			roty *= 0.5f;
		} else {
			roty = M_BADTAU - (M_BADTAU - roty) * 0.5f;
		}

		mtx4LoadYRotation(roty, finalmtx);

		if (roty >= M_PI) {
			roty = M_BADTAU - roty;
		}

		if (roty < 0.890118f) { // 51 degrees
			roty = func0f096700(roty);
		} else {
			roty = 1.5f;
		}

		mtx00015edc(roty, finalmtx);
		mtx4SetTranslation(pos, finalmtx);

		if (rendermtx != NULL) {
			Mtxf *nodemtx = &matrices[mtxindex2];
			mtx00015be4(rendermtx, finalmtx, nodemtx);
		}
	}
}

void model0001bfa8(struct modelrenderdata *renderdata, struct model *model, struct modelnode *node)
{
	struct anim *anim;
	struct modelrodata_position *rodata = &node->rodata->position;
	s32 partnum;
	struct skeleton *skel;
	struct coord sp144;
	struct coord sp138;
	struct coord sp12c;
	bool sp128;
	Mtxf spe8;
	Mtxf *mtx;
	f32 spe0;
	struct coord spd4;
	struct coord spc8;
	struct coord spbc;
	struct coord spb0;
	struct coord spa4;
	struct coord sp98;
	f32 sp88[4];
	f32 sp78[4];
	f32 sp68[4];
	struct coord sp5c;
	struct coord sp50;
	struct coord sp44;

	anim = model->anim;

	if (anim != NULL) {
		partnum = rodata->part;
		skel = model->filedata->skel;

		if (anim->animnum != 0) {
			sp128 = (g_Anims[anim->animnum].flags & ANIMFLAG_02) && node == model->filedata->rootnode;

			anim00024050(partnum, anim->flip, skel, anim->animnum, anim->unk04, &sp144, &sp138, &sp12c);

			if (g_Vars.in_cutscene && anim->speed > 0.0f) {
#if VERSION >= VERSION_PAL_BETA
				spe0 = floorf(anim->frac / anim->speed + 0.0099999997764826f) * anim->speed;
#else
				spe0 = floorf(anim->frac / anim->speed) * anim->speed;
#endif
			} else {
				spe0 = anim->frac;
			}

			if (spe0 != 0.0f) {
				anim00024050(partnum, anim->flip, skel, anim->animnum, anim->unk05, &spd4, &spc8, &spbc);
				model0001b07c(&sp144, &spd4, spe0);

#if VERSION >= VERSION_PAL_BETA
				if (sp128 || var8005efd8_2)
#else
				if (sp128)
#endif
				{
					model0001af98(&sp138, &spc8, spe0);
				}
			}
		} else {
			sp138.f[0] = sp138.f[1] = sp138.f[2] = 0.0f;
			sp144.f[0] = sp144.f[1] = sp144.f[2] = 0.0f;
			sp12c.f[0] = sp12c.f[1] = sp12c.f[2] = 1.0f;

			sp128 = false;
		}

		if (anim->fracmerge != 0.0f) {
			anim00024050(partnum, anim->flip2, skel, anim->animnum2, anim->unk06, &spb0, &spa4, &sp98);

			if (anim->frac2 != 0.0f) {
				anim00024050(partnum, anim->flip2, skel, anim->animnum2, anim->unk07, &sp5c, &sp50, &sp44);
				model0001b07c(&spb0, &sp5c, anim->frac2);
			}

			quaternion0f096ca0(&sp144, sp88);
			quaternion0f096ca0(&spb0, sp78);
			quaternion0f0976c0(sp88, sp78);
			quaternionSlerp(sp88, sp78, anim->fracmerge, sp68);

			if (sp138.f[0] != 0.0f || sp138.f[1] != 0.0f || sp138.f[2] != 0.0f) {
				sp138.x *= anim->animscale;
				sp138.y *= anim->animscale;
				sp138.z *= anim->animscale;

				if (node != model->filedata->rootnode) {
					sp138.x += rodata->pos.x;
					sp138.y += rodata->pos.y;
					sp138.z += rodata->pos.z;
				}

				modelPositionJointUsingQuatRot(renderdata, model, node, sp68, &sp138, &sp12c);
			} else if (node != model->filedata->rootnode) {
				modelPositionJointUsingQuatRot(renderdata, model, node, sp68, &rodata->pos, &sp12c);
			} else {
				modelPositionJointUsingQuatRot(renderdata, model, node, sp68, &sp138, &sp12c);
			}
		} else if (sp128) {
			f32 mult = func0f15c888();

			sp138.x *= mult;
			sp138.y *= mult;
			sp138.z *= mult;

			modelPositionJointUsingVecRot(renderdata, model, node, &sp144, &sp138, true, &sp12c);
		} else if (sp138.f[0] != 0.0f || sp138.f[1] != 0.0f || sp138.f[2] != 0.0f) {
			sp138.x *= anim->animscale;
			sp138.y *= anim->animscale;
			sp138.z *= anim->animscale;

			if (node != model->filedata->rootnode) {
				sp138.x += rodata->pos.x;
				sp138.y += rodata->pos.y;
				sp138.z += rodata->pos.z;
			}

			modelPositionJointUsingVecRot(renderdata, model, node, &sp144, &sp138, false, &sp12c);
		} else if (node != model->filedata->rootnode) {
			modelPositionJointUsingVecRot(renderdata, model, node, &sp144, &rodata->pos, false, &sp12c);
		} else {
			modelPositionJointUsingVecRot(renderdata, model, node, &sp144, &sp138, false, &sp12c);
		}
	} else {
		if (node->parent) {
			mtx = model0001a5cc(model, node->parent, 0);
		} else {
			mtx = renderdata->unk00;
		}

		if (mtx) {
			mtx4LoadTranslation(&rodata->pos, &spe8);
			mtx00015be4(mtx, &spe8, &model->matrices[rodata->mtxindex0]);
		} else {
			mtx4LoadTranslation(&rodata->pos, &model->matrices[rodata->mtxindex0]);
		}
	}
}

void model0001c5b4(struct modelrenderdata *arg0, struct model *model, struct modelnode *node)
{
	union modelrodata *rodata = node->rodata;
	Mtxf *sp68;
	Mtxf sp28;
	s32 mtxindex = rodata->positionheld.mtxindex;
	Mtxf *matrices = model->matrices;

	if (node->parent) {
		sp68 = model0001a5cc(model, node->parent, 0);
	} else {
		sp68 = arg0->unk00;
	}

	if (sp68) {
		mtx4LoadTranslation(&rodata->positionheld.pos, &sp28);
		mtx00015be4(sp68, &sp28, &matrices[mtxindex]);
	} else {
		mtx4LoadTranslation(&rodata->positionheld.pos, &matrices[mtxindex]);
	}
}

/**
 * For a distance node, set its target to visible based on distance.
 */
void model0001c664(struct model *model, struct modelnode *node)
{
	union modelrodata *rodata = node->rodata;
	union modelrwdata *rwdata = modelGetNodeRwData(model, node);
	Mtxf *mtx = model0001a5cc(model, node, 0);
	f32 distance;

	if (g_ModelDistanceDisabled || !mtx) {
		distance = 0;
	} else {
		distance = -mtx->m[3][2] * camGetLodScaleZ();

		if (g_ModelDistanceScale != 1) {
			distance *= g_ModelDistanceScale;
		}
	}

	if (distance > rodata->distance.near * model->scale || rodata->distance.near == 0) {
		if (distance <= rodata->distance.far * model->scale) {
			rwdata->distance.visible = true;
			node->child = rodata->distance.target;
			return;
		}
	}

	rwdata->distance.visible = false;
	node->child = NULL;
}

void model0001c784(struct model *model, struct modelnode *node)
{
	struct modelrodata_distance *rodata = &node->rodata->distance;
	struct modelrwdata_distance *rwdata = modelGetNodeRwData(model, node);

	if (rwdata->visible) {
		node->child = rodata->target;
	} else {
		node->child = NULL;
	}
}

void model0001c7d0(struct model *model, struct modelnode *node)
{
	struct modelrodata_toggle *rodata = &node->rodata->toggle;
	struct modelrwdata_toggle *rwdata = modelGetNodeRwData(model, node);

	if (rwdata->visible) {
		node->child = rodata->target;
	} else {
		node->child = NULL;
	}
}

/**
 * Attach a head model to its placeholder on the body model.
 *
 * The given modelnode is assumed to be of type MODELNODETYPE_HEADSPOT.
 */
void modelAttachHead(struct model *model, struct modelnode *bodynode)
{
	struct modelrwdata_headspot *rwdata = modelGetNodeRwData(model, bodynode);

	if (rwdata->modelfiledata) {
		struct modelnode *headnode = rwdata->modelfiledata->rootnode;

		bodynode->child = headnode;

		while (headnode) {
			headnode->parent = bodynode;
			headnode = headnode->next;
		}
	}
}

void model0001c868(struct modelnode *basenode, bool visible)
{
	union modelrodata *rodata = basenode->rodata;
	struct modelnode *node1;
	struct modelnode *node2;
	struct modelnode *loopnode;

	if (visible) {
		node1 = rodata->reorder.unk18;
		node2 = rodata->reorder.unk1c;
	} else {
		node1 = rodata->reorder.unk1c;
		node2 = rodata->reorder.unk18;
	}

	if (node1) {
		// I think what's happening here is there's two groups of siblings,
		// where node1 and node2 are the head nodes. Either group can be first,
		// and this is ensuring the node1 group is first.
		// Note that node2 might be NULL.

		basenode->child = node1;
		node1->prev = NULL;

		// Skip through node1's siblings until node2 is found or the end is
		// reached
		loopnode = node1;

		while (loopnode->next && loopnode->next != node2) {
			loopnode = loopnode->next;
		}

		loopnode->next = node2;

		if (node2) {
			// Append node2 and its siblings to node1's siblings
			node2->prev = loopnode;
			loopnode = node2;

			while (loopnode->next && loopnode->next != node1) {
				loopnode = loopnode->next;
			}

			loopnode->next = NULL;
		}
	} else {
		basenode->child = node2;

		if (node2) {
			node2->prev = NULL;
		}
	}
}

void modelRenderNodeReorder(struct model *model, struct modelnode *node)
{
	union modelrwdata *rwdata = modelGetNodeRwData(model, node);

	model0001c868(node, rwdata->reorder.visible);
}

void model0001c950(struct model *model, struct modelnode *node)
{
	union modelrodata *rodata = node->rodata;
	union modelrwdata *rwdata = modelGetNodeRwData(model, node);
	Mtxf *mtx = model0001a5cc(model, node, 0);
	struct coord sp38;
	struct coord sp2c;
	f32 tmp;

	if (rodata->reorder.unk20 == 0) {
		sp38.x = rodata->reorder.unk0c[0];
		sp38.y = rodata->reorder.unk0c[1];
		sp38.z = rodata->reorder.unk0c[2];
		mtx4RotateVecInPlace(mtx, &sp38);
	} else if (rodata->reorder.unk20 == 2) {
		sp38.x = mtx->m[1][0] * rodata->reorder.unk0c[1];
		sp38.y = mtx->m[1][1] * rodata->reorder.unk0c[1];
		sp38.z = mtx->m[1][2] * rodata->reorder.unk0c[1];
	} else if (rodata->reorder.unk20 == 3) {
		sp38.x = mtx->m[2][0] * rodata->reorder.unk0c[2];
		sp38.y = mtx->m[2][1] * rodata->reorder.unk0c[2];
		sp38.z = mtx->m[2][2] * rodata->reorder.unk0c[2];
	} else if (rodata->reorder.unk20 == 1) {
		sp38.x = mtx->m[0][0] * rodata->reorder.unk0c[0];
		sp38.y = mtx->m[0][1] * rodata->reorder.unk0c[0];
		sp38.z = mtx->m[0][2] * rodata->reorder.unk0c[0];
	}

	sp2c.x = rodata->reorder.unk00;
	sp2c.y = rodata->reorder.unk04;
	sp2c.z = rodata->reorder.unk08;

	mtx4TransformVecInPlace(mtx, &sp2c);

	tmp = sp38.f[0] * sp2c.f[0] + sp38.f[1] * sp2c.f[1] + sp38.f[2] * sp2c.f[2];

	if (tmp < 0) {
		rwdata->reorder.visible = true;
	} else {
		rwdata->reorder.visible = false;
	}

	modelRenderNodeReorder(model, node);
}

void model0001cb0c(struct model *model, struct modelnode *parent)
{
	struct modelnode *node = parent->child;

	if (parent);

	while (node) {
		s32 type = node->type & 0xff;
		bool dochildren = true;

		switch (type) {
		case MODELNODETYPE_CHRINFO:
		case MODELNODETYPE_POSITION:
		case MODELNODETYPE_0B:
		case MODELNODETYPE_CHRGUNFIRE:
		case MODELNODETYPE_0D:
		case MODELNODETYPE_0E:
		case MODELNODETYPE_0F:
		case MODELNODETYPE_POSITIONHELD:
			dochildren = false;
			break;
		case MODELNODETYPE_DISTANCE:
			model0001c664(model, node);
			break;
		case MODELNODETYPE_REORDER:
			model0001c950(model, node);
			break;
		case MODELNODETYPE_HEADSPOT:
			modelAttachHead(model, node);
			break;
		case MODELNODETYPE_DL:
			break;
		}

		if (dochildren && node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node == parent->parent) {
					node = NULL;
					break;
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}
}

void model0001cc20(struct model *model)
{
	struct modelnode *node = model->filedata->rootnode;

	while (node) {
		u32 type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_DISTANCE:
			model0001c664(model, node);
			break;
		case MODELNODETYPE_REORDER:
			model0001c950(model, node);
			break;
		case MODELNODETYPE_TOGGLE:
			model0001c7d0(model, node);
			break;
		case MODELNODETYPE_HEADSPOT:
			modelAttachHead(model, node);
			break;
		case MODELNODETYPE_CHRINFO:
		case MODELNODETYPE_DL:
		default:
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}
}

void model0001cd18(struct modelrenderdata *arg0, struct model *model)
{
	struct modelnode *node = model->filedata->rootnode;

	while (node) {
		u32 type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_CHRINFO:
			model0001b400(arg0, model, node);
			break;
		case MODELNODETYPE_POSITION:
			model0001bfa8(arg0, model, node);
			break;
		case MODELNODETYPE_POSITIONHELD:
			model0001c5b4(arg0, model, node);
			break;
		case MODELNODETYPE_DISTANCE:
			model0001c664(model, node);
			break;
		case MODELNODETYPE_REORDER:
			model0001c950(model, node);
			break;
		case MODELNODETYPE_TOGGLE:
			model0001c7d0(model, node);
			break;
		case MODELNODETYPE_HEADSPOT:
			modelAttachHead(model, node);
			break;
		case MODELNODETYPE_DL:
		default:
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}
}

void model0001ce64(struct modelrenderdata *arg0, struct model *model)
{
	model->matrices = arg0->unk10;

	arg0->unk10 += model->filedata->nummatrices;

#if VERSION >= VERSION_PAL_BETA
	if (var8005efb0_2 || !model00018680()) {
		model0001cd18(arg0, model);
	}
#else
	if (!model00018680()) {
		model0001cd18(arg0, model);
	}
#endif
}

void model0001cebc(struct modelrenderdata *arg0, struct model *model)
{
	struct anim *anim = model->anim;
	f32 speed;
	f32 frac;
	f32 frac2;

	if (anim && anim->animnum) {
		if (PLAYERCOUNT() >= 2) {
			frac = anim->frac;
			frac2 = anim->frac2;
			speed = anim->speed;

			if (speed < 0) {
				speed = -speed;
			}

			if (speed > 0.5f) {
				anim->frac = 0;
				anim->frac2 = 0;
			}
		}

		anim00023d38(anim->animnum);
		anim->unk04 = anim00023ab0(anim->animnum, anim->framea);

		if (anim->frac != 0) {
			anim->unk05 = anim00023ab0(anim->animnum, anim->frameb);
		}

		if (anim->animnum2) {
			anim00023d38(anim->animnum2);
			anim->unk06 = anim00023ab0(anim->animnum2, anim->frame2a);

			if (anim->frac2 != 0) {
				anim->unk07 = anim00023ab0(anim->animnum2, anim->frame2b);
			}
		}

		anim00023d0c();
	}

	model0001ce64(arg0, model);

	if (PLAYERCOUNT() >= 2 && anim && anim->animnum) {
		anim->frac = frac;
		anim->frac2 = frac2;
	}
}

s16 modelGetAnimNum(struct model *model)
{
	if (model->anim) {
		return model->anim->animnum;
	}

	return 0;
}

bool modelIsFlipped(struct model *model)
{
	if (model->anim) {
		return model->anim->flip;
	}

	return false;
}

f32 modelGetCurAnimFrame(struct model *model)
{
	if (model->anim) {
		return model->anim->frame;
	}

	return 0;
}

f32 modelGetAnimEndFrame(struct model *model)
{
	struct anim *anim = model->anim;

	if (anim) {
		if (anim->endframe >= 0) {
			return anim->endframe;
		}

		if (anim->animnum) {
			return animGetNumFrames(anim->animnum) - 1;
		} else {
			return 0;
		}
	}

	return 0;
}

s32 modelGetNumAnimFrames(struct model *model)
{
	if (model->anim) {
		return animGetNumFrames(modelGetAnimNum(model));
	}

	return 0;
}

f32 modelGetAnimSpeed(struct model *model)
{
	if (model->anim) {
		return model->anim->speed;
	}

	return 1;
}

f32 modelGetAbsAnimSpeed(struct model *model)
{
	f32 speed;

	if (model->anim) {
		speed = model->anim->speed;

		if (speed < 0) {
			speed = -speed;
		}

		return speed;
	}

	return 1;
}

f32 modelGetEffectiveAnimSpeed(struct model *model)
{
	if (model->anim) {
		return modelGetAnimSpeed(model) * model->anim->playspeed;
	}

	return 1;
}

/**
 * Constrain the given frame number to the bounds of the animation, unless the
 * animation is looping in which case wrap it to the other side.
 */
s32 modelConstrainOrWrapAnimFrame(s32 frame, s16 animnum, f32 endframe)
{
	if (frame < 0) {
		if (var8005efbc || (g_Anims[animnum].flags & ANIMFLAG_LOOP)) {
			frame = animGetNumFrames(animnum) - (-frame % animGetNumFrames(animnum));
		} else {
			frame = 0;
		}
	} else if (endframe >= 0 && frame > (s32)endframe) {
		frame = ceil(endframe);
	} else if (frame >= animGetNumFrames(animnum)) {
		if (var8005efbc || (g_Anims[animnum].flags & ANIMFLAG_LOOP)) {
			frame = frame % animGetNumFrames(animnum);
		} else {
			frame = animGetNumFrames(animnum) - 1;
		}
	}

	return frame;
}

void modelCopyAnimForMerge(struct model *model, f32 merge)
{
	struct anim *anim = model->anim;
	struct modelnode *node;
	u32 nodetype;

	if (anim) {
		if (merge > 0 && anim->animnum) {
			if (anim->animnum2 && anim->fracmerge == 1) {
				return;
			}

			node = model->filedata->rootnode;
			nodetype = node->type & 0xff;

			anim->frame2 = anim->frame;
			anim->frac2 = anim->frac;
			anim->animnum2 = anim->animnum;
			anim->flip2 = anim->flip;
			anim->frame2a = anim->framea;
			anim->frame2b = anim->frameb;
			anim->speed2 = anim->speed;
			anim->newspeed2 = anim->newspeed;
			anim->oldspeed2 = anim->oldspeed;
			anim->timespeed2 = anim->timespeed;
			anim->elapsespeed2 = anim->elapsespeed;
			anim->endframe2 = anim->endframe;

			if (nodetype == MODELNODETYPE_CHRINFO) {
				struct modelrwdata_chrinfo *rwdata = modelGetNodeRwData(model, node);
				rwdata->unk02 = 1;
				rwdata->unk4c.x = rwdata->unk34.x;
				rwdata->unk4c.y = rwdata->unk34.y;
				rwdata->unk4c.z = rwdata->unk34.z;
				rwdata->unk40.x = rwdata->unk24.x;
				rwdata->unk40.y = rwdata->unk24.y;
				rwdata->unk40.z = rwdata->unk24.z;
			}
		} else {
			anim->animnum2 = 0;
		}
	}
}

void model0001d62c(struct model *model, s16 animnum, s32 flip, f32 fstartframe, f32 speed, f32 merge)
{
	struct anim *anim = model->anim;

	if (anim) {
		s32 isfirstanim = !anim->animnum;
		s32 type;

		if (anim->animnum2) {
			anim->timemerge = merge;
			anim->elapsemerge = 0;
			anim->fracmerge = 1;
		} else {
			anim->timemerge = 0;
			anim->fracmerge = 0;
		}

		anim->animnum = animnum;
		anim->flip = flip;
		anim->endframe = -1;
		anim->speed = speed;
		anim->timespeed = 0;

		model0001e018(model, fstartframe);

		anim->looping = false;

		type = model->filedata->rootnode->type & 0xff;

		if (type == MODELNODETYPE_CHRINFO) {
			u32 stack;
			struct modelrodata_chrinfo *rodata = &model->filedata->rootnode->rodata->chrinfo;
			struct modelrwdata_chrinfo *rwdata = (struct modelrwdata_chrinfo *) modelGetNodeRwData(model, model->filedata->rootnode);
			s32 spa4 = rodata->unk00;
			struct skeleton *skel = model->filedata->skel;
			f32 scale;
			f32 sp98;
			f32 sp94;
			struct coord sp88 = {0, 0, 0};
			f32 sp84;
			u8 sp83;
			struct coord sp74;
			struct coord sp68;
			f32 sp64;
			struct coord sp58;
			struct coord sp4c;
			f32 angle;
			f32 y;
			f32 x;
			f32 z;

			if (g_Anims[anim->animnum].flags & ANIMFLAG_02) {
				sp64 = func0f15c888();
				anim00023d38(anim->animnum);
				sp83 = anim00023ab0(anim->animnum, anim->framea);
				anim00023d0c();
				anim00024050(spa4, anim->flip, skel, anim->animnum, sp83, &sp74, &sp88, &sp68);

				rwdata->unk34.x = sp88.x * sp64;
				rwdata->unk34.y = sp88.y * sp64;
				rwdata->unk34.z = sp88.z * sp64;
				rwdata->unk30 = rwdata->unk14;

				if (anim->frac == 0) {
					rwdata->unk01 = 0;
				} else {
					anim00023d38(anim->animnum);
					sp83 = anim00023ab0(anim->animnum, anim->frameb);
					anim00023d0c();
					anim00024050(spa4, anim->flip, skel, anim->animnum, sp83, &sp74, &sp88, &sp68);

					rwdata->unk24.x = sp88.x * sp64;
					rwdata->unk24.y = sp88.y * sp64;
					rwdata->unk24.z = sp88.z * sp64;
					rwdata->unk20 = rwdata->unk14;

					rwdata->unk01 = 1;
				}
			} else {
				sp84 = anim00024b64(spa4, anim->flip, skel, anim->animnum, anim->frameb, &sp88, anim->average);
				scale = model->scale * anim->animscale;

				if (scale != 1) {
					sp88.x *= scale;
					sp88.y *= scale;
					sp88.z *= scale;
				}

				if (anim->average) {
					sp88.y = rwdata->pos.y - rwdata->ground;
				}

				sp98 = cosf(rwdata->unk14);
				sp94 = sinf(rwdata->unk14);

				if (anim->frac == 0) {
					rwdata->unk34.x = rwdata->pos.f[0];
					rwdata->unk34.y = rwdata->pos.f[1] - rwdata->ground;
					rwdata->unk34.z = rwdata->pos.f[2];

					rwdata->unk30 = rwdata->unk14;

					sp58.x = rwdata->unk34.f[0] + sp88.f[0] * sp98 + sp88.f[2] * sp94;
					sp58.y = sp88.f[1];
					sp58.z = rwdata->unk34.f[2] - sp88.f[0] * sp94 + sp88.f[2] * sp98;

					rwdata->unk24.x = sp58.f[0];
					rwdata->unk24.y = sp58.f[1];
					rwdata->unk24.z = sp58.f[2];

					if (rwdata->unk18 == 0) {
						rwdata->unk20 = rwdata->unk30 + sp84;

						if (rwdata->unk20 >= M_BADTAU) {
							rwdata->unk20 -= M_BADTAU;
						}
					}

					rwdata->unk01 = 1;
				} else {
					x = sp88.f[0] * sp98 + sp88.f[2] * sp94;
					y = sp88.f[1];
					z = -sp88.f[0] * sp94 + sp88.f[2] * sp98;

					sp4c.f[0] = rwdata->pos.f[0] + x * (1 - anim->frac);
					sp4c.f[1] = y;
					sp4c.f[2] = rwdata->pos.f[2] + z * (1 - anim->frac);

					rwdata->unk24.f[0] = sp4c.f[0];
					rwdata->unk24.f[1] = sp4c.f[1];
					rwdata->unk24.f[2] = sp4c.f[2];

					rwdata->unk34.f[0] = rwdata->unk24.f[0] - x;
					rwdata->unk34.f[1] = (rwdata->pos.f[1] - rwdata->ground) - (y - (rwdata->pos.f[1] - rwdata->ground)) * anim->frac / (1 - anim->frac);
					rwdata->unk34.f[2] = rwdata->unk24.f[2] - z;

					angle = rwdata->unk14 - sp84;

					if (angle < 0) {
						angle += M_BADTAU;
					}

					rwdata->unk30 = model0001afe8(rwdata->unk14, angle, anim->frac);

					if (rwdata->unk18 == 0) {
						rwdata->unk20 = rwdata->unk30 + sp84;

						if (rwdata->unk20 >= M_BADTAU) {
							rwdata->unk20 -= M_BADTAU;
						}
					}

					rwdata->unk01 = 1;
				}

				if (isfirstanim) {
					rwdata->unk34.f[1] = rwdata->unk24.f[1];
				}
			}
		}
	}
}

bool modelIsAnimMerging(struct model *model)
{
	if (model && model->anim && model->anim->animnum2
			&& model->anim->fracmerge != 0 && model->anim->fracmerge != 1) {
		return true;
	}

	return false;
}

void modelSetAnimationWithMerge(struct model *model, s16 animnum, u32 flip, f32 startframe, f32 speed, f32 timemerge, bool domerge)
{
	if (model) {
		if (model->anim && model->anim->animnum
				&& (g_Anims[model->anim->animnum].flags & ANIMFLAG_02)
				&& (g_Anims[animnum].flags & ANIMFLAG_02) == 0) {
			timemerge = 0;
		}

		if (domerge) {
			modelCopyAnimForMerge(model, timemerge);
		}

		model0001d62c(model, animnum, flip, startframe, speed, timemerge);
	}
}

void modelSetAnimation(struct model *model, s16 animnum, s32 flip, f32 startframe, f32 speed, f32 merge)
{
	if (model) {
		if (model->anim && model->anim->animnum
				&& (g_Anims[model->anim->animnum].flags & ANIMFLAG_02)
				&& (g_Anims[animnum].flags & ANIMFLAG_02) == 0) {
			merge = 0;
		}

		modelCopyAnimForMerge(model, merge);
		model0001d62c(model, animnum, flip, startframe, speed, merge);
	}
}

void modelCopyAnimData(struct model *src, struct model *dst)
{
	if (src->anim && dst->anim) {
		*dst->anim = *src->anim;
	}
}

void modelSetAnimLooping(struct model *model, f32 loopframe, f32 loopmerge)
{
	if (model->anim) {
		model->anim->looping = true;
		model->anim->loopframe = loopframe;
		model->anim->loopmerge = loopmerge;
	}
}

void modelSetAnimEndFrame(struct model *model, f32 endframe)
{
	struct anim *anim = model->anim;

	if (anim) {
		if (anim->animnum && endframe < animGetNumFrames(anim->animnum) - 1) {
			anim->endframe = endframe;
		} else {
			anim->endframe = -1;
		}
	}
}

void modelSetAnimFlipFunction(struct model *model, void *callback)
{
	if (model->anim) {
		model->anim->flipfunc = callback;
	}
}

#if VERSION < VERSION_NTSC_1_0
void modelSetAnimUnk6c(struct model *model, s32 value)
{
	if (model->anim) {
		model->anim->unk6c = value;
	}
}
#endif

void modelSetAnimSpeed(struct model *model, f32 speed, f32 startframe)
{
	struct anim *anim = model->anim;

	if (anim) {
		if (startframe > 0) {
			anim->timespeed = startframe;
			anim->newspeed = speed;
			anim->elapsespeed = 0;
			anim->oldspeed = anim->speed;
		} else {
			anim->speed = speed;
			anim->timespeed = 0;
		}
	}
}

void modelSetAnimSpeedAuto(struct model *model, f32 arg1, f32 startframe)
{
	struct anim *anim = model->anim;
	f32 tmp;
	f32 speed;

	if (anim) {
		if (anim->frame <= arg1) {
			tmp = arg1 - anim->frame;
		} else {
			tmp = animGetNumFrames(anim->animnum) - anim->frame + arg1;
		}

		speed = anim->speed + (tmp + tmp) / startframe;

		modelSetAnimSpeed(model, speed, startframe);
	}
}

void modelSetAnimPlaySpeed(struct model *model, f32 speed, f32 startframe)
{
	struct anim *anim = model->anim;

	if (anim) {
		if (startframe > 0) {
			anim->timeplay = startframe;
			anim->newplay = speed;
			anim->elapseplay = 0;
			anim->oldplay = anim->playspeed;
		} else {
			anim->playspeed = speed;
			anim->timeplay = 0;
		}
	}
}

void modelSetAnim70(struct model *model, void *callback)
{
	if (model->anim) {
		model->anim->unk70 = callback;
	}
}

void model0001e018(struct model *model, f32 arg1)
{
	s32 sp28;
	s32 sp24;
	bool sp20;
	struct anim *anim = model->anim;

	if (anim) {
		sp28 = floor(arg1);
		sp20 = anim->speed >= 0;

		sp24 = (sp20 ? sp28 + 1 : sp28 - 1);

		anim->framea = modelConstrainOrWrapAnimFrame(sp28, anim->animnum, anim->endframe);
		anim->frameb = modelConstrainOrWrapAnimFrame(sp24, anim->animnum, anim->endframe);

		if (anim->framea == anim->frameb) {
			anim->frac = 0;
			anim->frame = anim->framea;
		} else if (sp20) {
			anim->frac = arg1 - sp28;
			anim->frame = anim->framea + anim->frac;
		} else {
			anim->frac = 1 - (arg1 - sp24);
			anim->frame = anim->frameb + (1 - anim->frac);
		}
	}
}

void model0001e14c(struct model *model, f32 arg1, f32 arg2)
{
	struct anim *anim = model->anim;

	if (anim) {
		model0001e018(model, arg1);

		if (anim->animnum2) {
			s32 sp28 = floor(arg2);
			s32 sp24;
			bool sp20 = anim->speed2 >= 0;

			sp24 = (sp20 ? sp28 + 1 : sp28 - 1);

			anim->frame2a = modelConstrainOrWrapAnimFrame(sp28, anim->animnum2, anim->endframe2);
			anim->frame2b = modelConstrainOrWrapAnimFrame(sp24, anim->animnum2, anim->endframe2);

			if (anim->frame2a == anim->frame2b) {
				anim->frac2 = 0;
				anim->frame2 = anim->frame2a;
			} else if (sp20) {
				anim->frac2 = arg2 - sp28;
				anim->frame2 = anim->frame2a + anim->frac2;
			} else {
				anim->frac2 = 1 - (arg2 - sp24);
				anim->frame2 = anim->frame2b + (1 - anim->frac2);
			}
		}
	}
}

bool var8005efdc = true;
u32 var8005efe0 = 0x00000000;
u32 var8005efe4 = 0x00000000;
u32 var8005efe8 = 0x00000000;
u32 var8005efec = 0x00000000;
u32 var8005eff0 = 0x00000000;
u32 var8005eff4 = 0x00000000;
u32 var8005eff8 = 0xffffffff;

void model0001e29c(bool value)
{
	var8005efdc = value;
}

bool model0001e2a8(void)
{
	return var8005efdc;
}

GLOBAL_ASM(
glabel model0001e2b4
.late_rodata
glabel var70054450
.word 0x40c907a9
.text
/*    1e2b4:	27bdfed0 */ 	addiu	$sp,$sp,-304
/*    1e2b8:	afbf006c */ 	sw	$ra,0x6c($sp)
/*    1e2bc:	afb40068 */ 	sw	$s4,0x68($sp)
/*    1e2c0:	afb30064 */ 	sw	$s3,0x64($sp)
/*    1e2c4:	afb20060 */ 	sw	$s2,0x60($sp)
/*    1e2c8:	afb1005c */ 	sw	$s1,0x5c($sp)
/*    1e2cc:	afb00058 */ 	sw	$s0,0x58($sp)
/*    1e2d0:	f7be0050 */ 	sdc1	$f30,0x50($sp)
/*    1e2d4:	f7bc0048 */ 	sdc1	$f28,0x48($sp)
/*    1e2d8:	f7ba0040 */ 	sdc1	$f26,0x40($sp)
/*    1e2dc:	f7b80038 */ 	sdc1	$f24,0x38($sp)
/*    1e2e0:	f7b60030 */ 	sdc1	$f22,0x30($sp)
/*    1e2e4:	f7b40028 */ 	sdc1	$f20,0x28($sp)
/*    1e2e8:	afa60138 */ 	sw	$a2,0x138($sp)
/*    1e2ec:	afa7013c */ 	sw	$a3,0x13c($sp)
/*    1e2f0:	8c910020 */ 	lw	$s1,0x20($a0)
/*    1e2f4:	4485a000 */ 	mtc1	$a1,$f20
/*    1e2f8:	00808025 */ 	or	$s0,$a0,$zero
/*    1e2fc:	522002b9 */ 	beqzl	$s1,.L0001ede4
/*    1e300:	8fbf006c */ 	lw	$ra,0x6c($sp)
/*    1e304:	8c8e0008 */ 	lw	$t6,0x8($a0)
/*    1e308:	24010001 */ 	addiu	$at,$zero,0x1
/*    1e30c:	8dc50000 */ 	lw	$a1,0x0($t6)
/*    1e310:	94a20000 */ 	lhu	$v0,0x0($a1)
/*    1e314:	304f00ff */ 	andi	$t7,$v0,0xff
/*    1e318:	55e102ac */ 	bnel	$t7,$at,.L0001edcc
/*    1e31c:	c7b40140 */ 	lwc1	$f20,0x140($sp)
/*    1e320:	0c006a87 */ 	jal	modelGetNodeRwData
/*    1e324:	8cb30004 */ 	lw	$s3,0x4($a1)
/*    1e328:	80580000 */ 	lb	$t8,0x0($v0)
/*    1e32c:	00409025 */ 	or	$s2,$v0,$zero
/*    1e330:	5700029e */ 	bnezl	$t8,.L0001edac
/*    1e334:	c7b40140 */ 	lwc1	$f20,0x140($sp)
/*    1e338:	96790000 */ 	lhu	$t9,0x0($s3)
/*    1e33c:	3c0b8006 */ 	lui	$t3,%hi(var8005efe0)
/*    1e340:	256befe0 */ 	addiu	$t3,$t3,%lo(var8005efe0)
/*    1e344:	afb90118 */ 	sw	$t9,0x118($sp)
/*    1e348:	8e080008 */ 	lw	$t0,0x8($s0)
/*    1e34c:	27aa00fc */ 	addiu	$t2,$sp,0xfc
/*    1e350:	4480d000 */ 	mtc1	$zero,$f26
/*    1e354:	8d090004 */ 	lw	$t1,0x4($t0)
/*    1e358:	afa90114 */ 	sw	$t1,0x114($sp)
/*    1e35c:	c6260088 */ 	lwc1	$f6,0x88($s1)
/*    1e360:	c6040014 */ 	lwc1	$f4,0x14($s0)
/*    1e364:	46062202 */ 	mul.s	$f8,$f4,$f6
/*    1e368:	e7a80110 */ 	swc1	$f8,0x110($sp)
/*    1e36c:	8d610000 */ 	lw	$at,0x0($t3)
/*    1e370:	8d6e0004 */ 	lw	$t6,0x4($t3)
/*    1e374:	ad410000 */ 	sw	$at,0x0($t2)
/*    1e378:	8d610008 */ 	lw	$at,0x8($t3)
/*    1e37c:	ad4e0004 */ 	sw	$t6,0x4($t2)
/*    1e380:	ad410008 */ 	sw	$at,0x8($t2)
/*    1e384:	c44a0034 */ 	lwc1	$f10,0x34($v0)
/*    1e388:	e7aa00e0 */ 	swc1	$f10,0xe0($sp)
/*    1e38c:	c4500038 */ 	lwc1	$f16,0x38($v0)
/*    1e390:	e7b000e4 */ 	swc1	$f16,0xe4($sp)
/*    1e394:	c452003c */ 	lwc1	$f18,0x3c($v0)
/*    1e398:	c7b00138 */ 	lwc1	$f16,0x138($sp)
/*    1e39c:	e7b200e8 */ 	swc1	$f18,0xe8($sp)
/*    1e3a0:	c4440024 */ 	lwc1	$f4,0x24($v0)
/*    1e3a4:	c45e0030 */ 	lwc1	$f30,0x30($v0)
/*    1e3a8:	e7a400d0 */ 	swc1	$f4,0xd0($sp)
/*    1e3ac:	c4460028 */ 	lwc1	$f6,0x28($v0)
/*    1e3b0:	e7a600d4 */ 	swc1	$f6,0xd4($sp)
/*    1e3b4:	c448002c */ 	lwc1	$f8,0x2c($v0)
/*    1e3b8:	e7a800d8 */ 	swc1	$f8,0xd8($sp)
/*    1e3bc:	c44a0020 */ 	lwc1	$f10,0x20($v0)
/*    1e3c0:	e7aa00cc */ 	swc1	$f10,0xcc($sp)
/*    1e3c4:	804f0001 */ 	lb	$t7,0x1($v0)
/*    1e3c8:	afaf00c8 */ 	sw	$t7,0xc8($sp)
/*    1e3cc:	c63c001c */ 	lwc1	$f28,0x1c($s1)
/*    1e3d0:	461ae03c */ 	c.lt.s	$f28,$f26
/*    1e3d4:	00000000 */ 	nop
/*    1e3d8:	45020003 */ 	bc1fl	.L0001e3e8
/*    1e3dc:	c6200040 */ 	lwc1	$f0,0x40($s1)
/*    1e3e0:	4600e707 */ 	neg.s	$f28,$f28
/*    1e3e4:	c6200040 */ 	lwc1	$f0,0x40($s1)
.L0001e3e8:
/*    1e3e8:	461a003c */ 	c.lt.s	$f0,$f26
/*    1e3ec:	00000000 */ 	nop
/*    1e3f0:	45020003 */ 	bc1fl	.L0001e400
/*    1e3f4:	4610a03e */ 	c.le.s	$f20,$f16
/*    1e3f8:	46000007 */ 	neg.s	$f0,$f0
/*    1e3fc:	4610a03e */ 	c.le.s	$f20,$f16
.L0001e400:
/*    1e400:	0000a025 */ 	or	$s4,$zero,$zero
/*    1e404:	45000002 */ 	bc1f	.L0001e410
/*    1e408:	00000000 */ 	nop
/*    1e40c:	24140001 */ 	addiu	$s4,$zero,0x1
.L0001e410:
/*    1e410:	5280000a */ 	beqzl	$s4,.L0001e43c
/*    1e414:	4600a306 */ 	mov.s	$f12,$f20
/*    1e418:	4600a306 */ 	mov.s	$f12,$f20
/*    1e41c:	0fc25e42 */ 	jal	floor
/*    1e420:	e7a000bc */ 	swc1	$f0,0xbc($sp)
/*    1e424:	24530001 */ 	addiu	$s3,$v0,0x1
/*    1e428:	0fc25e42 */ 	jal	floor
/*    1e42c:	c7ac0138 */ 	lwc1	$f12,0x138($sp)
/*    1e430:	10000009 */ 	b	.L0001e458
/*    1e434:	00403825 */ 	or	$a3,$v0,$zero
/*    1e438:	4600a306 */ 	mov.s	$f12,$f20
.L0001e43c:
/*    1e43c:	0fc25e7a */ 	jal	ceil
/*    1e440:	e7a000bc */ 	swc1	$f0,0xbc($sp)
/*    1e444:	2453ffff */ 	addiu	$s3,$v0,-1
/*    1e448:	0fc25e7a */ 	jal	ceil
/*    1e44c:	c7ac0138 */ 	lwc1	$f12,0x138($sp)
/*    1e450:	afa200ec */ 	sw	$v0,0xec($sp)
/*    1e454:	00403825 */ 	or	$a3,$v0,$zero
.L0001e458:
/*    1e458:	86390000 */ 	lh	$t9,0x0($s1)
/*    1e45c:	3c188006 */ 	lui	$t8,%hi(g_Anims)
/*    1e460:	8f18f00c */ 	lw	$t8,%lo(g_Anims)($t8)
/*    1e464:	00194080 */ 	sll	$t0,$t9,0x2
/*    1e468:	01194023 */ 	subu	$t0,$t0,$t9
/*    1e46c:	00084080 */ 	sll	$t0,$t0,0x2
/*    1e470:	03084821 */ 	addu	$t1,$t8,$t0
/*    1e474:	912d000b */ 	lbu	$t5,0xb($t1)
/*    1e478:	3c017005 */ 	lui	$at,%hi(var70054450)
/*    1e47c:	31ac0002 */ 	andi	$t4,$t5,0x2
/*    1e480:	11800067 */ 	beqz	$t4,.L0001e620
/*    1e484:	00000000 */ 	nop
/*    1e488:	0fc57222 */ 	jal	func0f15c888
/*    1e48c:	afa700ec */ 	sw	$a3,0xec($sp)
/*    1e490:	8faa00ec */ 	lw	$t2,0xec($sp)
/*    1e494:	862b0014 */ 	lh	$t3,0x14($s1)
/*    1e498:	46000506 */ 	mov.s	$f20,$f0
/*    1e49c:	01402025 */ 	or	$a0,$t2,$zero
/*    1e4a0:	114b005d */ 	beq	$t2,$t3,.L0001e618
/*    1e4a4:	00000000 */ 	nop
/*    1e4a8:	86250000 */ 	lh	$a1,0x0($s1)
/*    1e4ac:	0c0074c8 */ 	jal	modelConstrainOrWrapAnimFrame
/*    1e4b0:	8e260018 */ 	lw	$a2,0x18($s1)
/*    1e4b4:	a6220014 */ 	sh	$v0,0x14($s1)
/*    1e4b8:	8fae00c8 */ 	lw	$t6,0xc8($sp)
/*    1e4bc:	00408025 */ 	or	$s0,$v0,$zero
/*    1e4c0:	8faf00ec */ 	lw	$t7,0xec($sp)
/*    1e4c4:	11c0000b */ 	beqz	$t6,.L0001e4f4
/*    1e4c8:	00000000 */ 	nop
/*    1e4cc:	86390016 */ 	lh	$t9,0x16($s1)
/*    1e4d0:	c7b200d0 */ 	lwc1	$f18,0xd0($sp)
/*    1e4d4:	15f90007 */ 	bne	$t7,$t9,.L0001e4f4
/*    1e4d8:	00000000 */ 	nop
/*    1e4dc:	c7a400d4 */ 	lwc1	$f4,0xd4($sp)
/*    1e4e0:	c7a600d8 */ 	lwc1	$f6,0xd8($sp)
/*    1e4e4:	e7b200e0 */ 	swc1	$f18,0xe0($sp)
/*    1e4e8:	e7a400e4 */ 	swc1	$f4,0xe4($sp)
/*    1e4ec:	1000001f */ 	b	.L0001e56c
/*    1e4f0:	e7a600e8 */ 	swc1	$f6,0xe8($sp)
.L0001e4f4:
/*    1e4f4:	0c008f4e */ 	jal	anim00023d38
/*    1e4f8:	86240000 */ 	lh	$a0,0x0($s1)
/*    1e4fc:	86240000 */ 	lh	$a0,0x0($s1)
/*    1e500:	0c008eac */ 	jal	anim00023ab0
/*    1e504:	02002825 */ 	or	$a1,$s0,$zero
/*    1e508:	0c008f43 */ 	jal	anim00023d0c
/*    1e50c:	305000ff */ 	andi	$s0,$v0,0xff
/*    1e510:	82250008 */ 	lb	$a1,0x8($s1)
/*    1e514:	86270000 */ 	lh	$a3,0x0($s1)
/*    1e518:	27b800a8 */ 	addiu	$t8,$sp,0xa8
/*    1e51c:	27a800fc */ 	addiu	$t0,$sp,0xfc
/*    1e520:	27a9009c */ 	addiu	$t1,$sp,0x9c
/*    1e524:	afa9001c */ 	sw	$t1,0x1c($sp)
/*    1e528:	afa80018 */ 	sw	$t0,0x18($sp)
/*    1e52c:	afb80014 */ 	sw	$t8,0x14($sp)
/*    1e530:	afb00010 */ 	sw	$s0,0x10($sp)
/*    1e534:	8fa40118 */ 	lw	$a0,0x118($sp)
/*    1e538:	0c009014 */ 	jal	anim00024050
/*    1e53c:	8fa60114 */ 	lw	$a2,0x114($sp)
/*    1e540:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e544:	c7aa0100 */ 	lwc1	$f10,0x100($sp)
/*    1e548:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e54c:	46141202 */ 	mul.s	$f8,$f2,$f20
/*    1e550:	00000000 */ 	nop
/*    1e554:	46145402 */ 	mul.s	$f16,$f10,$f20
/*    1e558:	00000000 */ 	nop
/*    1e55c:	46146482 */ 	mul.s	$f18,$f12,$f20
/*    1e560:	e7a800e0 */ 	swc1	$f8,0xe0($sp)
/*    1e564:	e7b000e4 */ 	swc1	$f16,0xe4($sp)
/*    1e568:	e7b200e8 */ 	swc1	$f18,0xe8($sp)
.L0001e56c:
/*    1e56c:	12800004 */ 	beqz	$s4,.L0001e580
/*    1e570:	8fb300ec */ 	lw	$s3,0xec($sp)
/*    1e574:	8fb300ec */ 	lw	$s3,0xec($sp)
/*    1e578:	10000002 */ 	b	.L0001e584
/*    1e57c:	26730001 */ 	addiu	$s3,$s3,0x1
.L0001e580:
/*    1e580:	2673ffff */ 	addiu	$s3,$s3,-1
.L0001e584:
/*    1e584:	02602025 */ 	or	$a0,$s3,$zero
/*    1e588:	86250000 */ 	lh	$a1,0x0($s1)
/*    1e58c:	0c0074c8 */ 	jal	modelConstrainOrWrapAnimFrame
/*    1e590:	8e260018 */ 	lw	$a2,0x18($s1)
/*    1e594:	00408025 */ 	or	$s0,$v0,$zero
/*    1e598:	a6220016 */ 	sh	$v0,0x16($s1)
/*    1e59c:	0c008f4e */ 	jal	anim00023d38
/*    1e5a0:	86240000 */ 	lh	$a0,0x0($s1)
/*    1e5a4:	86240000 */ 	lh	$a0,0x0($s1)
/*    1e5a8:	0c008eac */ 	jal	anim00023ab0
/*    1e5ac:	02002825 */ 	or	$a1,$s0,$zero
/*    1e5b0:	0c008f43 */ 	jal	anim00023d0c
/*    1e5b4:	305000ff */ 	andi	$s0,$v0,0xff
/*    1e5b8:	82250008 */ 	lb	$a1,0x8($s1)
/*    1e5bc:	86270000 */ 	lh	$a3,0x0($s1)
/*    1e5c0:	27ad00a8 */ 	addiu	$t5,$sp,0xa8
/*    1e5c4:	27ac00fc */ 	addiu	$t4,$sp,0xfc
/*    1e5c8:	27ab009c */ 	addiu	$t3,$sp,0x9c
/*    1e5cc:	afab001c */ 	sw	$t3,0x1c($sp)
/*    1e5d0:	afac0018 */ 	sw	$t4,0x18($sp)
/*    1e5d4:	afad0014 */ 	sw	$t5,0x14($sp)
/*    1e5d8:	afb00010 */ 	sw	$s0,0x10($sp)
/*    1e5dc:	8fa40118 */ 	lw	$a0,0x118($sp)
/*    1e5e0:	0c009014 */ 	jal	anim00024050
/*    1e5e4:	8fa60114 */ 	lw	$a2,0x114($sp)
/*    1e5e8:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e5ec:	c7a60100 */ 	lwc1	$f6,0x100($sp)
/*    1e5f0:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e5f4:	46141102 */ 	mul.s	$f4,$f2,$f20
/*    1e5f8:	240a0001 */ 	addiu	$t2,$zero,0x1
/*    1e5fc:	afaa00c8 */ 	sw	$t2,0xc8($sp)
/*    1e600:	46143202 */ 	mul.s	$f8,$f6,$f20
/*    1e604:	00000000 */ 	nop
/*    1e608:	46146282 */ 	mul.s	$f10,$f12,$f20
/*    1e60c:	e7a400d0 */ 	swc1	$f4,0xd0($sp)
/*    1e610:	e7a800d4 */ 	swc1	$f8,0xd4($sp)
/*    1e614:	e7aa00d8 */ 	swc1	$f10,0xd8($sp)
.L0001e618:
/*    1e618:	10000147 */ 	b	.L0001eb38
/*    1e61c:	8fa700ec */ 	lw	$a3,0xec($sp)
.L0001e620:
/*    1e620:	c4384450 */ 	lwc1	$f24,%lo(var70054450)($at)
.L0001e624:
/*    1e624:	12800006 */ 	beqz	$s4,.L0001e640
/*    1e628:	0267082a */ 	slt	$at,$s3,$a3
/*    1e62c:	00f3082a */ 	slt	$at,$a3,$s3
/*    1e630:	50200006 */ 	beqzl	$at,.L0001e64c
/*    1e634:	86250000 */ 	lh	$a1,0x0($s1)
/*    1e638:	10000140 */ 	b	.L0001eb3c
/*    1e63c:	c7a400e0 */ 	lwc1	$f4,0xe0($sp)
.L0001e640:
/*    1e640:	5420013e */ 	bnezl	$at,.L0001eb3c
/*    1e644:	c7a400e0 */ 	lwc1	$f4,0xe0($sp)
/*    1e648:	86250000 */ 	lh	$a1,0x0($s1)
.L0001e64c:
/*    1e64c:	8e260018 */ 	lw	$a2,0x18($s1)
/*    1e650:	afa700ec */ 	sw	$a3,0xec($sp)
/*    1e654:	0c0074c8 */ 	jal	modelConstrainOrWrapAnimFrame
/*    1e658:	02602025 */ 	or	$a0,$s3,$zero
/*    1e65c:	a6220014 */ 	sh	$v0,0x14($s1)
/*    1e660:	8fae00c8 */ 	lw	$t6,0xc8($sp)
/*    1e664:	00408025 */ 	or	$s0,$v0,$zero
/*    1e668:	8fa40118 */ 	lw	$a0,0x118($sp)
/*    1e66c:	11c0000e */ 	beqz	$t6,.L0001e6a8
/*    1e670:	27af00fc */ 	addiu	$t7,$sp,0xfc
/*    1e674:	c7b000d0 */ 	lwc1	$f16,0xd0($sp)
/*    1e678:	c7b200d4 */ 	lwc1	$f18,0xd4($sp)
/*    1e67c:	c7a400d8 */ 	lwc1	$f4,0xd8($sp)
/*    1e680:	e7b000e0 */ 	swc1	$f16,0xe0($sp)
/*    1e684:	e7b200e4 */ 	swc1	$f18,0xe4($sp)
/*    1e688:	e7a400e8 */ 	swc1	$f4,0xe8($sp)
/*    1e68c:	c6460018 */ 	lwc1	$f6,0x18($s2)
/*    1e690:	4606d032 */ 	c.eq.s	$f26,$f6
/*    1e694:	00000000 */ 	nop
/*    1e698:	4500004f */ 	bc1f	.L0001e7d8
/*    1e69c:	00000000 */ 	nop
/*    1e6a0:	1000004d */ 	b	.L0001e7d8
/*    1e6a4:	c7be00cc */ 	lwc1	$f30,0xcc($sp)
.L0001e6a8:
/*    1e6a8:	82250008 */ 	lb	$a1,0x8($s1)
/*    1e6ac:	86270000 */ 	lh	$a3,0x0($s1)
/*    1e6b0:	afaf0014 */ 	sw	$t7,0x14($sp)
/*    1e6b4:	afb00010 */ 	sw	$s0,0x10($sp)
/*    1e6b8:	8239000b */ 	lb	$t9,0xb($s1)
/*    1e6bc:	8fa60114 */ 	lw	$a2,0x114($sp)
/*    1e6c0:	0c0092d9 */ 	jal	anim00024b64
/*    1e6c4:	afb90018 */ 	sw	$t9,0x18($sp)
/*    1e6c8:	3c013f80 */ 	lui	$at,0x3f80
/*    1e6cc:	44815000 */ 	mtc1	$at,$f10
/*    1e6d0:	c7a80110 */ 	lwc1	$f8,0x110($sp)
/*    1e6d4:	46000586 */ 	mov.s	$f22,$f0
/*    1e6d8:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e6dc:	460a4032 */ 	c.eq.s	$f8,$f10
/*    1e6e0:	c7b00100 */ 	lwc1	$f16,0x100($sp)
/*    1e6e4:	4503000a */ 	bc1tl	.L0001e710
/*    1e6e8:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e6ec:	46081082 */ 	mul.s	$f2,$f2,$f8
/*    1e6f0:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e6f4:	46088482 */ 	mul.s	$f18,$f16,$f8
/*    1e6f8:	00000000 */ 	nop
/*    1e6fc:	46086302 */ 	mul.s	$f12,$f12,$f8
/*    1e700:	e7a200fc */ 	swc1	$f2,0xfc($sp)
/*    1e704:	e7b20100 */ 	swc1	$f18,0x100($sp)
/*    1e708:	e7ac0104 */ 	swc1	$f12,0x104($sp)
/*    1e70c:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
.L0001e710:
/*    1e710:	16800008 */ 	bnez	$s4,.L0001e734
/*    1e714:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e718:	4600d03c */ 	c.lt.s	$f26,$f0
/*    1e71c:	46001087 */ 	neg.s	$f2,$f2
/*    1e720:	46006307 */ 	neg.s	$f12,$f12
/*    1e724:	e7a200fc */ 	swc1	$f2,0xfc($sp)
/*    1e728:	45000002 */ 	bc1f	.L0001e734
/*    1e72c:	e7ac0104 */ 	swc1	$f12,0x104($sp)
/*    1e730:	4600c581 */ 	sub.s	$f22,$f24,$f0
.L0001e734:
/*    1e734:	8238000b */ 	lb	$t8,0xb($s1)
/*    1e738:	13000005 */ 	beqz	$t8,.L0001e750
/*    1e73c:	00000000 */ 	nop
/*    1e740:	c644000c */ 	lwc1	$f4,0xc($s2)
/*    1e744:	c6460004 */ 	lwc1	$f6,0x4($s2)
/*    1e748:	46062281 */ 	sub.s	$f10,$f4,$f6
/*    1e74c:	e7aa0100 */ 	swc1	$f10,0x100($sp)
.L0001e750:
/*    1e750:	0c0068f4 */ 	jal	cosf
/*    1e754:	c64c0014 */ 	lwc1	$f12,0x14($s2)
/*    1e758:	46000506 */ 	mov.s	$f20,$f0
/*    1e75c:	0c0068f7 */ 	jal	sinf
/*    1e760:	c64c0014 */ 	lwc1	$f12,0x14($s2)
/*    1e764:	c7b000fc */ 	lwc1	$f16,0xfc($sp)
/*    1e768:	c7a80104 */ 	lwc1	$f8,0x104($sp)
/*    1e76c:	c7aa00e0 */ 	lwc1	$f10,0xe0($sp)
/*    1e770:	46148482 */ 	mul.s	$f18,$f16,$f20
/*    1e774:	00000000 */ 	nop
/*    1e778:	46004102 */ 	mul.s	$f4,$f8,$f0
/*    1e77c:	46049180 */ 	add.s	$f6,$f18,$f4
/*    1e780:	c7a40100 */ 	lwc1	$f4,0x100($sp)
/*    1e784:	46065480 */ 	add.s	$f18,$f10,$f6
/*    1e788:	e7a400e4 */ 	swc1	$f4,0xe4($sp)
/*    1e78c:	46008287 */ 	neg.s	$f10,$f16
/*    1e790:	e7b200e0 */ 	swc1	$f18,0xe0($sp)
/*    1e794:	46005182 */ 	mul.s	$f6,$f10,$f0
/*    1e798:	c7b000e8 */ 	lwc1	$f16,0xe8($sp)
/*    1e79c:	46144482 */ 	mul.s	$f18,$f8,$f20
/*    1e7a0:	46123100 */ 	add.s	$f4,$f6,$f18
/*    1e7a4:	46048280 */ 	add.s	$f10,$f16,$f4
/*    1e7a8:	e7aa00e8 */ 	swc1	$f10,0xe8($sp)
/*    1e7ac:	c6480018 */ 	lwc1	$f8,0x18($s2)
/*    1e7b0:	4608d032 */ 	c.eq.s	$f26,$f8
/*    1e7b4:	00000000 */ 	nop
/*    1e7b8:	45000007 */ 	bc1f	.L0001e7d8
/*    1e7bc:	00000000 */ 	nop
/*    1e7c0:	4616f780 */ 	add.s	$f30,$f30,$f22
/*    1e7c4:	461ec03e */ 	c.le.s	$f24,$f30
/*    1e7c8:	00000000 */ 	nop
/*    1e7cc:	45000002 */ 	bc1f	.L0001e7d8
/*    1e7d0:	00000000 */ 	nop
/*    1e7d4:	4618f781 */ 	sub.s	$f30,$f30,$f24
.L0001e7d8:
/*    1e7d8:	52800004 */ 	beqzl	$s4,.L0001e7ec
/*    1e7dc:	2673ffff */ 	addiu	$s3,$s3,-1
/*    1e7e0:	10000002 */ 	b	.L0001e7ec
/*    1e7e4:	26730001 */ 	addiu	$s3,$s3,0x1
/*    1e7e8:	2673ffff */ 	addiu	$s3,$s3,-1
.L0001e7ec:
/*    1e7ec:	02602025 */ 	or	$a0,$s3,$zero
/*    1e7f0:	86250000 */ 	lh	$a1,0x0($s1)
/*    1e7f4:	0c0074c8 */ 	jal	modelConstrainOrWrapAnimFrame
/*    1e7f8:	8e260018 */ 	lw	$a2,0x18($s1)
/*    1e7fc:	a6220016 */ 	sh	$v0,0x16($s1)
/*    1e800:	86290016 */ 	lh	$t1,0x16($s1)
/*    1e804:	86280014 */ 	lh	$t0,0x14($s1)
/*    1e808:	8fa60114 */ 	lw	$a2,0x114($sp)
/*    1e80c:	8fa40118 */ 	lw	$a0,0x118($sp)
/*    1e810:	110900c7 */ 	beq	$t0,$t1,.L0001eb30
/*    1e814:	27ad00fc */ 	addiu	$t5,$sp,0xfc
/*    1e818:	82250008 */ 	lb	$a1,0x8($s1)
/*    1e81c:	86270000 */ 	lh	$a3,0x0($s1)
/*    1e820:	afad0014 */ 	sw	$t5,0x14($sp)
/*    1e824:	afa20010 */ 	sw	$v0,0x10($sp)
/*    1e828:	822c000b */ 	lb	$t4,0xb($s1)
/*    1e82c:	240b0001 */ 	addiu	$t3,$zero,0x1
/*    1e830:	afab00c8 */ 	sw	$t3,0xc8($sp)
/*    1e834:	0c0092d9 */ 	jal	anim00024b64
/*    1e838:	afac0018 */ 	sw	$t4,0x18($sp)
/*    1e83c:	3c013f80 */ 	lui	$at,0x3f80
/*    1e840:	44819000 */ 	mtc1	$at,$f18
/*    1e844:	c7a60110 */ 	lwc1	$f6,0x110($sp)
/*    1e848:	46000586 */ 	mov.s	$f22,$f0
/*    1e84c:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e850:	46123032 */ 	c.eq.s	$f6,$f18
/*    1e854:	c7b00100 */ 	lwc1	$f16,0x100($sp)
/*    1e858:	4503000a */ 	bc1tl	.L0001e884
/*    1e85c:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e860:	46061082 */ 	mul.s	$f2,$f2,$f6
/*    1e864:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e868:	46068102 */ 	mul.s	$f4,$f16,$f6
/*    1e86c:	00000000 */ 	nop
/*    1e870:	46066302 */ 	mul.s	$f12,$f12,$f6
/*    1e874:	e7a200fc */ 	swc1	$f2,0xfc($sp)
/*    1e878:	e7a40100 */ 	swc1	$f4,0x100($sp)
/*    1e87c:	e7ac0104 */ 	swc1	$f12,0x104($sp)
/*    1e880:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
.L0001e884:
/*    1e884:	16800008 */ 	bnez	$s4,.L0001e8a8
/*    1e888:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e88c:	4600d03c */ 	c.lt.s	$f26,$f0
/*    1e890:	46001087 */ 	neg.s	$f2,$f2
/*    1e894:	46006307 */ 	neg.s	$f12,$f12
/*    1e898:	e7a200fc */ 	swc1	$f2,0xfc($sp)
/*    1e89c:	45000002 */ 	bc1f	.L0001e8a8
/*    1e8a0:	e7ac0104 */ 	swc1	$f12,0x104($sp)
/*    1e8a4:	4600c581 */ 	sub.s	$f22,$f24,$f0
.L0001e8a8:
/*    1e8a8:	822a000b */ 	lb	$t2,0xb($s1)
/*    1e8ac:	11400003 */ 	beqz	$t2,.L0001e8bc
/*    1e8b0:	00000000 */ 	nop
/*    1e8b4:	c64a0038 */ 	lwc1	$f10,0x38($s2)
/*    1e8b8:	e7aa0100 */ 	swc1	$f10,0x100($sp)
.L0001e8bc:
/*    1e8bc:	0c0068f4 */ 	jal	cosf
/*    1e8c0:	c64c0030 */ 	lwc1	$f12,0x30($s2)
/*    1e8c4:	46000506 */ 	mov.s	$f20,$f0
/*    1e8c8:	0c0068f7 */ 	jal	sinf
/*    1e8cc:	c64c0030 */ 	lwc1	$f12,0x30($s2)
/*    1e8d0:	3c0e8006 */ 	lui	$t6,%hi(var8005efdc)
/*    1e8d4:	8dceefdc */ 	lw	$t6,%lo(var8005efdc)($t6)
/*    1e8d8:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e8dc:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e8e0:	11c00056 */ 	beqz	$t6,.L0001ea3c
/*    1e8e4:	00000000 */ 	nop
/*    1e8e8:	862f0002 */ 	lh	$t7,0x2($s1)
/*    1e8ec:	11e00053 */ 	beqz	$t7,.L0001ea3c
/*    1e8f0:	00000000 */ 	nop
/*    1e8f4:	c7ac0104 */ 	lwc1	$f12,0x104($sp)
/*    1e8f8:	c7a200fc */ 	lwc1	$f2,0xfc($sp)
/*    1e8fc:	461cd03c */ 	c.lt.s	$f26,$f28
/*    1e900:	46006202 */ 	mul.s	$f8,$f12,$f0
/*    1e904:	46001107 */ 	neg.s	$f4,$f2
/*    1e908:	3c013f00 */ 	lui	$at,0x3f00
/*    1e90c:	46141482 */ 	mul.s	$f18,$f2,$f20
/*    1e910:	00000000 */ 	nop
/*    1e914:	46146402 */ 	mul.s	$f16,$f12,$f20
/*    1e918:	00000000 */ 	nop
/*    1e91c:	46002282 */ 	mul.s	$f10,$f4,$f0
/*    1e920:	46124180 */ 	add.s	$f6,$f8,$f18
/*    1e924:	e7a600d0 */ 	swc1	$f6,0xd0($sp)
/*    1e928:	460a8200 */ 	add.s	$f8,$f16,$f10
/*    1e92c:	45000028 */ 	bc1f	.L0001e9d0
/*    1e930:	e7a800d8 */ 	swc1	$f8,0xd8($sp)
/*    1e934:	c6260058 */ 	lwc1	$f6,0x58($s1)
/*    1e938:	c6320074 */ 	lwc1	$f18,0x74($s1)
/*    1e93c:	c6220054 */ 	lwc1	$f2,0x54($s1)
/*    1e940:	4606e102 */ 	mul.s	$f4,$f28,$f6
/*    1e944:	46049403 */ 	div.s	$f16,$f18,$f4
/*    1e948:	46101001 */ 	sub.s	$f0,$f2,$f16
/*    1e94c:	461a003c */ 	c.lt.s	$f0,$f26
/*    1e950:	00000000 */ 	nop
/*    1e954:	45020003 */ 	bc1fl	.L0001e964
/*    1e958:	46001280 */ 	add.s	$f10,$f2,$f0
/*    1e95c:	4600d006 */ 	mov.s	$f0,$f26
/*    1e960:	46001280 */ 	add.s	$f10,$f2,$f0
.L0001e964:
/*    1e964:	c6460040 */ 	lwc1	$f6,0x40($s2)
/*    1e968:	c652004c */ 	lwc1	$f18,0x4c($s2)
/*    1e96c:	44814000 */ 	mtc1	$at,$f8
/*    1e970:	c7b000bc */ 	lwc1	$f16,0xbc($sp)
/*    1e974:	46123101 */ 	sub.s	$f4,$f6,$f18
/*    1e978:	46085002 */ 	mul.s	$f0,$f10,$f8
/*    1e97c:	00000000 */ 	nop
/*    1e980:	46102282 */ 	mul.s	$f10,$f4,$f16
/*    1e984:	461c5203 */ 	div.s	$f8,$f10,$f28
/*    1e988:	e7a80090 */ 	swc1	$f8,0x90($sp)
/*    1e98c:	c6520054 */ 	lwc1	$f18,0x54($s2)
/*    1e990:	c6460048 */ 	lwc1	$f6,0x48($s2)
/*    1e994:	46123101 */ 	sub.s	$f4,$f6,$f18
/*    1e998:	c7b200d0 */ 	lwc1	$f18,0xd0($sp)
/*    1e99c:	46102282 */ 	mul.s	$f10,$f4,$f16
/*    1e9a0:	46124101 */ 	sub.s	$f4,$f8,$f18
/*    1e9a4:	c7a800d8 */ 	lwc1	$f8,0xd8($sp)
/*    1e9a8:	46002402 */ 	mul.s	$f16,$f4,$f0
/*    1e9ac:	461c5183 */ 	div.s	$f6,$f10,$f28
/*    1e9b0:	46109280 */ 	add.s	$f10,$f18,$f16
/*    1e9b4:	e7aa00d0 */ 	swc1	$f10,0xd0($sp)
/*    1e9b8:	46083101 */ 	sub.s	$f4,$f6,$f8
/*    1e9bc:	e7a60098 */ 	swc1	$f6,0x98($sp)
/*    1e9c0:	46002482 */ 	mul.s	$f18,$f4,$f0
/*    1e9c4:	46124400 */ 	add.s	$f16,$f8,$f18
/*    1e9c8:	10000011 */ 	b	.L0001ea10
/*    1e9cc:	e7b000d8 */ 	swc1	$f16,0xd8($sp)
.L0001e9d0:
/*    1e9d0:	c64a0040 */ 	lwc1	$f10,0x40($s2)
/*    1e9d4:	c646004c */ 	lwc1	$f6,0x4c($s2)
/*    1e9d8:	c6280054 */ 	lwc1	$f8,0x54($s1)
/*    1e9dc:	c7b000d0 */ 	lwc1	$f16,0xd0($sp)
/*    1e9e0:	46065101 */ 	sub.s	$f4,$f10,$f6
/*    1e9e4:	46082482 */ 	mul.s	$f18,$f4,$f8
/*    1e9e8:	46128280 */ 	add.s	$f10,$f16,$f18
/*    1e9ec:	e7aa00d0 */ 	swc1	$f10,0xd0($sp)
/*    1e9f0:	c6440054 */ 	lwc1	$f4,0x54($s2)
/*    1e9f4:	c6460048 */ 	lwc1	$f6,0x48($s2)
/*    1e9f8:	c6300054 */ 	lwc1	$f16,0x54($s1)
/*    1e9fc:	c7aa00d8 */ 	lwc1	$f10,0xd8($sp)
/*    1ea00:	46043201 */ 	sub.s	$f8,$f6,$f4
/*    1ea04:	46104482 */ 	mul.s	$f18,$f8,$f16
/*    1ea08:	46125180 */ 	add.s	$f6,$f10,$f18
/*    1ea0c:	e7a600d8 */ 	swc1	$f6,0xd8($sp)
.L0001ea10:
/*    1ea10:	c7a400d0 */ 	lwc1	$f4,0xd0($sp)
/*    1ea14:	c7a800e0 */ 	lwc1	$f8,0xe0($sp)
/*    1ea18:	c7aa00d8 */ 	lwc1	$f10,0xd8($sp)
/*    1ea1c:	c7b200e8 */ 	lwc1	$f18,0xe8($sp)
/*    1ea20:	46082400 */ 	add.s	$f16,$f4,$f8
/*    1ea24:	c7a40100 */ 	lwc1	$f4,0x100($sp)
/*    1ea28:	46125180 */ 	add.s	$f6,$f10,$f18
/*    1ea2c:	e7b000d0 */ 	swc1	$f16,0xd0($sp)
/*    1ea30:	e7a400d4 */ 	swc1	$f4,0xd4($sp)
/*    1ea34:	1000000f */ 	b	.L0001ea74
/*    1ea38:	e7a600d8 */ 	swc1	$f6,0xd8($sp)
.L0001ea3c:
/*    1ea3c:	46141402 */ 	mul.s	$f16,$f2,$f20
/*    1ea40:	c7a800e0 */ 	lwc1	$f8,0xe0($sp)
/*    1ea44:	c7a40100 */ 	lwc1	$f4,0x100($sp)
/*    1ea48:	46006482 */ 	mul.s	$f18,$f12,$f0
/*    1ea4c:	e7a400d4 */ 	swc1	$f4,0xd4($sp)
/*    1ea50:	46104280 */ 	add.s	$f10,$f8,$f16
/*    1ea54:	46001402 */ 	mul.s	$f16,$f2,$f0
/*    1ea58:	c7a800e8 */ 	lwc1	$f8,0xe8($sp)
/*    1ea5c:	460a9180 */ 	add.s	$f6,$f18,$f10
/*    1ea60:	46146282 */ 	mul.s	$f10,$f12,$f20
/*    1ea64:	46104481 */ 	sub.s	$f18,$f8,$f16
/*    1ea68:	e7a600d0 */ 	swc1	$f6,0xd0($sp)
/*    1ea6c:	46125180 */ 	add.s	$f6,$f10,$f18
/*    1ea70:	e7a600d8 */ 	swc1	$f6,0xd8($sp)
.L0001ea74:
/*    1ea74:	c640005c */ 	lwc1	$f0,0x5c($s2)
/*    1ea78:	4600d03c */ 	c.lt.s	$f26,$f0
/*    1ea7c:	00000000 */ 	nop
/*    1ea80:	45020020 */ 	bc1fl	.L0001eb04
/*    1ea84:	c6520018 */ 	lwc1	$f18,0x18($s2)
/*    1ea88:	461cd03c */ 	c.lt.s	$f26,$f28
/*    1ea8c:	3c013f80 */ 	lui	$at,0x3f80
/*    1ea90:	4502001c */ 	bc1fl	.L0001eb04
/*    1ea94:	c6520018 */ 	lwc1	$f18,0x18($s2)
/*    1ea98:	44812000 */ 	mtc1	$at,$f4
/*    1ea9c:	00000000 */ 	nop
/*    1eaa0:	461c2083 */ 	div.s	$f2,$f4,$f28
/*    1eaa4:	4602003c */ 	c.lt.s	$f0,$f2
/*    1eaa8:	46001306 */ 	mov.s	$f12,$f2
/*    1eaac:	45020005 */ 	bc1fl	.L0001eac4
/*    1eab0:	46020201 */ 	sub.s	$f8,$f0,$f2
/*    1eab4:	46000306 */ 	mov.s	$f12,$f0
/*    1eab8:	10000003 */ 	b	.L0001eac8
/*    1eabc:	e65a005c */ 	swc1	$f26,0x5c($s2)
/*    1eac0:	46020201 */ 	sub.s	$f8,$f0,$f2
.L0001eac4:
/*    1eac4:	e648005c */ 	swc1	$f8,0x5c($s2)
.L0001eac8:
/*    1eac8:	c6500058 */ 	lwc1	$f16,0x58($s2)
/*    1eacc:	460c8282 */ 	mul.s	$f10,$f16,$f12
/*    1ead0:	460ab580 */ 	add.s	$f22,$f22,$f10
/*    1ead4:	461ab03c */ 	c.lt.s	$f22,$f26
/*    1ead8:	00000000 */ 	nop
/*    1eadc:	45020004 */ 	bc1fl	.L0001eaf0
/*    1eae0:	4616c03e */ 	c.le.s	$f24,$f22
/*    1eae4:	10000006 */ 	b	.L0001eb00
/*    1eae8:	4618b580 */ 	add.s	$f22,$f22,$f24
/*    1eaec:	4616c03e */ 	c.le.s	$f24,$f22
.L0001eaf0:
/*    1eaf0:	00000000 */ 	nop
/*    1eaf4:	45020003 */ 	bc1fl	.L0001eb04
/*    1eaf8:	c6520018 */ 	lwc1	$f18,0x18($s2)
/*    1eafc:	4618b581 */ 	sub.s	$f22,$f22,$f24
.L0001eb00:
/*    1eb00:	c6520018 */ 	lwc1	$f18,0x18($s2)
.L0001eb04:
/*    1eb04:	4612d032 */ 	c.eq.s	$f26,$f18
/*    1eb08:	00000000 */ 	nop
/*    1eb0c:	45000008 */ 	bc1f	.L0001eb30
/*    1eb10:	00000000 */ 	nop
/*    1eb14:	4616f000 */ 	add.s	$f0,$f30,$f22
/*    1eb18:	4600c03e */ 	c.le.s	$f24,$f0
/*    1eb1c:	e7a000cc */ 	swc1	$f0,0xcc($sp)
/*    1eb20:	45000003 */ 	bc1f	.L0001eb30
/*    1eb24:	00000000 */ 	nop
/*    1eb28:	46180181 */ 	sub.s	$f6,$f0,$f24
/*    1eb2c:	e7a600cc */ 	swc1	$f6,0xcc($sp)
.L0001eb30:
/*    1eb30:	1000febc */ 	b	.L0001e624
/*    1eb34:	8fa700ec */ 	lw	$a3,0xec($sp)
.L0001eb38:
/*    1eb38:	c7a400e0 */ 	lwc1	$f4,0xe0($sp)
.L0001eb3c:
/*    1eb3c:	e6440034 */ 	swc1	$f4,0x34($s2)
/*    1eb40:	c7a800e4 */ 	lwc1	$f8,0xe4($sp)
/*    1eb44:	e6480038 */ 	swc1	$f8,0x38($s2)
/*    1eb48:	c7b000e8 */ 	lwc1	$f16,0xe8($sp)
/*    1eb4c:	e65e0030 */ 	swc1	$f30,0x30($s2)
/*    1eb50:	e650003c */ 	swc1	$f16,0x3c($s2)
/*    1eb54:	c7aa00d0 */ 	lwc1	$f10,0xd0($sp)
/*    1eb58:	e64a0024 */ 	swc1	$f10,0x24($s2)
/*    1eb5c:	c7b200d4 */ 	lwc1	$f18,0xd4($sp)
/*    1eb60:	e6520028 */ 	swc1	$f18,0x28($s2)
/*    1eb64:	c7a600d8 */ 	lwc1	$f6,0xd8($sp)
/*    1eb68:	e646002c */ 	swc1	$f6,0x2c($s2)
/*    1eb6c:	c7a400cc */ 	lwc1	$f4,0xcc($sp)
/*    1eb70:	e6440020 */ 	swc1	$f4,0x20($s2)
/*    1eb74:	8fb900c8 */ 	lw	$t9,0xc8($sp)
/*    1eb78:	a2590001 */ 	sb	$t9,0x1($s2)
/*    1eb7c:	86220014 */ 	lh	$v0,0x14($s1)
/*    1eb80:	86230016 */ 	lh	$v1,0x16($s1)
/*    1eb84:	14620006 */ 	bne	$v1,$v0,.L0001eba0
/*    1eb88:	00000000 */ 	nop
/*    1eb8c:	44824000 */ 	mtc1	$v0,$f8
/*    1eb90:	e63a0010 */ 	swc1	$f26,0x10($s1)
/*    1eb94:	46804420 */ 	cvt.s.w	$f16,$f8
/*    1eb98:	10000019 */ 	b	.L0001ec00
/*    1eb9c:	e630000c */ 	swc1	$f16,0xc($s1)
.L0001eba0:
/*    1eba0:	5280000c */ 	beqzl	$s4,.L0001ebd4
/*    1eba4:	44879000 */ 	mtc1	$a3,$f18
/*    1eba8:	44879000 */ 	mtc1	$a3,$f18
/*    1ebac:	44822000 */ 	mtc1	$v0,$f4
/*    1ebb0:	c7aa0138 */ 	lwc1	$f10,0x138($sp)
/*    1ebb4:	468091a0 */ 	cvt.s.w	$f6,$f18
/*    1ebb8:	46802220 */ 	cvt.s.w	$f8,$f4
/*    1ebbc:	46065001 */ 	sub.s	$f0,$f10,$f6
/*    1ebc0:	46004400 */ 	add.s	$f16,$f8,$f0
/*    1ebc4:	e6200010 */ 	swc1	$f0,0x10($s1)
/*    1ebc8:	1000000d */ 	b	.L0001ec00
/*    1ebcc:	e630000c */ 	swc1	$f16,0xc($s1)
/*    1ebd0:	44879000 */ 	mtc1	$a3,$f18
.L0001ebd4:
/*    1ebd4:	c7a60138 */ 	lwc1	$f6,0x138($sp)
/*    1ebd8:	44832000 */ 	mtc1	$v1,$f4
/*    1ebdc:	468092a0 */ 	cvt.s.w	$f10,$f18
/*    1ebe0:	3c013f80 */ 	lui	$at,0x3f80
/*    1ebe4:	44818000 */ 	mtc1	$at,$f16
/*    1ebe8:	46802220 */ 	cvt.s.w	$f8,$f4
/*    1ebec:	46065001 */ 	sub.s	$f0,$f10,$f6
/*    1ebf0:	46008481 */ 	sub.s	$f18,$f16,$f0
/*    1ebf4:	e6200010 */ 	swc1	$f0,0x10($s1)
/*    1ebf8:	46124280 */ 	add.s	$f10,$f8,$f18
/*    1ebfc:	e62a000c */ 	swc1	$f10,0xc($s1)
.L0001ec00:
/*    1ec00:	86380002 */ 	lh	$t8,0x2($s1)
/*    1ec04:	13000066 */ 	beqz	$t8,.L0001eda0
/*    1ec08:	00000000 */ 	nop
/*    1ec0c:	86290000 */ 	lh	$t1,0x0($s1)
/*    1ec10:	3c088006 */ 	lui	$t0,%hi(g_Anims)
/*    1ec14:	8d08f00c */ 	lw	$t0,%lo(g_Anims)($t0)
/*    1ec18:	00096880 */ 	sll	$t5,$t1,0x2
/*    1ec1c:	01a96823 */ 	subu	$t5,$t5,$t1
/*    1ec20:	000d6880 */ 	sll	$t5,$t5,0x2
/*    1ec24:	010d6021 */ 	addu	$t4,$t0,$t5
/*    1ec28:	918b000b */ 	lbu	$t3,0xb($t4)
/*    1ec2c:	316a0002 */ 	andi	$t2,$t3,0x2
/*    1ec30:	1540005b */ 	bnez	$t2,.L0001eda0
/*    1ec34:	00000000 */ 	nop
/*    1ec38:	0fc25e42 */ 	jal	floor
/*    1ec3c:	c7ac013c */ 	lwc1	$f12,0x13c($sp)
/*    1ec40:	c7b40140 */ 	lwc1	$f20,0x140($sp)
/*    1ec44:	00408025 */ 	or	$s0,$v0,$zero
/*    1ec48:	0fc25e42 */ 	jal	floor
/*    1ec4c:	4600a306 */ 	mov.s	$f12,$f20
/*    1ec50:	12800004 */ 	beqz	$s4,.L0001ec64
/*    1ec54:	00409825 */ 	or	$s3,$v0,$zero
/*    1ec58:	0202082a */ 	slt	$at,$s0,$v0
/*    1ec5c:	54200006 */ 	bnezl	$at,.L0001ec78
/*    1ec60:	824e0002 */ 	lb	$t6,0x2($s2)
.L0001ec64:
/*    1ec64:	16800032 */ 	bnez	$s4,.L0001ed30
/*    1ec68:	0050082a */ 	slt	$at,$v0,$s0
/*    1ec6c:	50200031 */ 	beqzl	$at,.L0001ed34
/*    1ec70:	3c013f80 */ 	lui	$at,0x3f80
/*    1ec74:	824e0002 */ 	lb	$t6,0x2($s2)
.L0001ec78:
/*    1ec78:	02602025 */ 	or	$a0,$s3,$zero
/*    1ec7c:	51c00005 */ 	beqzl	$t6,.L0001ec94
/*    1ec80:	c6440038 */ 	lwc1	$f4,0x38($s2)
/*    1ec84:	c6460044 */ 	lwc1	$f6,0x44($s2)
/*    1ec88:	10000003 */ 	b	.L0001ec98
/*    1ec8c:	e6460050 */ 	swc1	$f6,0x50($s2)
/*    1ec90:	c6440038 */ 	lwc1	$f4,0x38($s2)
.L0001ec94:
/*    1ec94:	e6440050 */ 	swc1	$f4,0x50($s2)
.L0001ec98:
/*    1ec98:	86250002 */ 	lh	$a1,0x2($s1)
/*    1ec9c:	0c0074c8 */ 	jal	modelConstrainOrWrapAnimFrame
/*    1eca0:	8e26003c */ 	lw	$a2,0x3c($s1)
/*    1eca4:	a6220038 */ 	sh	$v0,0x38($s1)
/*    1eca8:	26640001 */ 	addiu	$a0,$s3,0x1
/*    1ecac:	86250002 */ 	lh	$a1,0x2($s1)
/*    1ecb0:	0c0074c8 */ 	jal	modelConstrainOrWrapAnimFrame
/*    1ecb4:	8e26003c */ 	lw	$a2,0x3c($s1)
/*    1ecb8:	a622003a */ 	sh	$v0,0x3a($s1)
/*    1ecbc:	82250009 */ 	lb	$a1,0x9($s1)
/*    1ecc0:	86270002 */ 	lh	$a3,0x2($s1)
/*    1ecc4:	27af00fc */ 	addiu	$t7,$sp,0xfc
/*    1ecc8:	afaf0014 */ 	sw	$t7,0x14($sp)
/*    1eccc:	afa20010 */ 	sw	$v0,0x10($sp)
/*    1ecd0:	8239000b */ 	lb	$t9,0xb($s1)
/*    1ecd4:	8fa60114 */ 	lw	$a2,0x114($sp)
/*    1ecd8:	8fa40118 */ 	lw	$a0,0x118($sp)
/*    1ecdc:	0c0092d9 */ 	jal	anim00024b64
/*    1ece0:	afb90018 */ 	sw	$t9,0x18($sp)
/*    1ece4:	3c013f80 */ 	lui	$at,0x3f80
/*    1ece8:	44816000 */ 	mtc1	$at,$f12
/*    1ecec:	c7a20110 */ 	lwc1	$f2,0x110($sp)
/*    1ecf0:	c7b00100 */ 	lwc1	$f16,0x100($sp)
/*    1ecf4:	460c1032 */ 	c.eq.s	$f2,$f12
/*    1ecf8:	00000000 */ 	nop
/*    1ecfc:	45030004 */ 	bc1tl	.L0001ed10
/*    1ed00:	8238000b */ 	lb	$t8,0xb($s1)
/*    1ed04:	46028202 */ 	mul.s	$f8,$f16,$f2
/*    1ed08:	e7a80100 */ 	swc1	$f8,0x100($sp)
/*    1ed0c:	8238000b */ 	lb	$t8,0xb($s1)
.L0001ed10:
/*    1ed10:	53000004 */ 	beqzl	$t8,.L0001ed24
/*    1ed14:	c7aa0100 */ 	lwc1	$f10,0x100($sp)
/*    1ed18:	c6520050 */ 	lwc1	$f18,0x50($s2)
/*    1ed1c:	e7b20100 */ 	swc1	$f18,0x100($sp)
/*    1ed20:	c7aa0100 */ 	lwc1	$f10,0x100($sp)
.L0001ed24:
/*    1ed24:	24090001 */ 	addiu	$t1,$zero,0x1
/*    1ed28:	a2490002 */ 	sb	$t1,0x2($s2)
/*    1ed2c:	e64a0044 */ 	swc1	$f10,0x44($s2)
.L0001ed30:
/*    1ed30:	3c013f80 */ 	lui	$at,0x3f80
.L0001ed34:
/*    1ed34:	44816000 */ 	mtc1	$at,$f12
/*    1ed38:	5280000d */ 	beqzl	$s4,.L0001ed70
/*    1ed3c:	44935000 */ 	mtc1	$s3,$f10
/*    1ed40:	44933000 */ 	mtc1	$s3,$f6
/*    1ed44:	86280038 */ 	lh	$t0,0x38($s1)
/*    1ed48:	46803120 */ 	cvt.s.w	$f4,$f6
/*    1ed4c:	44888000 */ 	mtc1	$t0,$f16
/*    1ed50:	00000000 */ 	nop
/*    1ed54:	46808220 */ 	cvt.s.w	$f8,$f16
/*    1ed58:	4604a001 */ 	sub.s	$f0,$f20,$f4
/*    1ed5c:	46004480 */ 	add.s	$f18,$f8,$f0
/*    1ed60:	e6200034 */ 	swc1	$f0,0x34($s1)
/*    1ed64:	1000001e */ 	b	.L0001ede0
/*    1ed68:	e6320030 */ 	swc1	$f18,0x30($s1)
/*    1ed6c:	44935000 */ 	mtc1	$s3,$f10
.L0001ed70:
/*    1ed70:	862d003a */ 	lh	$t5,0x3a($s1)
/*    1ed74:	468051a0 */ 	cvt.s.w	$f6,$f10
/*    1ed78:	448d8000 */ 	mtc1	$t5,$f16
/*    1ed7c:	00000000 */ 	nop
/*    1ed80:	46808220 */ 	cvt.s.w	$f8,$f16
/*    1ed84:	4606a101 */ 	sub.s	$f4,$f20,$f6
/*    1ed88:	46046001 */ 	sub.s	$f0,$f12,$f4
/*    1ed8c:	46006481 */ 	sub.s	$f18,$f12,$f0
/*    1ed90:	e6200034 */ 	swc1	$f0,0x34($s1)
/*    1ed94:	46124280 */ 	add.s	$f10,$f8,$f18
/*    1ed98:	10000011 */ 	b	.L0001ede0
/*    1ed9c:	e62a0030 */ 	swc1	$f10,0x30($s1)
.L0001eda0:
/*    1eda0:	1000000f */ 	b	.L0001ede0
/*    1eda4:	a2400002 */ 	sb	$zero,0x2($s2)
/*    1eda8:	c7b40140 */ 	lwc1	$f20,0x140($sp)
.L0001edac:
/*    1edac:	02002025 */ 	or	$a0,$s0,$zero
/*    1edb0:	8fa50138 */ 	lw	$a1,0x138($sp)
/*    1edb4:	4406a000 */ 	mfc1	$a2,$f20
/*    1edb8:	0c007853 */ 	jal	model0001e14c
/*    1edbc:	00000000 */ 	nop
/*    1edc0:	10000008 */ 	b	.L0001ede4
/*    1edc4:	8fbf006c */ 	lw	$ra,0x6c($sp)
/*    1edc8:	c7b40140 */ 	lwc1	$f20,0x140($sp)
.L0001edcc:
/*    1edcc:	02002025 */ 	or	$a0,$s0,$zero
/*    1edd0:	8fa50138 */ 	lw	$a1,0x138($sp)
/*    1edd4:	4406a000 */ 	mfc1	$a2,$f20
/*    1edd8:	0c007853 */ 	jal	model0001e14c
/*    1eddc:	00000000 */ 	nop
.L0001ede0:
/*    1ede0:	8fbf006c */ 	lw	$ra,0x6c($sp)
.L0001ede4:
/*    1ede4:	d7b40028 */ 	ldc1	$f20,0x28($sp)
/*    1ede8:	d7b60030 */ 	ldc1	$f22,0x30($sp)
/*    1edec:	d7b80038 */ 	ldc1	$f24,0x38($sp)
/*    1edf0:	d7ba0040 */ 	ldc1	$f26,0x40($sp)
/*    1edf4:	d7bc0048 */ 	ldc1	$f28,0x48($sp)
/*    1edf8:	d7be0050 */ 	ldc1	$f30,0x50($sp)
/*    1edfc:	8fb00058 */ 	lw	$s0,0x58($sp)
/*    1ee00:	8fb1005c */ 	lw	$s1,0x5c($sp)
/*    1ee04:	8fb20060 */ 	lw	$s2,0x60($sp)
/*    1ee08:	8fb30064 */ 	lw	$s3,0x64($sp)
/*    1ee0c:	8fb40068 */ 	lw	$s4,0x68($sp)
/*    1ee10:	03e00008 */ 	jr	$ra
/*    1ee14:	27bd0130 */ 	addiu	$sp,$sp,0x130
);

void model0001ee18(struct model *model, s32 lvupdate240, bool arg2)
{
	f32 frame;
	f32 frame2;
	f32 speed;
	f32 speed2;
	f32 startframe;
	f32 endframe;
	f32 realendframe;
	struct anim *anim = model->anim;

	if (anim && lvupdate240 > 0) {
		frame = anim->frame;
		frame2 = anim->frame2;

		for (; lvupdate240 > 0; lvupdate240--) {
			if (anim->timeplay > 0) {
				anim->elapseplay += 0.25f;

				if (anim->elapseplay < anim->timeplay) {
					anim->playspeed = anim->oldplay + (anim->newplay - anim->oldplay) * anim->elapseplay / anim->timeplay;
				} else {
					anim->timeplay = 0;
					anim->playspeed = anim->newplay;
				}
			}

			if (anim->timemerge > 0) {
				anim->elapsemerge += anim->playspeed * 0.25f;

				if (anim->elapsemerge == 0) {
					anim->fracmerge = 1;
				} else {
					if (anim->elapsemerge < anim->timemerge) {
						anim->fracmerge = (anim->timemerge - anim->elapsemerge) / anim->timemerge;
					} else {
						anim->timemerge = 0;
						anim->fracmerge = 0;
						anim->animnum2 = 0;
					}
				}
			}

			if (anim->timespeed > 0) {
				anim->elapsespeed += anim->playspeed * 0.25f;

				if (anim->elapsespeed < anim->timespeed) {
					anim->speed = anim->oldspeed + (anim->newspeed - anim->oldspeed) * anim->elapsespeed / anim->timespeed;
				} else {
					anim->timespeed = 0;
					anim->speed = anim->newspeed;
				}
			}

			speed = anim->speed;
			frame += anim->playspeed * speed * 0.25f;

			if (anim->animnum2) {
				if (anim->timespeed2 > 0) {
					anim->elapsespeed2 += anim->playspeed * 0.25f;

					if (anim->elapsespeed2 < anim->timespeed2) {
						anim->speed2 = anim->oldspeed2 + (anim->newspeed2 - anim->oldspeed2) * anim->elapsespeed2 / anim->timespeed2;
					} else {
						anim->timespeed2 = 0;
						anim->speed2 = anim->newspeed2;
					}
				}

				speed2 = anim->speed2;
				frame2 += anim->playspeed * speed2 * 0.25f;
			}

			if (anim->looping) {
				realendframe = anim->endframe;

				if (speed >= 0) {
					endframe = animGetNumFrames(anim->animnum) - 1;
					startframe = anim->loopframe;

					if (realendframe >= 0 && endframe > realendframe) {
						endframe = realendframe;
					}
				} else {
					endframe = anim->loopframe;
					startframe = animGetNumFrames(anim->animnum) - 1;

					if (realendframe >= 0 && startframe > realendframe) {
						startframe = realendframe;
					}
				}

				if ((speed >= 0 && frame >= endframe) || (speed < 0 && frame <= endframe)) {
					f32 prevnewspeed = anim->newspeed;
					f32 prevoldspeed = anim->oldspeed;
					f32 prevtimespeed = anim->timespeed;
					f32 prevelapsespeed = anim->elapsespeed;

					if (arg2) {
						model0001e2b4(model, anim->frame, endframe, 0, 0);
					} else {
						model0001e14c(model, endframe, 0);
					}

					modelSetAnimation(model, anim->animnum, anim->flip, startframe, anim->speed, anim->loopmerge);

					anim->looping = true;
					anim->endframe = realendframe;

					anim->newspeed = prevnewspeed;
					anim->oldspeed = prevoldspeed;
					anim->timespeed = prevtimespeed;
					anim->elapsespeed = prevelapsespeed;

					frame2 = frame;
					frame = startframe + frame - endframe;

					if (anim->flipfunc) {
						anim->flipfunc();
					}
				}
			}
		}

		if (arg2) {
			if (anim->animnum2) {
				model0001e2b4(model, anim->frame, frame, anim->frame2, frame2);
			} else {
				model0001e2b4(model, anim->frame, frame, 0, 0);
			}
		} else {
			if (anim->animnum2) {
				model0001e14c(model, frame, frame2);
			} else {
				model0001e14c(model, frame, 0);
			}
		}
	}
}

#if VERSION < VERSION_PAL_BETA
/**
 * This is identical to the above function but removes the 0.25f multipliers.
 */
void model0001f314(struct model *model, s32 lvupdate240, bool arg2)
{
	f32 frame;
	f32 frame2;
	f32 speed;
	f32 speed2;
	f32 startframe;
	f32 endframe;
	f32 realendframe;
	struct anim *anim = model->anim;

	if (anim && lvupdate240 > 0) {
		frame = anim->frame;
		frame2 = anim->frame2;

		for (; lvupdate240 > 0; lvupdate240--) {
			if (anim->timeplay > 0) {
				anim->elapseplay++;

				if (anim->elapseplay < anim->timeplay) {
					anim->playspeed = anim->oldplay + (anim->newplay - anim->oldplay) * anim->elapseplay / anim->timeplay;
				} else {
					anim->timeplay = 0;
					anim->playspeed = anim->newplay;
				}
			}

			if (anim->timemerge > 0) {
				anim->elapsemerge += anim->playspeed;

				if (anim->elapsemerge == 0) {
					anim->fracmerge = 1;
				} else {
					if (anim->elapsemerge < anim->timemerge) {
						anim->fracmerge = (anim->timemerge - anim->elapsemerge) / anim->timemerge;
					} else {
						anim->timemerge = 0;
						anim->fracmerge = 0;
						anim->animnum2 = 0;
					}
				}
			}

			if (anim->timespeed > 0) {
				anim->elapsespeed += anim->playspeed;

				if (anim->elapsespeed < anim->timespeed) {
					anim->speed = anim->oldspeed + (anim->newspeed - anim->oldspeed) * anim->elapsespeed / anim->timespeed;
				} else {
					anim->timespeed = 0;
					anim->speed = anim->newspeed;
				}
			}

			speed = anim->speed;
			frame += anim->playspeed * speed;

			if (anim->animnum2) {
				if (anim->timespeed2 > 0) {
					anim->elapsespeed2 += anim->playspeed;

					if (anim->elapsespeed2 < anim->timespeed2) {
						anim->speed2 = anim->oldspeed2 + (anim->newspeed2 - anim->oldspeed2) * anim->elapsespeed2 / anim->timespeed2;
					} else {
						anim->timespeed2 = 0;
						anim->speed2 = anim->newspeed2;
					}
				}

				speed2 = anim->speed2;
				frame2 += anim->playspeed * speed2;
			}

			if (anim->looping) {
				realendframe = anim->endframe;

				if (speed >= 0) {
					endframe = animGetNumFrames(anim->animnum) - 1;
					startframe = anim->loopframe;

					if (realendframe >= 0 && endframe > realendframe) {
						endframe = realendframe;
					}
				} else {
					endframe = anim->loopframe;
					startframe = animGetNumFrames(anim->animnum) - 1;

					if (realendframe >= 0 && startframe > realendframe) {
						startframe = realendframe;
					}
				}

				if ((speed >= 0 && frame >= endframe) || (speed < 0 && frame <= endframe)) {
					f32 prevnewspeed = anim->newspeed;
					f32 prevoldspeed = anim->oldspeed;
					f32 prevtimespeed = anim->timespeed;
					f32 prevelapsespeed = anim->elapsespeed;

					if (arg2) {
						model0001e2b4(model, anim->frame, endframe, 0, 0);
					} else {
						model0001e14c(model, endframe, 0);
					}

					modelSetAnimation(model, anim->animnum, anim->flip, startframe, anim->speed, anim->loopmerge);

					anim->looping = true;
					anim->endframe = realendframe;

					anim->newspeed = prevnewspeed;
					anim->oldspeed = prevoldspeed;
					anim->timespeed = prevtimespeed;
					anim->elapsespeed = prevelapsespeed;

					frame2 = frame;
					frame = startframe + frame - endframe;

					if (anim->flipfunc) {
						anim->flipfunc();
					}
				}
			}
		}

		if (arg2) {
			if (anim->animnum2) {
				model0001e2b4(model, anim->frame, frame, anim->frame2, frame2);
			} else {
				model0001e2b4(model, anim->frame, frame, 0, 0);
			}
		} else {
			if (anim->animnum2) {
				model0001e14c(model, frame, frame2);
			} else {
				model0001e14c(model, frame, 0);
			}
		}
	}
}
#endif

void model0001f7e0(struct modelrenderdata *renderdata)
{
	gDPPipeSync(renderdata->gdl++);
	gDPSetCycleType(renderdata->gdl++, G_CYC_1CYCLE);

	if (renderdata->zbufferenabled) {
		gDPSetRenderMode(renderdata->gdl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
	} else {
		gDPSetRenderMode(renderdata->gdl++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
	}

	gDPSetCombineMode(renderdata->gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
}

void model0001f890(struct modelrenderdata *renderdata, bool arg1)
{
	if (renderdata->unk30 == 7) {
		if (arg1) {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
			gDPSetEnvColorViaWord(renderdata->gdl++, renderdata->envcolour | 0xff);
			gDPSetCombineLERP(renderdata->gdl++, TEXEL0, ENVIRONMENT, SHADE_ALPHA, ENVIRONMENT, TEXEL0, ENVIRONMENT, SHADE, ENVIRONMENT, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED);

			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
			}
		} else {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		}
	} else if (renderdata->unk30 == 8) {
		if (arg1) {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
			gDPSetEnvColorViaWord(renderdata->gdl++, renderdata->envcolour);
			gDPSetCombineLERP(renderdata->gdl++, TEXEL0, ENVIRONMENT, SHADE_ALPHA, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED);

			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		}
	} else if (renderdata->unk30 == 9) {
		if ((renderdata->envcolour & 0xff) == 0) {
			if (arg1) {
				gDPPipeSync(renderdata->gdl++);
				gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
				gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
				gDPSetEnvColorViaWord(renderdata->gdl++, 0xffffffff);
				gDPSetPrimColor(renderdata->gdl++, 0, 0, 0, 0, 0, (renderdata->envcolour >> 8) & 0xff);
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, PRIMITIVE);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
				}
			} else {
				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
				}
			}
		} else {
			if (arg1) {
				gDPPipeSync(renderdata->gdl++);
				gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
				gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
				gDPSetEnvColor(renderdata->gdl++, 0, 0, 0, renderdata->envcolour);
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 1, 0, SHADE, ENVIRONMENT, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_TEX_EDGE2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_TEX_EDGE2);
				}
			} else {
				gDPSetPrimColor(renderdata->gdl++, 0, 0, 0, 0, 0, (renderdata->envcolour >> 8) & 0xff);
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, SHADE, ENVIRONMENT, TEXEL0, 0, COMBINED, 0, SHADE, 0, 1, 0, PRIMITIVE, COMBINED);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_TEX_EDGE2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_TEX_EDGE2);
				}
			}
		}
	} else if (renderdata->unk30 == 4) {
		if (arg1) {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetFogColorViaWord(renderdata->gdl++, renderdata->envcolour);
			gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);

			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
			}
		} else {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		}
	} else if (renderdata->unk30 == 5) {
		u8 alpha;

		if (arg1) {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);

			alpha = renderdata->envcolour & 0xff;

			if (alpha < 255) {
				gDPSetEnvColor(renderdata->gdl++, 0xff, 0xff, 0xff, alpha);

				if (renderdata->envcolour & 0xff00) {
					gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 1, SHADE, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
				} else {
					gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 1, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
				}
			} else {
				gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);
			}

			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		} else {
			alpha = renderdata->envcolour & 0xff;

			if (alpha < 255) {
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
			} else {
				gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);
			}
		}
	} else {
		if (arg1) {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);

			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_PASS, G_RM_AA_ZB_OPA_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_PASS, G_RM_AA_OPA_SURF2);
			}
		} else {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_PASS, G_RM_AA_XLU_SURF2);
			}
		}
	}
}

void model00020248(struct modelrenderdata *renderdata, bool arg1)
{
	if (renderdata->unk30 == 7) {
		gDPPipeSync(renderdata->gdl++);
		gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
		gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
		gDPSetEnvColorViaWord(renderdata->gdl++, renderdata->envcolour | 0x000000ff);
		gDPSetCombineLERP(renderdata->gdl++, TEXEL0, ENVIRONMENT, SHADE_ALPHA, ENVIRONMENT, TEXEL0, ENVIRONMENT, SHADE, ENVIRONMENT, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED);

		if (arg1) {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
			}
		} else {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		}
	} else if (renderdata->unk30 == 8) {
		gDPPipeSync(renderdata->gdl++);
		gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
		gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
		gDPSetEnvColorViaWord(renderdata->gdl++, renderdata->envcolour);
		gDPSetCombineLERP(renderdata->gdl++, TEXEL0, ENVIRONMENT, SHADE_ALPHA, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED);

		if (renderdata->zbufferenabled) {
			gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
		} else {
			gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
		}
	} else if (renderdata->unk30 == 9) {
		if ((renderdata->envcolour & 0xff) == 0) {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
			gDPSetEnvColorViaWord(renderdata->gdl++, 0xffffffff);
			gDPSetPrimColor(renderdata->gdl++, 0, 0, 0, 0, 0, (renderdata->envcolour >> 8) & 0xff);

			if (arg1) {
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, PRIMITIVE);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
				}
			} else {
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, PRIMITIVE);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
				}
			}
		} else {
			gDPPipeSync(renderdata->gdl++);
			gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
			gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);
			gDPSetEnvColorViaWord(renderdata->gdl++, renderdata->envcolour & 0xff);

			if (arg1) {
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 1, 0, SHADE, ENVIRONMENT, COMBINED, 0, SHADE, 0, 0, 0, 0, COMBINED);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_TEX_EDGE2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_TEX_EDGE2);
				}
			} else {
				gDPSetPrimColor(renderdata->gdl++, 0, 0, 0, 0, 0, (renderdata->envcolour >> 8) & 0xff);
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, SHADE, ENVIRONMENT, TEXEL0, 0, COMBINED, 0, SHADE, 0, 1, 0, PRIMITIVE, COMBINED);

				if (renderdata->zbufferenabled) {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_TEX_EDGE2);
				} else {
					gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_TEX_EDGE2);
				}
			}
		}
	} else if (renderdata->unk30 == 4) {
		gDPPipeSync(renderdata->gdl++);
		gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
		gDPSetFogColorViaWord(renderdata->gdl++, renderdata->envcolour);
		gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);

		if (arg1) {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
			}
		} else {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		}
	} else if (renderdata->unk30 == 5) {
		u8 alpha;

		gDPPipeSync(renderdata->gdl++);
		gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
		gDPSetFogColorViaWord(renderdata->gdl++, renderdata->fogcolour);

		alpha = renderdata->envcolour & 0xff;

		if (alpha < 255) {
			gDPSetEnvColor(renderdata->gdl++, 0xff, 0xff, 0xff, alpha);

			if (arg1) {
				if (renderdata->envcolour & 0xff00) {
					gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 1, SHADE, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
				} else {
					gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, 1, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
				}
			} else {
				gDPSetCombineLERP(renderdata->gdl++, TEXEL1, TEXEL0, LOD_FRACTION, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, COMBINED, 0, SHADE, 0, COMBINED, 0, SHADE, 0);
			}
		} else {
			gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);
		}

		if (renderdata->zbufferenabled) {
			gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
		} else {
			gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
		}
	} else {
		gDPPipeSync(renderdata->gdl++);
		gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);
		gDPSetFogColorViaWord(renderdata->gdl++, 0xffffff00);
		gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);

		if (arg1) {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_OPA_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_OPA_SURF2);
			}
		} else {
			if (renderdata->zbufferenabled) {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_ZB_XLU_SURF2);
			} else {
				gDPSetRenderMode(renderdata->gdl++, G_RM_FOG_PRIM_A, G_RM_AA_XLU_SURF2);
			}
		}
	}
}

void model00020bdc(struct modelrenderdata *renderdata)
{
	gDPPipeSync(renderdata->gdl++);
	gDPSetCycleType(renderdata->gdl++, G_CYC_2CYCLE);

	if (renderdata->zbufferenabled) {
		gDPSetRenderMode(renderdata->gdl++, G_RM_PASS, G_RM_AA_ZB_OPA_SURF2);
	} else {
		gDPSetRenderMode(renderdata->gdl++, G_RM_PASS, G_RM_AA_OPA_SURF2);
	}

	gDPSetCombineMode(renderdata->gdl++, G_CC_TRILERP, G_CC_MODULATEIA2);
}

void modelApplyCullMode(struct modelrenderdata *renderdata)
{
	if (renderdata->cullmode == CULLMODE_NONE) {
		gSPClearGeometryMode(renderdata->gdl++, G_CULL_BOTH);
	} else if (renderdata->cullmode == CULLMODE_FRONT) {
		gSPSetGeometryMode(renderdata->gdl++, G_CULL_FRONT);
	} else if (renderdata->cullmode == CULLMODE_BACK) {
		gSPSetGeometryMode(renderdata->gdl++, G_CULL_BACK);
	}
}

void modelRenderNodeGundl(struct modelrenderdata *renderdata, struct model *model, struct modelnode *node)
{
	struct modelrodata_gundl *rodata = &node->rodata->gundl;

	if (var8005efc4 && !var8005efc4(model, node)) {
		return;
	}

	if ((renderdata->flags & 1) && rodata->primary) {
		gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL1, osVirtualToPhysical(rodata->baseaddr));

		if (renderdata->cullmode) {
			modelApplyCullMode(renderdata);
		}

		switch (rodata->unk12) {
		case 1:
			model0001f7e0(renderdata);
			break;
		case 3:
			model0001f890(renderdata, true);
			break;
		case 4:
			model00020248(renderdata, true);
			break;
		case 2:
			model00020bdc(renderdata);
			break;
		}

		gSPDisplayList(renderdata->gdl++, rodata->primary);

		if (rodata->unk12 == 3 && rodata->secondary) {
			model0001f890(renderdata, false);

			gSPDisplayList(renderdata->gdl++, rodata->secondary);
		}
	}

	if ((renderdata->flags & 2) && rodata->primary && rodata->unk12 == 4 && rodata->secondary) {
		gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL1, osVirtualToPhysical(rodata->baseaddr));

		if (renderdata->cullmode) {
			modelApplyCullMode(renderdata);
		}

		model00020248(renderdata, false);

		gSPDisplayList(renderdata->gdl++, rodata->secondary);
	}
}

void modelRenderNodeDl(struct modelrenderdata *renderdata, struct model *model, struct modelnode *node)
{
	union modelrodata *rodata = node->rodata;

	if (var8005efc4 && !var8005efc4(model, node)) {
		return;
	}

	if (renderdata->flags & 1) {
		union modelrwdata *rwdata = modelGetNodeRwData(model, node);

		if (rwdata->dl.gdl) {
			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL1, osVirtualToPhysical(rodata->dl.colourtable));

			if (renderdata->cullmode) {
				modelApplyCullMode(renderdata);
			}

			switch (rodata->dl.mcount) {
			case 1:
				model0001f7e0(renderdata);
				break;
			case 3:
				model0001f890(renderdata, true);
				break;
			case 4:
				model00020248(renderdata, true);
				break;
			case 2:
				model00020bdc(renderdata);
				break;
			}

			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_VTX, osVirtualToPhysical(rwdata->dl.vertices));
			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL2, osVirtualToPhysical(rwdata->dl.colours));

			gSPDisplayList(renderdata->gdl++, rwdata->dl.gdl);

			if (rodata->dl.mcount == 3 && rodata->dl.secondary) {
				model0001f890(renderdata, false);

				gSPDisplayList(renderdata->gdl++, rodata->dl.secondary);
			}
		}
	}

	if (renderdata->flags & 2) {
		union modelrwdata *rwdata = modelGetNodeRwData(model, node);

		if (rwdata->dl.gdl && rodata->dl.mcount == 4 && rodata->dl.secondary) {
			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL1, osVirtualToPhysical(rodata->dl.colourtable));

			if (renderdata->cullmode) {
				modelApplyCullMode(renderdata);
			}

			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_VTX, osVirtualToPhysical(rwdata->dl.vertices));
			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL2, osVirtualToPhysical(rwdata->dl.colours));

			model00020248(renderdata, false);

			gSPDisplayList(renderdata->gdl++, rodata->dl.secondary);
		}
	}
}

/**
 * Star gunfire is a muzzle flash in a first person perspective, where the
 * muzzle flash has 3 or 4 "arms" that flare out from the main body.
 *
 * Some weapons that use this are the Cyclone, Dragon, K7 Avenger, AR34,
 * SuperDragon and RC-P45.
 *
 * This function reads vertices from the model definition, tweaks them randomly,
 * writes them to a newly allocated vertices table and queues the node's
 * displaylist to the renderdata's DL.
 */
void modelRenderNodeStarGunfire(struct modelrenderdata *renderdata, struct modelnode *node)
{
	if (renderdata->flags & 2) {
		struct modelrodata_stargunfire *rodata = &node->rodata->stargunfire;
		s32 i;

		if (rodata->gdl) {
			struct gfxvtx *src = (struct gfxvtx *) rodata->vertices;
			struct gfxvtx *dst = g_ModelVtxAllocatorFunc(rodata->unk00 * 4);

			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_VTX, osVirtualToPhysical(dst));
			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL2, osVirtualToPhysical((void *)ALIGN8((u32)&rodata->vertices[rodata->unk00 << 2])));
			gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL1, osVirtualToPhysical(rodata->baseaddr));

			gDPSetFogColor(renderdata->gdl++, 0x00, 0x00, 0x00, 0x00);
			gSPDisplayList(renderdata->gdl++, rodata->gdl);

			for (i = 0; i < rodata->unk00; i++) {
				u16 rand1 = (random() << 10) & 0xffff;
				s32 s4 = ((coss(rand1) << 5) * 181) >> 18;
				s32 s3 = ((sins(rand1) << 5) * 181) >> 18;
				s32 s1 = random() >> 31;
				s32 mult = 0x10000 - (random() & 0x3fff);
				s32 corner1 = 0x200 + s3;
				s32 corner2 = 0x200 - s3;
				s32 corner3 = 0x200 - s4;
				s32 corner4 = 0x200 + s4;

				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = src[3];

				dst[0].s = corner3;
				dst[0].t = corner2;
				dst[0].x = (src[(s1 + 0) % 4].x * mult) >> 16;
				dst[0].y = (src[(s1 + 0) % 4].y * mult) >> 16;
				dst[0].z = (src[(s1 + 0) % 4].z * mult) >> 16;

				dst[1].s = corner1;
				dst[1].t = corner3;
				dst[1].x = (src[(s1 + 1) % 4].x * mult) >> 16;
				dst[1].y = (src[(s1 + 1) % 4].y * mult) >> 16;
				dst[1].z = (src[(s1 + 1) % 4].z * mult) >> 16;

				dst[2].s = corner4;
				dst[2].t = corner1;
				dst[2].x = (src[(s1 + 2) % 4].x * mult) >> 16;
				dst[2].y = (src[(s1 + 2) % 4].y * mult) >> 16;
				dst[2].z = (src[(s1 + 2) % 4].z * mult) >> 16;

				dst[3].s = corner2;
				dst[3].t = corner4;
				dst[3].x = (src[(s1 + 3) % 4].x * mult) >> 16;
				dst[3].y = (src[(s1 + 3) % 4].y * mult) >> 16;
				dst[3].z = (src[(s1 + 3) % 4].z * mult) >> 16;

				src += 4;
				dst += 4;
			}
		}
	}
}

void model000216cc(struct modelrenderdata *renderdata, struct textureconfig *tconfig, s32 arg2)
{
	texSelect(&renderdata->gdl, tconfig, arg2, renderdata->zbufferenabled, 2, 1, NULL);
}

GLOBAL_ASM(
glabel modelRenderNodeChrGunfire
.late_rodata
glabel var70054454
.word 0x40c907a9
.text
/*    2170c:	27bdfef0 */ 	addiu	$sp,$sp,-272
/*    21710:	afb30020 */ 	sw	$s3,0x20($sp)
/*    21714:	00809825 */ 	or	$s3,$a0,$zero
/*    21718:	afbf0024 */ 	sw	$ra,0x24($sp)
/*    2171c:	afb2001c */ 	sw	$s2,0x1c($sp)
/*    21720:	afb10018 */ 	sw	$s1,0x18($sp)
/*    21724:	afb00014 */ 	sw	$s0,0x14($sp)
/*    21728:	afa50114 */ 	sw	$a1,0x114($sp)
/*    2172c:	00a02025 */ 	or	$a0,$a1,$zero
/*    21730:	00c08025 */ 	or	$s0,$a2,$zero
/*    21734:	8cd10004 */ 	lw	$s1,0x4($a2)
/*    21738:	0c006a87 */ 	jal	modelGetNodeRwData
/*    2173c:	00c02825 */ 	or	$a1,$a2,$zero
/*    21740:	3c198006 */ 	lui	$t9,%hi(var8005efec)
/*    21744:	2739efec */ 	addiu	$t9,$t9,%lo(var8005efec)
/*    21748:	8f210000 */ 	lw	$at,0x0($t9)
/*    2174c:	27ab0084 */ 	addiu	$t3,$sp,0x84
/*    21750:	8f380004 */ 	lw	$t8,0x4($t9)
/*    21754:	ad610000 */ 	sw	$at,0x0($t3)
/*    21758:	8f210008 */ 	lw	$at,0x8($t9)
/*    2175c:	3c0c8006 */ 	lui	$t4,%hi(var8005eff8)
/*    21760:	ad780004 */ 	sw	$t8,0x4($t3)
/*    21764:	ad610008 */ 	sw	$at,0x8($t3)
/*    21768:	8d8ceff8 */ 	lw	$t4,%lo(var8005eff8)($t4)
/*    2176c:	afac0080 */ 	sw	$t4,0x80($sp)
/*    21770:	8e6f0008 */ 	lw	$t7,0x8($s3)
/*    21774:	31ed0002 */ 	andi	$t5,$t7,0x2
/*    21778:	51a001d8 */ 	beqzl	$t5,.L00021edc
/*    2177c:	8fbf0024 */ 	lw	$ra,0x24($sp)
/*    21780:	844e0000 */ 	lh	$t6,0x0($v0)
/*    21784:	02002025 */ 	or	$a0,$s0,$zero
/*    21788:	51c001d4 */ 	beqzl	$t6,.L00021edc
/*    2178c:	8fbf0024 */ 	lw	$ra,0x24($sp)
/*    21790:	0c006949 */ 	jal	model0001a524
/*    21794:	00002825 */ 	or	$a1,$zero,$zero
/*    21798:	8fab0114 */ 	lw	$t3,0x114($sp)
/*    2179c:	0002c180 */ 	sll	$t8,$v0,0x6
/*    217a0:	c6280000 */ 	lwc1	$f8,0x0($s1)
/*    217a4:	8d79000c */ 	lw	$t9,0xc($t3)
/*    217a8:	c6240004 */ 	lwc1	$f4,0x4($s1)
/*    217ac:	03389021 */ 	addu	$s2,$t9,$t8
/*    217b0:	c64a0000 */ 	lwc1	$f10,0x0($s2)
/*    217b4:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*    217b8:	c6480010 */ 	lwc1	$f8,0x10($s2)
/*    217bc:	46082282 */ 	mul.s	$f10,$f4,$f8
/*    217c0:	c6280008 */ 	lwc1	$f8,0x8($s1)
/*    217c4:	460a3100 */ 	add.s	$f4,$f6,$f10
/*    217c8:	c6460020 */ 	lwc1	$f6,0x20($s2)
/*    217cc:	46064282 */ 	mul.s	$f10,$f8,$f6
/*    217d0:	c6460030 */ 	lwc1	$f6,0x30($s2)
/*    217d4:	460a2200 */ 	add.s	$f8,$f4,$f10
/*    217d8:	46083100 */ 	add.s	$f4,$f6,$f8
/*    217dc:	46002287 */ 	neg.s	$f10,$f4
/*    217e0:	e7aa00e0 */ 	swc1	$f10,0xe0($sp)
/*    217e4:	c6260000 */ 	lwc1	$f6,0x0($s1)
/*    217e8:	c6480004 */ 	lwc1	$f8,0x4($s2)
/*    217ec:	46083102 */ 	mul.s	$f4,$f6,$f8
/*    217f0:	c6480014 */ 	lwc1	$f8,0x14($s2)
/*    217f4:	c6260004 */ 	lwc1	$f6,0x4($s1)
/*    217f8:	46083182 */ 	mul.s	$f6,$f6,$f8
/*    217fc:	46062200 */ 	add.s	$f8,$f4,$f6
/*    21800:	c6460024 */ 	lwc1	$f6,0x24($s2)
/*    21804:	c6240008 */ 	lwc1	$f4,0x8($s1)
/*    21808:	46062102 */ 	mul.s	$f4,$f4,$f6
/*    2180c:	46044180 */ 	add.s	$f6,$f8,$f4
/*    21810:	c6480034 */ 	lwc1	$f8,0x34($s2)
/*    21814:	46064100 */ 	add.s	$f4,$f8,$f6
/*    21818:	46002207 */ 	neg.s	$f8,$f4
/*    2181c:	e7a800e4 */ 	swc1	$f8,0xe4($sp)
/*    21820:	c6440008 */ 	lwc1	$f4,0x8($s2)
/*    21824:	c6260000 */ 	lwc1	$f6,0x0($s1)
/*    21828:	46043182 */ 	mul.s	$f6,$f6,$f4
/*    2182c:	c6240004 */ 	lwc1	$f4,0x4($s1)
/*    21830:	e7aa0028 */ 	swc1	$f10,0x28($sp)
/*    21834:	c64a0018 */ 	lwc1	$f10,0x18($s2)
/*    21838:	460a2102 */ 	mul.s	$f4,$f4,$f10
/*    2183c:	46043280 */ 	add.s	$f10,$f6,$f4
/*    21840:	c6440028 */ 	lwc1	$f4,0x28($s2)
/*    21844:	c6260008 */ 	lwc1	$f6,0x8($s1)
/*    21848:	46043182 */ 	mul.s	$f6,$f6,$f4
/*    2184c:	46065100 */ 	add.s	$f4,$f10,$f6
/*    21850:	c64a0038 */ 	lwc1	$f10,0x38($s2)
/*    21854:	46045180 */ 	add.s	$f6,$f10,$f4
/*    21858:	c7a40028 */ 	lwc1	$f4,0x28($sp)
/*    2185c:	46003287 */ 	neg.s	$f10,$f6
/*    21860:	46042182 */ 	mul.s	$f6,$f4,$f4
/*    21864:	e7aa00e8 */ 	swc1	$f10,0xe8($sp)
/*    21868:	46084102 */ 	mul.s	$f4,$f8,$f8
/*    2186c:	46043200 */ 	add.s	$f8,$f6,$f4
/*    21870:	460a5282 */ 	mul.s	$f10,$f10,$f10
/*    21874:	0c012974 */ 	jal	sqrtf
/*    21878:	46085300 */ 	add.s	$f12,$f10,$f8
/*    2187c:	44807000 */ 	mtc1	$zero,$f14
/*    21880:	00000000 */ 	nop
/*    21884:	4600703c */ 	c.lt.s	$f14,$f0
/*    21888:	00000000 */ 	nop
/*    2188c:	45020012 */ 	bc1fl	.L000218d8
/*    21890:	8faf0114 */ 	lw	$t7,0x114($sp)
/*    21894:	8fac0114 */ 	lw	$t4,0x114($sp)
/*    21898:	3c013f80 */ 	lui	$at,0x3f80
/*    2189c:	44813000 */ 	mtc1	$at,$f6
/*    218a0:	c5840014 */ 	lwc1	$f4,0x14($t4)
/*    218a4:	c7a800e0 */ 	lwc1	$f8,0xe0($sp)
/*    218a8:	46002282 */ 	mul.s	$f10,$f4,$f0
/*    218ac:	460a3083 */ 	div.s	$f2,$f6,$f10
/*    218b0:	c7a600e4 */ 	lwc1	$f6,0xe4($sp)
/*    218b4:	46024102 */ 	mul.s	$f4,$f8,$f2
/*    218b8:	c7a800e8 */ 	lwc1	$f8,0xe8($sp)
/*    218bc:	46023282 */ 	mul.s	$f10,$f6,$f2
/*    218c0:	e7a400e0 */ 	swc1	$f4,0xe0($sp)
/*    218c4:	46024102 */ 	mul.s	$f4,$f8,$f2
/*    218c8:	e7aa00e4 */ 	swc1	$f10,0xe4($sp)
/*    218cc:	10000009 */ 	b	.L000218f4
/*    218d0:	e7a400e8 */ 	swc1	$f4,0xe8($sp)
/*    218d4:	8faf0114 */ 	lw	$t7,0x114($sp)
.L000218d8:
/*    218d8:	e7ae00e0 */ 	swc1	$f14,0xe0($sp)
/*    218dc:	e7ae00e4 */ 	swc1	$f14,0xe4($sp)
/*    218e0:	3c013f80 */ 	lui	$at,0x3f80
/*    218e4:	44813000 */ 	mtc1	$at,$f6
/*    218e8:	c5ea0014 */ 	lwc1	$f10,0x14($t7)
/*    218ec:	460a3203 */ 	div.s	$f8,$f6,$f10
/*    218f0:	e7a800e8 */ 	swc1	$f8,0xe8($sp)
.L000218f4:
/*    218f4:	c7a400e0 */ 	lwc1	$f4,0xe0($sp)
/*    218f8:	c6460010 */ 	lwc1	$f6,0x10($s2)
/*    218fc:	c7a800e4 */ 	lwc1	$f8,0xe4($sp)
/*    21900:	46062282 */ 	mul.s	$f10,$f4,$f6
/*    21904:	c6440014 */ 	lwc1	$f4,0x14($s2)
/*    21908:	46044182 */ 	mul.s	$f6,$f8,$f4
/*    2190c:	c6440018 */ 	lwc1	$f4,0x18($s2)
/*    21910:	46065200 */ 	add.s	$f8,$f10,$f6
/*    21914:	c7aa00e8 */ 	lwc1	$f10,0xe8($sp)
/*    21918:	460a2182 */ 	mul.s	$f6,$f4,$f10
/*    2191c:	0fc25a74 */ 	jal	acosf
/*    21920:	46083300 */ 	add.s	$f12,$f6,$f8
/*    21924:	e7a000ec */ 	swc1	$f0,0xec($sp)
/*    21928:	0c0068f7 */ 	jal	sinf
/*    2192c:	46000306 */ 	mov.s	$f12,$f0
/*    21930:	c7a400e0 */ 	lwc1	$f4,0xe0($sp)
/*    21934:	c64a0020 */ 	lwc1	$f10,0x20($s2)
/*    21938:	c7a800e4 */ 	lwc1	$f8,0xe4($sp)
/*    2193c:	460a2182 */ 	mul.s	$f6,$f4,$f10
/*    21940:	c6440024 */ 	lwc1	$f4,0x24($s2)
/*    21944:	46044282 */ 	mul.s	$f10,$f8,$f4
/*    21948:	c7a400e8 */ 	lwc1	$f4,0xe8($sp)
/*    2194c:	460a3200 */ 	add.s	$f8,$f6,$f10
/*    21950:	c6460028 */ 	lwc1	$f6,0x28($s2)
/*    21954:	46062282 */ 	mul.s	$f10,$f4,$f6
/*    21958:	460a4100 */ 	add.s	$f4,$f8,$f10
/*    2195c:	46002187 */ 	neg.s	$f6,$f4
/*    21960:	0fc25a74 */ 	jal	acosf
/*    21964:	46003303 */ 	div.s	$f12,$f6,$f0
/*    21968:	c7a800e0 */ 	lwc1	$f8,0xe0($sp)
/*    2196c:	c64a0000 */ 	lwc1	$f10,0x0($s2)
/*    21970:	c7a600e4 */ 	lwc1	$f6,0xe4($sp)
/*    21974:	46000306 */ 	mov.s	$f12,$f0
/*    21978:	460a4102 */ 	mul.s	$f4,$f8,$f10
/*    2197c:	c6480004 */ 	lwc1	$f8,0x4($s2)
/*    21980:	46083282 */ 	mul.s	$f10,$f6,$f8
/*    21984:	c6480008 */ 	lwc1	$f8,0x8($s2)
/*    21988:	460a2180 */ 	add.s	$f6,$f4,$f10
/*    2198c:	c7a400e8 */ 	lwc1	$f4,0xe8($sp)
/*    21990:	46044282 */ 	mul.s	$f10,$f8,$f4
/*    21994:	44804000 */ 	mtc1	$zero,$f8
/*    21998:	46065080 */ 	add.s	$f2,$f10,$f6
/*    2199c:	46001087 */ 	neg.s	$f2,$f2
/*    219a0:	4608103c */ 	c.lt.s	$f2,$f8
/*    219a4:	00000000 */ 	nop
/*    219a8:	45000003 */ 	bc1f	.L000219b8
/*    219ac:	3c017005 */ 	lui	$at,%hi(var70054454)
/*    219b0:	c4244454 */ 	lwc1	$f4,%lo(var70054454)($at)
/*    219b4:	46002301 */ 	sub.s	$f12,$f4,$f0
.L000219b8:
/*    219b8:	0c0068f4 */ 	jal	cosf
/*    219bc:	e7ac00f0 */ 	swc1	$f12,0xf0($sp)
/*    219c0:	c7ac00f0 */ 	lwc1	$f12,0xf0($sp)
/*    219c4:	0c0068f7 */ 	jal	sinf
/*    219c8:	e7a000dc */ 	swc1	$f0,0xdc($sp)
/*    219cc:	e7a000d8 */ 	swc1	$f0,0xd8($sp)
/*    219d0:	0c0068f4 */ 	jal	cosf
/*    219d4:	c7ac00ec */ 	lwc1	$f12,0xec($sp)
/*    219d8:	e7a000d4 */ 	swc1	$f0,0xd4($sp)
/*    219dc:	0c0068f7 */ 	jal	sinf
/*    219e0:	c7ac00ec */ 	lwc1	$f12,0xec($sp)
/*    219e4:	0c004b70 */ 	jal	random
/*    219e8:	e7a000d0 */ 	swc1	$f0,0xd0($sp)
/*    219ec:	304d007f */ 	andi	$t5,$v0,0x7f
/*    219f0:	448d5000 */ 	mtc1	$t5,$f10
/*    219f4:	3c013f00 */ 	lui	$at,0x3f00
/*    219f8:	44810000 */ 	mtc1	$at,$f0
/*    219fc:	c7ae00dc */ 	lwc1	$f14,0xdc($sp)
/*    21a00:	c7b000d8 */ 	lwc1	$f16,0xd8($sp)
/*    21a04:	c7b200d4 */ 	lwc1	$f18,0xd4($sp)
/*    21a08:	05a10005 */ 	bgez	$t5,.L00021a20
/*    21a0c:	468051a0 */ 	cvt.s.w	$f6,$f10
/*    21a10:	3c014f80 */ 	lui	$at,0x4f80
/*    21a14:	44814000 */ 	mtc1	$at,$f8
/*    21a18:	00000000 */ 	nop
/*    21a1c:	46083180 */ 	add.s	$f6,$f6,$f8
.L00021a20:
/*    21a20:	3c013b80 */ 	lui	$at,0x3b80
/*    21a24:	44812000 */ 	mtc1	$at,$f4
/*    21a28:	3c013f40 */ 	lui	$at,0x3f40
/*    21a2c:	44814000 */ 	mtc1	$at,$f8
/*    21a30:	46043282 */ 	mul.s	$f10,$f6,$f4
/*    21a34:	c626000c */ 	lwc1	$f6,0xc($s1)
/*    21a38:	3c198006 */ 	lui	$t9,%hi(g_ModelVtxAllocatorFunc)
/*    21a3c:	8f39efc8 */ 	lw	$t9,%lo(g_ModelVtxAllocatorFunc)($t9)
/*    21a40:	24040004 */ 	addiu	$a0,$zero,0x4
/*    21a44:	46085080 */ 	add.s	$f2,$f10,$f8
/*    21a48:	46023102 */ 	mul.s	$f4,$f6,$f2
/*    21a4c:	e7a4009c */ 	swc1	$f4,0x9c($sp)
/*    21a50:	c62a0010 */ 	lwc1	$f10,0x10($s1)
/*    21a54:	46025202 */ 	mul.s	$f8,$f10,$f2
/*    21a58:	c7aa009c */ 	lwc1	$f10,0x9c($sp)
/*    21a5c:	e7a800a0 */ 	swc1	$f8,0xa0($sp)
/*    21a60:	c6260014 */ 	lwc1	$f6,0x14($s1)
/*    21a64:	46023102 */ 	mul.s	$f4,$f6,$f2
/*    21a68:	00000000 */ 	nop
/*    21a6c:	460e5202 */ 	mul.s	$f8,$f10,$f14
/*    21a70:	e7a400a4 */ 	swc1	$f4,0xa4($sp)
/*    21a74:	c7a400a4 */ 	lwc1	$f4,0xa4($sp)
/*    21a78:	46004182 */ 	mul.s	$f6,$f8,$f0
/*    21a7c:	00000000 */ 	nop
/*    21a80:	46102202 */ 	mul.s	$f8,$f4,$f16
/*    21a84:	e7a600cc */ 	swc1	$f6,0xcc($sp)
/*    21a88:	46004182 */ 	mul.s	$f6,$f8,$f0
/*    21a8c:	c7a800a0 */ 	lwc1	$f8,0xa0($sp)
/*    21a90:	e7a600c8 */ 	swc1	$f6,0xc8($sp)
/*    21a94:	c7a600d0 */ 	lwc1	$f6,0xd0($sp)
/*    21a98:	46064202 */ 	mul.s	$f8,$f8,$f6
/*    21a9c:	00000000 */ 	nop
/*    21aa0:	46004302 */ 	mul.s	$f12,$f8,$f0
/*    21aa4:	00000000 */ 	nop
/*    21aa8:	46125182 */ 	mul.s	$f6,$f10,$f18
/*    21aac:	00000000 */ 	nop
/*    21ab0:	46103202 */ 	mul.s	$f8,$f6,$f16
/*    21ab4:	00000000 */ 	nop
/*    21ab8:	46004182 */ 	mul.s	$f6,$f8,$f0
/*    21abc:	00000000 */ 	nop
/*    21ac0:	46122202 */ 	mul.s	$f8,$f4,$f18
/*    21ac4:	e7a600c0 */ 	swc1	$f6,0xc0($sp)
/*    21ac8:	460e4182 */ 	mul.s	$f6,$f8,$f14
/*    21acc:	00000000 */ 	nop
/*    21ad0:	46003102 */ 	mul.s	$f4,$f6,$f0
/*    21ad4:	e7a400bc */ 	swc1	$f4,0xbc($sp)
/*    21ad8:	46005182 */ 	mul.s	$f6,$f10,$f0
/*    21adc:	c6280000 */ 	lwc1	$f8,0x0($s1)
/*    21ae0:	46064101 */ 	sub.s	$f4,$f8,$f6
/*    21ae4:	e7a40090 */ 	swc1	$f4,0x90($sp)
/*    21ae8:	c62a0004 */ 	lwc1	$f10,0x4($s1)
/*    21aec:	e7aa0094 */ 	swc1	$f10,0x94($sp)
/*    21af0:	c6280008 */ 	lwc1	$f8,0x8($s1)
/*    21af4:	e7ac00c4 */ 	swc1	$f12,0xc4($sp)
/*    21af8:	0320f809 */ 	jalr	$t9
/*    21afc:	e7a80098 */ 	swc1	$f8,0x98($sp)
/*    21b00:	00408025 */ 	or	$s0,$v0,$zero
/*    21b04:	0fc59e73 */ 	jal	gfxAllocateColours
/*    21b08:	24040001 */ 	addiu	$a0,$zero,0x1
/*    21b0c:	27a30084 */ 	addiu	$v1,$sp,0x84
/*    21b10:	afa20078 */ 	sw	$v0,0x78($sp)
/*    21b14:	8c610000 */ 	lw	$at,0x0($v1)
/*    21b18:	c7ac00c4 */ 	lwc1	$f12,0xc4($sp)
/*    21b1c:	aa010000 */ 	swl	$at,0x0($s0)
/*    21b20:	ba010003 */ 	swr	$at,0x3($s0)
/*    21b24:	8c780004 */ 	lw	$t8,0x4($v1)
/*    21b28:	aa180004 */ 	swl	$t8,0x4($s0)
/*    21b2c:	ba180007 */ 	swr	$t8,0x7($s0)
/*    21b30:	8c610008 */ 	lw	$at,0x8($v1)
/*    21b34:	aa010008 */ 	swl	$at,0x8($s0)
/*    21b38:	ba01000b */ 	swr	$at,0xb($s0)
/*    21b3c:	8c610000 */ 	lw	$at,0x0($v1)
/*    21b40:	aa01000c */ 	swl	$at,0xc($s0)
/*    21b44:	ba01000f */ 	swr	$at,0xf($s0)
/*    21b48:	8c6d0004 */ 	lw	$t5,0x4($v1)
/*    21b4c:	aa0d0010 */ 	swl	$t5,0x10($s0)
/*    21b50:	ba0d0013 */ 	swr	$t5,0x13($s0)
/*    21b54:	8c610008 */ 	lw	$at,0x8($v1)
/*    21b58:	27ad0080 */ 	addiu	$t5,$sp,0x80
/*    21b5c:	aa010014 */ 	swl	$at,0x14($s0)
/*    21b60:	ba010017 */ 	swr	$at,0x17($s0)
/*    21b64:	8c610000 */ 	lw	$at,0x0($v1)
/*    21b68:	aa010018 */ 	swl	$at,0x18($s0)
/*    21b6c:	ba01001b */ 	swr	$at,0x1b($s0)
/*    21b70:	8c6e0004 */ 	lw	$t6,0x4($v1)
/*    21b74:	aa0e001c */ 	swl	$t6,0x1c($s0)
/*    21b78:	ba0e001f */ 	swr	$t6,0x1f($s0)
/*    21b7c:	8c610008 */ 	lw	$at,0x8($v1)
/*    21b80:	aa010020 */ 	swl	$at,0x20($s0)
/*    21b84:	ba010023 */ 	swr	$at,0x23($s0)
/*    21b88:	8c610000 */ 	lw	$at,0x0($v1)
/*    21b8c:	aa010024 */ 	swl	$at,0x24($s0)
/*    21b90:	ba010027 */ 	swr	$at,0x27($s0)
/*    21b94:	8c6c0004 */ 	lw	$t4,0x4($v1)
/*    21b98:	aa0c0028 */ 	swl	$t4,0x28($s0)
/*    21b9c:	ba0c002b */ 	swr	$t4,0x2b($s0)
/*    21ba0:	8c610008 */ 	lw	$at,0x8($v1)
/*    21ba4:	aa01002c */ 	swl	$at,0x2c($s0)
/*    21ba8:	ba01002f */ 	swr	$at,0x2f($s0)
/*    21bac:	8da10000 */ 	lw	$at,0x0($t5)
/*    21bb0:	ac410000 */ 	sw	$at,0x0($v0)
/*    21bb4:	c7a000cc */ 	lwc1	$f0,0xcc($sp)
/*    21bb8:	c7a60090 */ 	lwc1	$f6,0x90($sp)
/*    21bbc:	c7ae00c0 */ 	lwc1	$f14,0xc0($sp)
/*    21bc0:	46000007 */ 	neg.s	$f0,$f0
/*    21bc4:	46003100 */ 	add.s	$f4,$f6,$f0
/*    21bc8:	46007387 */ 	neg.s	$f14,$f14
/*    21bcc:	460e2280 */ 	add.s	$f10,$f4,$f14
/*    21bd0:	4600520d */ 	trunc.w.s	$f8,$f10
/*    21bd4:	44184000 */ 	mfc1	$t8,$f8
/*    21bd8:	00000000 */ 	nop
/*    21bdc:	a6180000 */ 	sh	$t8,0x0($s0)
/*    21be0:	c7a60094 */ 	lwc1	$f6,0x94($sp)
/*    21be4:	460c3101 */ 	sub.s	$f4,$f6,$f12
/*    21be8:	4600228d */ 	trunc.w.s	$f10,$f4
/*    21bec:	44195000 */ 	mfc1	$t9,$f10
/*    21bf0:	00000000 */ 	nop
/*    21bf4:	a6190002 */ 	sh	$t9,0x2($s0)
/*    21bf8:	c7a200c8 */ 	lwc1	$f2,0xc8($sp)
/*    21bfc:	c7a80098 */ 	lwc1	$f8,0x98($sp)
/*    21c00:	c7b000bc */ 	lwc1	$f16,0xbc($sp)
/*    21c04:	46001087 */ 	neg.s	$f2,$f2
/*    21c08:	46024181 */ 	sub.s	$f6,$f8,$f2
/*    21c0c:	46008407 */ 	neg.s	$f16,$f16
/*    21c10:	46103100 */ 	add.s	$f4,$f6,$f16
/*    21c14:	4600228d */ 	trunc.w.s	$f10,$f4
/*    21c18:	440d5000 */ 	mfc1	$t5,$f10
/*    21c1c:	00000000 */ 	nop
/*    21c20:	a60d0004 */ 	sh	$t5,0x4($s0)
/*    21c24:	c7a80090 */ 	lwc1	$f8,0x90($sp)
/*    21c28:	46004180 */ 	add.s	$f6,$f8,$f0
/*    21c2c:	460e3101 */ 	sub.s	$f4,$f6,$f14
/*    21c30:	4600228d */ 	trunc.w.s	$f10,$f4
/*    21c34:	440f5000 */ 	mfc1	$t7,$f10
/*    21c38:	00000000 */ 	nop
/*    21c3c:	a60f000c */ 	sh	$t7,0xc($s0)
/*    21c40:	c7a80094 */ 	lwc1	$f8,0x94($sp)
/*    21c44:	460c4180 */ 	add.s	$f6,$f8,$f12
/*    21c48:	4600310d */ 	trunc.w.s	$f4,$f6
/*    21c4c:	440c2000 */ 	mfc1	$t4,$f4
/*    21c50:	00000000 */ 	nop
/*    21c54:	a60c000e */ 	sh	$t4,0xe($s0)
/*    21c58:	c7aa0098 */ 	lwc1	$f10,0x98($sp)
/*    21c5c:	46025201 */ 	sub.s	$f8,$f10,$f2
/*    21c60:	46104181 */ 	sub.s	$f6,$f8,$f16
/*    21c64:	4600310d */ 	trunc.w.s	$f4,$f6
/*    21c68:	440b2000 */ 	mfc1	$t3,$f4
/*    21c6c:	00000000 */ 	nop
/*    21c70:	a60b0010 */ 	sh	$t3,0x10($s0)
/*    21c74:	c7aa0090 */ 	lwc1	$f10,0x90($sp)
/*    21c78:	3c0bbc00 */ 	lui	$t3,0xbc00
/*    21c7c:	356b1406 */ 	ori	$t3,$t3,0x1406
/*    21c80:	46005201 */ 	sub.s	$f8,$f10,$f0
/*    21c84:	460e4181 */ 	sub.s	$f6,$f8,$f14
/*    21c88:	4600310d */ 	trunc.w.s	$f4,$f6
/*    21c8c:	440e2000 */ 	mfc1	$t6,$f4
/*    21c90:	00000000 */ 	nop
/*    21c94:	a60e0018 */ 	sh	$t6,0x18($s0)
/*    21c98:	c7aa0094 */ 	lwc1	$f10,0x94($sp)
/*    21c9c:	460c5200 */ 	add.s	$f8,$f10,$f12
/*    21ca0:	4600418d */ 	trunc.w.s	$f6,$f8
/*    21ca4:	44183000 */ 	mfc1	$t8,$f6
/*    21ca8:	00000000 */ 	nop
/*    21cac:	a618001a */ 	sh	$t8,0x1a($s0)
/*    21cb0:	c7a40098 */ 	lwc1	$f4,0x98($sp)
/*    21cb4:	46022280 */ 	add.s	$f10,$f4,$f2
/*    21cb8:	46105201 */ 	sub.s	$f8,$f10,$f16
/*    21cbc:	4600418d */ 	trunc.w.s	$f6,$f8
/*    21cc0:	44193000 */ 	mfc1	$t9,$f6
/*    21cc4:	00000000 */ 	nop
/*    21cc8:	a619001c */ 	sh	$t9,0x1c($s0)
/*    21ccc:	c7a40090 */ 	lwc1	$f4,0x90($sp)
/*    21cd0:	46002281 */ 	sub.s	$f10,$f4,$f0
/*    21cd4:	460e5200 */ 	add.s	$f8,$f10,$f14
/*    21cd8:	4600418d */ 	trunc.w.s	$f6,$f8
/*    21cdc:	440d3000 */ 	mfc1	$t5,$f6
/*    21ce0:	00000000 */ 	nop
/*    21ce4:	a60d0024 */ 	sh	$t5,0x24($s0)
/*    21ce8:	c7a40094 */ 	lwc1	$f4,0x94($sp)
/*    21cec:	460c2281 */ 	sub.s	$f10,$f4,$f12
/*    21cf0:	4600520d */ 	trunc.w.s	$f8,$f10
/*    21cf4:	440f4000 */ 	mfc1	$t7,$f8
/*    21cf8:	00000000 */ 	nop
/*    21cfc:	a60f0026 */ 	sh	$t7,0x26($s0)
/*    21d00:	c7a60098 */ 	lwc1	$f6,0x98($sp)
/*    21d04:	46023100 */ 	add.s	$f4,$f6,$f2
/*    21d08:	46102280 */ 	add.s	$f10,$f4,$f16
/*    21d0c:	4600520d */ 	trunc.w.s	$f8,$f10
/*    21d10:	440c4000 */ 	mfc1	$t4,$f8
/*    21d14:	00000000 */ 	nop
/*    21d18:	a60c0028 */ 	sh	$t4,0x28($s0)
/*    21d1c:	8e63000c */ 	lw	$v1,0xc($s3)
/*    21d20:	24790008 */ 	addiu	$t9,$v1,0x8
/*    21d24:	ae79000c */ 	sw	$t9,0xc($s3)
/*    21d28:	ac6b0000 */ 	sw	$t3,0x0($v1)
/*    21d2c:	8e240024 */ 	lw	$a0,0x24($s1)
/*    21d30:	0c012d20 */ 	jal	osVirtualToPhysical
/*    21d34:	afa30068 */ 	sw	$v1,0x68($sp)
/*    21d38:	8fa50068 */ 	lw	$a1,0x68($sp)
/*    21d3c:	02602025 */ 	or	$a0,$s3,$zero
/*    21d40:	aca20004 */ 	sw	$v0,0x4($a1)
/*    21d44:	8e230018 */ 	lw	$v1,0x18($s1)
/*    21d48:	00002825 */ 	or	$a1,$zero,$zero
/*    21d4c:	10600036 */ 	beqz	$v1,.L00021e28
/*    21d50:	00000000 */ 	nop
/*    21d54:	0c004b70 */ 	jal	random
/*    21d58:	00608825 */ 	or	$s1,$v1,$zero
/*    21d5c:	00026a80 */ 	sll	$t5,$v0,0xa
/*    21d60:	a7ad0062 */ 	sh	$t5,0x62($sp)
/*    21d64:	0c013ef0 */ 	jal	coss
/*    21d68:	31a4ffff */ 	andi	$a0,$t5,0xffff
/*    21d6c:	922e0004 */ 	lbu	$t6,0x4($s1)
/*    21d70:	97a40062 */ 	lhu	$a0,0x62($sp)
/*    21d74:	004e0019 */ 	multu	$v0,$t6
/*    21d78:	00004012 */ 	mflo	$t0
/*    21d7c:	00087880 */ 	sll	$t7,$t0,0x2
/*    21d80:	01e87823 */ 	subu	$t7,$t7,$t0
/*    21d84:	000f7880 */ 	sll	$t7,$t7,0x2
/*    21d88:	01e87823 */ 	subu	$t7,$t7,$t0
/*    21d8c:	000f7880 */ 	sll	$t7,$t7,0x2
/*    21d90:	01e87821 */ 	addu	$t7,$t7,$t0
/*    21d94:	000f7880 */ 	sll	$t7,$t7,0x2
/*    21d98:	01e87821 */ 	addu	$t7,$t7,$t0
/*    21d9c:	000fc483 */ 	sra	$t8,$t7,0x12
/*    21da0:	0c013efc */ 	jal	sins
/*    21da4:	afb8005c */ 	sw	$t8,0x5c($sp)
/*    21da8:	92240004 */ 	lbu	$a0,0x4($s1)
/*    21dac:	8fa8005c */ 	lw	$t0,0x5c($sp)
/*    21db0:	00440019 */ 	multu	$v0,$a0
/*    21db4:	00041900 */ 	sll	$v1,$a0,0x4
/*    21db8:	00683023 */ 	subu	$a2,$v1,$t0
/*    21dbc:	a6060008 */ 	sh	$a2,0x8($s0)
/*    21dc0:	a6060016 */ 	sh	$a2,0x16($s0)
/*    21dc4:	00685021 */ 	addu	$t2,$v1,$t0
/*    21dc8:	a60a0020 */ 	sh	$t2,0x20($s0)
/*    21dcc:	a60a002e */ 	sh	$t2,0x2e($s0)
/*    21dd0:	24060004 */ 	addiu	$a2,$zero,0x4
/*    21dd4:	02602025 */ 	or	$a0,$s3,$zero
/*    21dd8:	00002812 */ 	mflo	$a1
/*    21ddc:	00056080 */ 	sll	$t4,$a1,0x2
/*    21de0:	01856023 */ 	subu	$t4,$t4,$a1
/*    21de4:	000c6080 */ 	sll	$t4,$t4,0x2
/*    21de8:	01856023 */ 	subu	$t4,$t4,$a1
/*    21dec:	000c6080 */ 	sll	$t4,$t4,0x2
/*    21df0:	01856021 */ 	addu	$t4,$t4,$a1
/*    21df4:	000c6080 */ 	sll	$t4,$t4,0x2
/*    21df8:	01856021 */ 	addu	$t4,$t4,$a1
/*    21dfc:	000ccc83 */ 	sra	$t9,$t4,0x12
/*    21e00:	00794823 */ 	subu	$t1,$v1,$t9
/*    21e04:	00793821 */ 	addu	$a3,$v1,$t9
/*    21e08:	a609000a */ 	sh	$t1,0xa($s0)
/*    21e0c:	a6070014 */ 	sh	$a3,0x14($s0)
/*    21e10:	a6070022 */ 	sh	$a3,0x22($s0)
/*    21e14:	a609002c */ 	sh	$t1,0x2c($s0)
/*    21e18:	0c0085b3 */ 	jal	model000216cc
/*    21e1c:	02202825 */ 	or	$a1,$s1,$zero
/*    21e20:	10000004 */ 	b	.L00021e34
/*    21e24:	8e63000c */ 	lw	$v1,0xc($s3)
.L00021e28:
/*    21e28:	0c0085b3 */ 	jal	model000216cc
/*    21e2c:	24060001 */ 	addiu	$a2,$zero,0x1
/*    21e30:	8e63000c */ 	lw	$v1,0xc($s3)
.L00021e34:
/*    21e34:	3c0db700 */ 	lui	$t5,0xb700
/*    21e38:	240e2000 */ 	addiu	$t6,$zero,0x2000
/*    21e3c:	246b0008 */ 	addiu	$t3,$v1,0x8
/*    21e40:	ae6b000c */ 	sw	$t3,0xc($s3)
/*    21e44:	ac6e0004 */ 	sw	$t6,0x4($v1)
/*    21e48:	ac6d0000 */ 	sw	$t5,0x0($v1)
/*    21e4c:	8e71000c */ 	lw	$s1,0xc($s3)
/*    21e50:	3c180102 */ 	lui	$t8,0x102
/*    21e54:	37180040 */ 	ori	$t8,$t8,0x40
/*    21e58:	262f0008 */ 	addiu	$t7,$s1,0x8
/*    21e5c:	ae6f000c */ 	sw	$t7,0xc($s3)
/*    21e60:	02402025 */ 	or	$a0,$s2,$zero
/*    21e64:	0c012d20 */ 	jal	osVirtualToPhysical
/*    21e68:	ae380000 */ 	sw	$t8,0x0($s1)
/*    21e6c:	ae220004 */ 	sw	$v0,0x4($s1)
/*    21e70:	8e72000c */ 	lw	$s2,0xc($s3)
/*    21e74:	3c190700 */ 	lui	$t9,0x700
/*    21e78:	37390004 */ 	ori	$t9,$t9,0x4
/*    21e7c:	264c0008 */ 	addiu	$t4,$s2,0x8
/*    21e80:	ae6c000c */ 	sw	$t4,0xc($s3)
/*    21e84:	ae590000 */ 	sw	$t9,0x0($s2)
/*    21e88:	0c012d20 */ 	jal	osVirtualToPhysical
/*    21e8c:	8fa40078 */ 	lw	$a0,0x78($sp)
/*    21e90:	ae420004 */ 	sw	$v0,0x4($s2)
/*    21e94:	8e71000c */ 	lw	$s1,0xc($s3)
/*    21e98:	3c0d0430 */ 	lui	$t5,0x430
/*    21e9c:	35ad0030 */ 	ori	$t5,$t5,0x30
/*    21ea0:	262b0008 */ 	addiu	$t3,$s1,0x8
/*    21ea4:	ae6b000c */ 	sw	$t3,0xc($s3)
/*    21ea8:	02002025 */ 	or	$a0,$s0,$zero
/*    21eac:	0c012d20 */ 	jal	osVirtualToPhysical
/*    21eb0:	ae2d0000 */ 	sw	$t5,0x0($s1)
/*    21eb4:	ae220004 */ 	sw	$v0,0x4($s1)
/*    21eb8:	8e63000c */ 	lw	$v1,0xc($s3)
/*    21ebc:	3c0fb100 */ 	lui	$t7,0xb100
/*    21ec0:	35ef0002 */ 	ori	$t7,$t7,0x2
/*    21ec4:	246e0008 */ 	addiu	$t6,$v1,0x8
/*    21ec8:	ae6e000c */ 	sw	$t6,0xc($s3)
/*    21ecc:	24183210 */ 	addiu	$t8,$zero,0x3210
/*    21ed0:	ac780004 */ 	sw	$t8,0x4($v1)
/*    21ed4:	ac6f0000 */ 	sw	$t7,0x0($v1)
/*    21ed8:	8fbf0024 */ 	lw	$ra,0x24($sp)
.L00021edc:
/*    21edc:	8fb00014 */ 	lw	$s0,0x14($sp)
/*    21ee0:	8fb10018 */ 	lw	$s1,0x18($sp)
/*    21ee4:	8fb2001c */ 	lw	$s2,0x1c($sp)
/*    21ee8:	8fb30020 */ 	lw	$s3,0x20($sp)
/*    21eec:	03e00008 */ 	jr	$ra
/*    21ef0:	27bd0110 */ 	addiu	$sp,$sp,0x110
);

//struct tmpcolour {
//	u32 value;
//};

// Mismatch: Reordered instructions near assign to spcc
//void modelRenderNodeChrGunfire(struct modelrenderdata *renderdata, struct model *model, struct modelnode *node)
//{
//	struct modelrodata_chrgunfire *rodata = &node->rodata->gunfire;
//	union modelrwdata *rwdata = modelGetNodeRwData(model, node);
//	struct gfxvtx *vertices;
//	f32 spf0;
//	f32 spec;
//	struct coord spe0;
//	f32 spdc;
//	f32 spd8;
//	f32 spd4;
//	f32 spd0;
//	f32 spcc;
//	f32 spc8;
//	f32 spc4;
//	f32 spc0;
//	f32 spbc;
//	struct coord sp9c;
//	struct coord sp90;
//	struct gfxvtx vtxtemplate = {0}; // 84, 88, 8c
//	struct tmpcolour colourtemplate = {0xffffffff}; // 80
//	f32 scale;
//	struct tmpcolour *colours;
//	struct textureconfig *texture;
//	Mtxf *mtx;
//	f32 tmp;
//	f32 distance;
//	u16 sp62;
//	s32 sp5c;
//	s32 sp58;
//	s32 centre;
//	u32 stack[6];
//
//	// 778
//	if ((renderdata->flags & 2) && rwdata->gunfire.visible) {
//		mtx = &model->matrices[model0001a524(node, 0)];
//
//		spe0.x = -(rodata->pos.f[0] * mtx->m[0][0] + rodata->pos.f[1] * mtx->m[1][0] + rodata->pos.f[2] * mtx->m[2][0] + mtx->m[3][0]);
//		spe0.y = -(rodata->pos.f[0] * mtx->m[0][1] + rodata->pos.f[1] * mtx->m[1][1] + rodata->pos.f[2] * mtx->m[2][1] + mtx->m[3][1]);
//		spe0.z = -(rodata->pos.f[0] * mtx->m[0][2] + rodata->pos.f[1] * mtx->m[1][2] + rodata->pos.f[2] * mtx->m[2][2] + mtx->m[3][2]);
//
//		distance = sqrtf(spe0.f[0] * spe0.f[0] + spe0.f[1] * spe0.f[1] + spe0.f[2] * spe0.f[2]);
//
//		// 88c
//		if (distance > 0) {
//			f32 tmp = 1 / (model->scale * distance);
//			spe0.f[0] *= tmp;
//			spe0.f[1] *= tmp;
//			spe0.f[2] *= tmp;
//		} else {
//			spe0.f[0] = 0;
//			spe0.f[1] = 0;
//			spe0.f[2] = 1 / model->scale;
//		}
//
//		// 8f4
//		spec = acosf(spe0.f[0] * mtx->m[1][0] + spe0.f[1] * mtx->m[1][1] + spe0.f[2] * mtx->m[1][2]);
//		spf0 = acosf(-(spe0.f[0] * mtx->m[2][0] + spe0.f[1] * mtx->m[2][1] + spe0.f[2] * mtx->m[2][2]) / sinf(spec));
//
//		tmp = -(spe0.f[0] * mtx->m[0][0] + spe0.f[1] * mtx->m[0][1] + spe0.f[2] * mtx->m[0][2]);
//
//		// 9a8
//		if (tmp < 0) {
//			spf0 = M_BADTAU - spf0;
//		}
//
//		// 9b8
//		spdc = cosf(spf0);
//		spd8 = sinf(spf0);
//		spd4 = cosf(spec);
//		spd0 = sinf(spec);
//
//		scale = 0.75f + (random() % 128) * (1.0f / 256.0f); // 0.75 to 1.25
//
//		sp9c.f[0] = rodata->dim.f[0] * scale;
//		sp9c.f[1] = rodata->dim.f[1] * scale;
//		sp9c.f[2] = rodata->dim.f[2] * scale;
//
//		spcc = sp9c.f[0] * spdc * 0.5f;
//		spc8 = sp9c.f[1] * spd8 * 0.5f;
//		spc4 = sp9c.f[2] * spd0 * 0.5f;
//
//		spc0 = sp9c.f[0] * spd4 * spd8 * 0.5f;
//		spbc = sp9c.f[2] * spd4 * spdc * 0.5f;
//
//		sp90.f[0] = rodata->pos.f[0] - sp9c.f[0] * 0.5f;
//		sp90.f[1] = rodata->pos.f[1];
//		sp90.f[2] = rodata->pos.f[2];
//
//		vertices = g_ModelVtxAllocatorFunc(4);
//
//		// b04
//		colours = (struct tmpcolour *) gfxAllocateColours(1);
//
//		vertices[0] = vtxtemplate;
//		vertices[1] = vtxtemplate;
//		vertices[2] = vtxtemplate;
//		vertices[3] = vtxtemplate;
//
//		colours[0] = colourtemplate;
//
//		vertices[0].x = sp90.f[0] + -spcc + -spc0;
//		vertices[0].y = sp90.f[1] - spc4;
//		vertices[0].z = sp90.f[2] - -spc8 + -spbc;
//		vertices[1].x = sp90.f[0] + -spcc - -spc0;
//		vertices[1].y = sp90.f[1] + spc4;
//		vertices[1].z = sp90.f[2] - -spc8 - -spbc;
//		vertices[2].x = sp90.f[0] - -spcc - -spc0;
//		vertices[2].y = sp90.f[1] + spc4;
//		vertices[2].z = sp90.f[2] + -spc8 - -spbc;
//		vertices[3].x = sp90.f[0] - -spcc + -spc0;
//		vertices[3].y = sp90.f[1] - spc4;
//		vertices[3].z = sp90.f[2] + -spc8 + -spbc;
//
//		gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_COL1, osVirtualToPhysical(rodata->baseaddr));
//
//		// d4c
//		if (rodata->texture) {
//			tconfig = rodata->texture;
//
//			sp62 = random() * 1024;
//			sp5c = (coss(sp62) * tconfig->width * 0xb5) >> 18;
//			sp58 = (sins(sp62) * tconfig->width * 0xb5) >> 18;
//
//			centre = tconfig->width << 4;
//
//			vertices[0].s = centre - sp5c;
//			vertices[0].t = centre - sp58;
//			vertices[1].s = centre + sp58;
//			vertices[1].t = centre - sp5c;
//			vertices[2].s = centre + sp5c;
//			vertices[2].t = centre + sp58;
//			vertices[3].s = centre - sp58;
//			vertices[3].t = centre + sp5c;
//
//			model000216cc(renderdata, tconfig, 4);
//		} else {
//			model000216cc(renderdata, NULL, 1);
//		}
//
//		gSPSetGeometryMode(renderdata->gdl++, G_CULL_BACK);
//		gSPMatrix(renderdata->gdl++, osVirtualToPhysical(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
//		gDPSetColorArray(renderdata->gdl++, osVirtualToPhysical(colours), 1);
//		gDPSetVerticeArray(renderdata->gdl++, osVirtualToPhysical(vertices), 4);
//		gDPTri2(renderdata->gdl++, 0, 1, 2, 2, 3, 0);
//	}
//}

void modelRender(struct modelrenderdata *renderdata, struct model *model)
{
	union modelrodata *rodata;
	union modelrwdata *rwdata;
	u32 type;
	struct modelnode *node = model->filedata->rootnode;

	gSPSegment(renderdata->gdl++, SPSEGMENT_MODEL_MTX, osVirtualToPhysical(model->matrices));

	while (node) {
		type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_DISTANCE:
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);

			switch (type) {
			case MODELNODETYPE_DISTANCE:
				node->child = rwdata->distance.visible ? rodata->distance.target : NULL;
				break;
			case MODELNODETYPE_TOGGLE:
				node->child = rwdata->toggle.visible ? rodata->toggle.target : NULL;
				break;
			}
			break;
		case MODELNODETYPE_HEADSPOT:
			rwdata = modelGetNodeRwData(model, node);

			if (rwdata->headspot.modelfiledata) {
				struct modelnode *loopnode = rwdata->headspot.modelfiledata->rootnode;
				node->child = loopnode;

				while (loopnode) {
					loopnode->parent = node;
					loopnode = loopnode->next;
				}
			}
			break;
		case MODELNODETYPE_REORDER:
			modelRenderNodeReorder(model, node);
			break;
		case MODELNODETYPE_CHRGUNFIRE:
			modelRenderNodeChrGunfire(renderdata, model, node);
			break;
		case MODELNODETYPE_GUNDL:
			modelRenderNodeGundl(renderdata, model, node);
			break;
		case MODELNODETYPE_DL:
			modelRenderNodeDl(renderdata, model, node);
			break;
		case MODELNODETYPE_STARGUNFIRE:
			modelRenderNodeStarGunfire(renderdata, node);
			break;
		case MODELNODETYPE_CHRINFO:
		default:
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}
}

bool model000220fc(struct modelrodata_bbox *bbox, Mtxf *mtx, struct coord *arg2, struct coord *arg3)
{
	f32 xthingx;
	f32 xthingy;
	f32 xthingz;
	u32 stack1[3];
	f32 xsum1;
	f32 xsum2;
	f32 xsum3;

	f32 f0;
	u32 stack;

	f32 ythingx;
	f32 ythingy;
	f32 ythingz;
	u32 stack2[3];
	f32 ysum1;
	f32 ysum2;
	f32 ysum3;

	f32 mult1;
	f32 mult2;

	f32 bestsum2;
	f32 bestsum1;
	f32 anotherbestsum3;
	f32 anotherbestsum1;

	f32 xmin = bbox->xmin;
	f32 xmax = bbox->xmax;
	f32 ymin = bbox->ymin;
	f32 ymax = bbox->ymax;
	f32 zmin = bbox->zmin;
	f32 zmax = bbox->zmax;

	f32 mult3;
	f32 mult4;

	f32 zthingx;
	f32 zthingy;
	f32 zthingz;
	u32 stack3[3];
	f32 zsum1;
	f32 zsum2;
	f32 zsum3;

	if (var8005efc0 != 0.0f) {
		xmin -= var8005efc0;
		xmax += var8005efc0;
		ymin -= var8005efc0;
		ymax += var8005efc0;
		zmin -= var8005efc0;
		zmax += var8005efc0;
	}

	// x
	xthingx = mtx->m[0][0] * mtx->m[0][0];
	xthingy = mtx->m[0][1] * mtx->m[0][1];
	xthingz = mtx->m[0][2] * mtx->m[0][2];

	xsum1 = mtx->m[0][0] * arg3->f[0] + mtx->m[0][1] * arg3->f[1] + mtx->m[0][2] * arg3->f[2];
	xsum2 = mtx->m[0][0] * (arg2->f[0] - mtx->m[3][0]) + mtx->m[0][1] * (arg2->f[1] - mtx->m[3][1]) + mtx->m[0][2] * (arg2->f[2] - mtx->m[3][2]);

	f0 = -(xthingx + xthingy + xthingz) * xmax;
	xsum3 = -(xsum2 + f0);
	f0 = -(xthingx + xthingy + xthingz) * xmin;
	xsum2 = -(xsum2 + f0);

	if (xsum1 < 0.0f) {
		xsum1 = -xsum1;
		xsum2 = -xsum2;
		xsum3 = -xsum3;
	}

	if (xsum2 < 0.0f && xsum3 < 0.0f) {
		return false;
	}

	if (xsum3 < xsum2) {
		f32 tmp = xsum2;
		xsum2 = xsum3;
		xsum3 = tmp;
	}

	// y
	ythingx = mtx->m[1][0] * mtx->m[1][0];
	ythingy = mtx->m[1][1] * mtx->m[1][1];
	ythingz = mtx->m[1][2] * mtx->m[1][2];

	ysum1 = mtx->m[1][0] * arg3->f[0] + mtx->m[1][1] * arg3->f[1] + mtx->m[1][2] * arg3->f[2];
	ysum2 = mtx->m[1][0] * (arg2->f[0] - mtx->m[3][0]) + mtx->m[1][1] * (arg2->f[1] - mtx->m[3][1]) + mtx->m[1][2] * (arg2->f[2] - mtx->m[3][2]);

	f0 = -(ythingx + ythingy + ythingz) * ymax;
	ysum3 = -(ysum2 + f0);
	f0 = -(ythingx + ythingy + ythingz) * ymin;
	ysum2 = -(ysum2 + f0);

	if (ysum1 < 0.0f) {
		ysum1 = -ysum1;
		ysum2 = -ysum2;
		ysum3 = -ysum3;
	}

	if (ysum2 < 0.0f && ysum3 < 0.0f) {
		return false;
	}

	if (ysum3 < ysum2) {
		f32 tmp = ysum2;
		ysum2 = ysum3;
		ysum3 = tmp;
	}

	// Do x and y comparison things
	mult1 = ysum2 * xsum1;
	mult2 = xsum2 * ysum1;
	mult3 = xsum3 * ysum1;
	mult4 = ysum3 * xsum1;

	if (mult1 < mult2) {
		if (mult4 < mult2) {
			return false;
		}

		bestsum2 = xsum2;
		bestsum1 = xsum1;
	} else {
		if (mult3 < mult1) {
			return false;
		}

		bestsum2 = ysum2;
		bestsum1 = ysum1;
	}

	if (mult3 < mult4) {
		anotherbestsum3 = xsum3;
		anotherbestsum1 = xsum1;
	} else {
		anotherbestsum3 = ysum3;
		anotherbestsum1 = ysum1;
	}

	// z
	zthingx = mtx->m[2][0] * mtx->m[2][0];
	zthingy = mtx->m[2][1] * mtx->m[2][1];
	zthingz = mtx->m[2][2] * mtx->m[2][2];

	zsum1 = mtx->m[2][0] * arg3->f[0] + mtx->m[2][1] * arg3->f[1] + mtx->m[2][2] * arg3->f[2];
	zsum2 = mtx->m[2][0] * (arg2->f[0] - mtx->m[3][0]) + mtx->m[2][1] * (arg2->f[1] - mtx->m[3][1]) + mtx->m[2][2] * (arg2->f[2] - mtx->m[3][2]);

	f0 = -(zthingx + zthingy + zthingz) * zmax;
	zsum3 = -(zsum2 + f0);
	f0 = -(zthingx + zthingy + zthingz) * zmin;
	zsum2 = -(zsum2 + f0);

	if (zsum1 < 0.0f) {
		zsum1 = -zsum1;
		zsum2 = -zsum2;
		zsum3 = -zsum3;
	}

	if (zsum2 < 0.0f && zsum3 < 0.0f) {
		return false;
	}

	if (zsum3 < zsum2) {
		f32 tmp = zsum2;
		zsum2 = zsum3;
		zsum3 = tmp;
	}

	// Do z comparison things with the result of the x/y comparison thing
	if (bestsum2 * zsum1 < zsum2 * bestsum1) {
		if (anotherbestsum3 * zsum1 < zsum2 * anotherbestsum1) {
			return false;
		}
	} else {
		if (zsum3 * bestsum1 < bestsum2 * zsum1) {
			return false;
		}
	}

	return true;
}

s32 model000225d4(struct model *model, struct coord *arg1, struct coord *arg2, struct modelnode **startnode)
{
	struct modelnode *node;
	bool dochildren = true;
	Mtxf *mtx;
	union modelrodata *rodata;
	union modelrwdata *rwdata;
	u32 type;

	if (model);

	if (*startnode) {
		node = *startnode;
		*startnode = NULL;
	} else {
		node = model->filedata->rootnode;
	}

	while (node) {
		if (dochildren && node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}

			if (!node) {
				break;
			}
		}

		dochildren = true;
		type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_BBOX:
			rodata = node->rodata;
			mtx = model0001a5cc(model, node, 0);

			if (model000220fc(&rodata->bbox, mtx, arg1, arg2)) {
				*startnode = node;
				return rodata->bbox.hitpart;
			}

			dochildren = false;
			break;
		case MODELNODETYPE_DISTANCE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			node->child = rwdata->distance.visible ? rodata->distance.target : NULL;
			break;
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			node->child = rwdata->toggle.visible ? rodata->toggle.target : NULL;
			break;
		case MODELNODETYPE_HEADSPOT:
			rwdata = modelGetNodeRwData(model, node);

			if (rwdata->headspot.modelfiledata) {
				struct modelnode *loopnode = rwdata->headspot.modelfiledata->rootnode;

				node->child = loopnode;

				while (loopnode) {
					loopnode->parent = node;
					loopnode = loopnode->next;
				}
			}
			break;
		case MODELNODETYPE_CHRINFO:
		case MODELNODETYPE_DL:
		default:
			break;
		}
	}

	return 0;
}

#define PROMOTE(var) \
	if (var) \
		var = (void *)((u32)var + diff)

void modelPromoteNodeOffsetsToPointers(struct modelnode *node, u32 vma, u32 fileramaddr)
{
	union modelrodata *rodata;
	s32 diff = fileramaddr - vma;

	while (node) {
		u32 type = node->type & 0xff;

		PROMOTE(node->rodata);
		PROMOTE(node->parent);
		PROMOTE(node->next);
		PROMOTE(node->prev);
		PROMOTE(node->child);

		switch (type) {
		case MODELNODETYPE_CHRINFO:
			break;
		case MODELNODETYPE_POSITION:
			break;
		case MODELNODETYPE_GUNDL:
			rodata = node->rodata;
			PROMOTE(rodata->gundl.vertices);
			rodata->gundl.baseaddr = (void *)fileramaddr;
			break;
		case MODELNODETYPE_DL:
			rodata = node->rodata;
			PROMOTE(rodata->dl.vertices);
			rodata->dl.colourtable = (void *)fileramaddr;
			break;
		case MODELNODETYPE_DISTANCE:
			rodata = node->rodata;
			PROMOTE(rodata->distance.target);
			node->child = rodata->distance.target;
			break;
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			PROMOTE(rodata->toggle.target);
			break;
		case MODELNODETYPE_REORDER:
			rodata = node->rodata;
			PROMOTE(rodata->reorder.unk18);
			PROMOTE(rodata->reorder.unk1c);
			break;
		case MODELNODETYPE_11:
			rodata = node->rodata;
			PROMOTE(rodata->type11.unk14);
			break;
		case MODELNODETYPE_0B:
			rodata = node->rodata;
			PROMOTE(rodata->type0b.unk3c);
			rodata->type0b.baseaddr = (void *)fileramaddr;
			break;
		case MODELNODETYPE_CHRGUNFIRE:
			rodata = node->rodata;
			PROMOTE(rodata->chrgunfire.texture);
			rodata->chrgunfire.baseaddr = (void *)fileramaddr;
			break;
		case MODELNODETYPE_0D:
			rodata = node->rodata;
			PROMOTE(rodata->type0d.unk10);
			PROMOTE(rodata->type0d.unk14);
			rodata->type0d.baseaddr = (void *)fileramaddr;
			break;
		case MODELNODETYPE_STARGUNFIRE:
			rodata = node->rodata;
			PROMOTE(rodata->stargunfire.vertices);
			rodata->stargunfire.baseaddr = (void *)fileramaddr;
			break;
		default:
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}
}

/**
 * Convert a model file's file-relative offsets to global pointers,
 * and sort the part numbers list so they can be looked up using bisection.
 *
 * Offsets in model files are based from virtual memory address 0x0f000000.
 * This vma address is specified as an argument to the function.
 */
void modelPromoteOffsetsToPointers(struct modelfiledata *filedata, u32 vma, u32 fileramaddr)
{
	s32 diff = fileramaddr - vma;
	s32 i;
	s16 *partnums;

	PROMOTE(filedata->rootnode);
	PROMOTE(filedata->parts);
	PROMOTE(filedata->texconfigs);

	for (i = 0; i < filedata->numparts; i++) {
		PROMOTE(filedata->parts[i]);
	}

	modelPromoteNodeOffsetsToPointers(filedata->rootnode, vma, fileramaddr);

	// Sort parts by part number so they can be bisected during lookup
	partnums = (s16 *)&filedata->parts[filedata->numparts];

	if (filedata->numparts) {
		struct modelnode *tmpnode;
		s16 tmpnum;
		bool changed;

		do {
			changed = false;

			for (i = 0; i < filedata->numparts - 1; i++) {
				if (partnums[i] > partnums[i + 1]) {
					tmpnum = partnums[i];
					partnums[i] = partnums[i + 1];
					partnums[i + 1] = tmpnum;

					tmpnode = filedata->parts[i];
					filedata->parts[i] = filedata->parts[i + 1];
					filedata->parts[i + 1] = tmpnode;

					changed = true;
				}
			}
		} while (changed == true);
	}
}

s32 modelCalculateRwDataIndexes(struct modelnode *basenode)
{
	u16 len = 0;
	struct modelnode *node = basenode;
	union modelrodata *rodata;

	while (node) {
		u32 type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_CHRINFO:
			rodata = node->rodata;
			rodata->chrinfo.rwdataindex = len;
			len += sizeof(struct modelrwdata_chrinfo) / 4;
			break;
		case MODELNODETYPE_DISTANCE:
			rodata = node->rodata;
			rodata->distance.rwdataindex = len;
			len += sizeof(struct modelrwdata_distance) / 4;
			node->child = rodata->distance.target;
			break;
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			rodata->toggle.rwdataindex = len;
			len += sizeof(struct modelrwdata_toggle) / 4;
			node->child = rodata->toggle.target;
			break;
		case MODELNODETYPE_HEADSPOT:
			rodata = node->rodata;
			rodata->headspot.rwdataindex = len;
			len += sizeof(struct modelrwdata_headspot) / 4;
			node->child = NULL;
			break;
		case MODELNODETYPE_REORDER:
			rodata = node->rodata;
			rodata->reorder.rwdataindex = len;
			len += sizeof(struct modelrwdata_reorder) / 4;
			model0001c868(node, false);
			break;
		case MODELNODETYPE_0B:
			rodata = node->rodata;
			rodata->type0b.rwdataindex = len;
			len += sizeof(struct modelrwdata_0b) / 4;
			break;
		case MODELNODETYPE_CHRGUNFIRE:
			rodata = node->rodata;
			rodata->chrgunfire.rwdataindex = len;
			len += sizeof(struct modelrwdata_chrgunfire) / 4;
			break;
		case MODELNODETYPE_DL:
			rodata = node->rodata;
			rodata->dl.rwdataindex = len;
			len += sizeof(struct modelrwdata_dl) / 4;
			break;
		default:
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node == basenode->parent) {
					node = NULL;
					break;
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}

	return len;
}

void modelCalculateRwDataLen(struct modelfiledata *filedata)
{
	filedata->rwdatalen = modelCalculateRwDataIndexes(filedata->rootnode);
}

void modelInitRwData(struct model *model, struct modelnode *startnode)
{
	struct modelnode *node = startnode;
	union modelrodata *rodata;
	union modelrwdata *rwdata;

	while (node) {
		u32 type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_CHRINFO:
			rwdata = modelGetNodeRwData(model, node);

			rwdata->chrinfo.unk00 = 0;
			rwdata->chrinfo.ground = 0;
			rwdata->chrinfo.pos.x = 0;
			rwdata->chrinfo.pos.y = 0;
			rwdata->chrinfo.pos.z = 0;
			rwdata->chrinfo.unk14 = 0;
			rwdata->chrinfo.unk18 = 0;
			rwdata->chrinfo.unk1c = 0;

			rwdata->chrinfo.unk01 = 0;
			rwdata->chrinfo.unk34.x = 0;
			rwdata->chrinfo.unk34.y = 0;
			rwdata->chrinfo.unk34.z = 0;
			rwdata->chrinfo.unk30 = 0;
			rwdata->chrinfo.unk24.x = 0;
			rwdata->chrinfo.unk24.y = 0;
			rwdata->chrinfo.unk24.z = 0;
			rwdata->chrinfo.unk20 = 0;

			rwdata->chrinfo.unk02 = 0;
			rwdata->chrinfo.unk4c.x = 0;
			rwdata->chrinfo.unk4c.y = 0;
			rwdata->chrinfo.unk4c.z = 0;
			rwdata->chrinfo.unk40.x = 0;
			rwdata->chrinfo.unk40.y = 0;
			rwdata->chrinfo.unk40.z = 0;
			rwdata->chrinfo.unk5c = 0;
			break;
		case MODELNODETYPE_DISTANCE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			rwdata->distance.visible = false;
			node->child = rodata->distance.target;
			break;
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			rwdata->toggle.visible = true;
			node->child = rodata->toggle.target;
			break;
		case MODELNODETYPE_HEADSPOT:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->headspot.modelfiledata = NULL;
			rwdata->headspot.rwdatas = NULL;
			break;
		case MODELNODETYPE_REORDER:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->reorder.visible = false;
			modelRenderNodeReorder(model, node);
			break;
		case MODELNODETYPE_0B:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->type0b.unk00 = 0;
			break;
		case MODELNODETYPE_CHRGUNFIRE:
			rwdata = modelGetNodeRwData(model, node);
			rwdata->chrgunfire.visible = false;
			break;
		case MODELNODETYPE_DL:
			rodata = node->rodata;
			rwdata = modelGetNodeRwData(model, node);
			rwdata->dl.vertices = rodata->dl.vertices;
			rwdata->dl.gdl = rodata->dl.primary;
			rwdata->dl.colours = (void *) ALIGN8((u32)(rodata->dl.vertices + rodata->dl.numvertices));
			if (rodata->dl.numvertices);
			if (rodata->dl.numvertices);
			if (rodata->dl.numvertices);
			break;
		default:
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node == startnode->parent) {
					node = NULL;
					break;
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}
}

void modelInit(struct model *model, struct modelfiledata *filedata, union modelrwdata **rwdatas, bool resetanim)
{
	struct modelnode *node;

	model->unk00 = 0;
	model->filedata = filedata;
	model->rwdatas = rwdatas;
	model->unk02 = -1;
	model->scale = 1;
	model->attachedtomodel = NULL;
	model->attachedtonode = NULL;

	node = filedata->rootnode;

	while (node) {
		u32 type = node->type & 0xff;

		if (type == MODELNODETYPE_HEADSPOT) {
			model->unk00 |= 1;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node == filedata->rootnode->parent) {
					node = NULL;
					break;
				}

				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}

	if (rwdatas != NULL) {
		modelInitRwData(model, filedata->rootnode);
	}

	if (resetanim) {
		model->anim = NULL;
	}
}

void animInit(struct anim *anim)
{
	anim->animnum = 0;
	anim->animnum2 = 0;
	anim->looping = 0;
	anim->flipfunc = NULL;
	anim->unk6c = 0;
	anim->unk70 = NULL;
	anim->average = 0;
	anim->frac = 0;
	anim->timespeed = 0;
	anim->frac2 = 0;
	anim->timespeed2 = 0;
	anim->fracmerge = 0;
	anim->timemerge = 0;
	anim->timeplay = 0;
	anim->endframe = -1;
	anim->endframe2 = -1;
	anim->speed = 1;
	anim->speed2 = 1;
	anim->playspeed = 1;
	anim->animscale = 1;
}

void model00023108(struct model *pmodel, struct modelfiledata *pmodeldef, struct modelnode *pnode, struct modelfiledata *cmodeldef)
{
	struct modelrwdata_headspot *rwdata = modelGetNodeRwData(pmodel, pnode);
	struct modelnode *node;

	rwdata->modelfiledata = cmodeldef;
	rwdata->rwdatas = &pmodel->rwdatas[pmodeldef->rwdatalen];

	pnode->child = cmodeldef->rootnode;

	node = pnode->child;

	while (node) {
		node->parent = pnode;
		node = node->next;
	}

	pmodeldef->rwdatalen += modelCalculateRwDataIndexes(pnode->child);
}

/**
 * This function can be called repeatedly to iterate a model's display lists.
 *
 * On the first call, the value passed as nodeptr should point to a NULL value.
 * Each time the function is called, it will update *gdlptr to point to the next
 * display list, and will update *nodeptr to point to the current node. On
 * subsequent calls, the same values should be passed as nodeptr and gdlptr so
 * the function can continue correctly.
 *
 * Note that some node types support multiple display lists, so the function
 * may return the same node while it iterates the display lists for that node.
 */
void modelIterateDisplayLists(struct modelfiledata *filedata, struct modelnode **nodeptr, Gfx **gdlptr)
{
	struct modelnode *node = *nodeptr;
	union modelrodata *rodata;
	Gfx *gdl = NULL;

	if (node == NULL) {
		node = filedata->rootnode;
	}

	while (node) {
		u32 type = node->type & 0xff;

		switch (type) {
		case MODELNODETYPE_GUNDL:
			rodata = node->rodata;

			if (node != *nodeptr) {
				gdl = rodata->gundl.primary;
			} else if (rodata->gundl.secondary != *gdlptr) {
				gdl = rodata->gundl.secondary;
			}
			break;
		case MODELNODETYPE_DL:
			rodata = node->rodata;

			if (node != *nodeptr) {
				gdl = rodata->dl.primary;
			} else if (rodata->dl.secondary != *gdlptr) {
				gdl = rodata->dl.secondary;
			}
			break;
		case MODELNODETYPE_STARGUNFIRE:
			rodata = node->rodata;

			if (node != *nodeptr) {
				gdl = rodata->stargunfire.gdl;
			}
			break;
		case MODELNODETYPE_DISTANCE:
			rodata = node->rodata;
			node->child = rodata->distance.target;
			break;
		case MODELNODETYPE_TOGGLE:
			rodata = node->rodata;
			node->child = rodata->toggle.target;
			break;
		case MODELNODETYPE_REORDER:
			model0001c868(node, true);
			break;
		}

		if (gdl) {
			break;
		}

		if (node->child) {
			node = node->child;
		} else {
			while (node) {
				if (node->next) {
					node = node->next;
					break;
				}

				node = node->parent;
			}
		}
	}

	*gdlptr = gdl;
	*nodeptr = node;
}

void modelNodeReplaceGdl(u32 arg0, struct modelnode *node, Gfx *find, Gfx *replacement)
{
	union modelrodata *rodata;
	u32 type = node->type & 0xff;

	switch (type) {
	case MODELNODETYPE_GUNDL:
		rodata = node->rodata;

		if (rodata->gundl.primary == find) {
			rodata->gundl.primary = replacement;
			return;
		}

		if (rodata->gundl.secondary == find) {
			rodata->gundl.secondary = replacement;
			return;
		}
		break;
	case MODELNODETYPE_DL:
		rodata = node->rodata;

		if (rodata->dl.primary == find) {
			rodata->dl.primary = replacement;
			return;
		}

		if (rodata->dl.secondary == find) {
			rodata->dl.secondary = replacement;
			return;
		}
		break;
	case MODELNODETYPE_STARGUNFIRE:
		rodata = node->rodata;

		if (rodata->stargunfire.gdl == find) {
			rodata->stargunfire.gdl = replacement;
			return;
		}
		break;
	}
}
