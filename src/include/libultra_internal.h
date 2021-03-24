#ifndef _LIBULTRA_INTERNAL_H_
#define _LIBULTRA_INTERNAL_H_
#include <ultra64.h>

typedef struct
{
    u32 initialized; //probably something like initialized?
    OSThread *mgrThread;
    OSMesgQueue *unk08;
    OSMesgQueue *unk0c;
    OSMesgQueue *unk10;
    s32 (*dma_func)(s32,u32,void*,size_t);
    u64 force_align;
} OSMgrArgs;

s32 __osDisableInt();
void __osRestoreInt(s32);
s32 __osContRamRead(OSMesgQueue *mq, int channel, u16 address, u8 *buffer);
s32 __osContRamWrite(OSMesgQueue *mq, int channel, u16 address, u8 *buffer, int force);
void __osEnqueueAndYield(OSThread**);
void __osDequeueThread(OSThread**, OSThread*);
void __osEnqueueThread(OSThread**, OSThread*);
OSThread* __osPopThread(OSThread**);
s32 __osSiRawStartDma(s32, void*);
void __osSiCreateAccessQueue();
void __osSiGetAccess();
void __osSiRelAccess();
u32 __osProbeTLB(void*);
void __osPiCreateAccessQueue();
void __osPiGetAccess();
void __osSetSR(u32);
u32 __osGetSR();
void __osSetFpcCsr(u32);
s32 __osSiRawReadIo(u32, u32*);
s32 __osSiRawWriteIo(u32, u32);
s32 osPiRawReadIo(u32 a0, u32 *a1);
void __osSpSetStatus(u32);
u32 __osSpGetStatus();
s32 __osSpSetPc(u32);
s32 __osSpDeviceBusy();
s32 __osSiDeviceBusy();
s32 __osSpRawStartDma(s32 direction, u32 devAddr, void *dramAddr, u32 size);
void __osViInit();
__OSViContext * __osViGetCurrentContext();
void __osViSwapContext();
void __osSetTimerIntr(u64);
u64 __osInsertTimer(OSTimer *);
void __osSetCompare(u32);
s32 __osAiDeviceBusy();
void __osDispatchThread();
u32 __osGetCause();
s32 __osAtomicDec(u32*);
s32 __osPfsSelectBank(OSPfs *pfs, u8 bank);

#endif
