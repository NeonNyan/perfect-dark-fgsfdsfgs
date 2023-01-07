#ifndef _IN_GAME_LV_H
#define _IN_GAME_LV_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern bool g_LvPaused;
// The effective SLOWMOTION_* mode in play
// When "Smart Slow motion" is not in use,
// g_slowmo is set to SLOWMOTION_OFF or SLOWMOTION_ON
// as one would expect. When "Smart Slow Motion" is used,
// this is always set to SLOWMOTION_OFF until the required conditions
// to trigger the apparent slow motion happen. And it's reset to SLOWMOTION_OFF when not.
// By default, this is when two human-controled player characters are in the same or nearby room.
extern s32 g_slowmo;

u32 getVar80084040(void);
void lvInit(void);
void lvResetMiscSfx(void);
s32 lvGetMiscSfxIndex(u32 arg0);
void lvSetMiscSfxState(u32 type, bool play);
void lvUpdateMiscSfx(void);
void lvReset(s32 stagenum);
Gfx *lvRenderFade(Gfx *gdl);
void lvFadeReset(void);
bool lvUpdateTrackedProp(struct trackedprop *trackedprop, s32 index);
void lvFindThreatsForProp(struct prop *prop, bool inchild, struct coord *playerpos, s32 *activeslots, f32 *param_5);
void func0f168f24(struct prop *prop, bool inchild, struct coord *playerpos, s32 *activeslots, f32 *distances);
void lvFindThreats(void);
Gfx *lvRender(Gfx *gdl);
void lvUpdateSoloHandicaps(void);
s32 sub54321(s32 value);
void lvUpdateCutsceneTime(void);
s32 lvGetSlowMotionType(void);
void lvTick(void);
void lvTickPlayer(void);
void lvCheckPauseStateChanged(void);
void lvSetPaused(bool paused);
void lvConfigureFade(u32 color, s16 num_frames);
bool lvIsFadeActive(void);
void lvStop(void);
bool lvIsPaused(void);
s32 lvGetDifficulty(void);
void lvSetDifficulty(s32 difficulty);
void lvSetMpTimeLimit60(u32 limit);
void lvSetMpScoreLimit(u32 limit);
void lvSetMpTeamScoreLimit(u32 limit);
f32 lvGetStageTimeInSeconds(void);
s32 lvGetStageTime60(void);

#endif
