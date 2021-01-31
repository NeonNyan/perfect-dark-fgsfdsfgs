#ifndef _ULTRA64_TIME_H_
#define _ULTRA64_TIME_H_
#include <PR/os_message.h>

/* Types */

typedef struct OSTimer_str
{
    struct OSTimer_str *next;
    struct OSTimer_str *prev;
    u64 interval;
    u64 remaining;
    OSMesgQueue *mq;
    OSMesg *msg;
} OSTimer;

typedef u64 OSTime;

/* Functions */

OSTime osGetTime(void);
void osSetTime(OSTime time);
int osSetTimer(OSTimer *t, OSTime value, OSTime interval, OSMesgQueue *mq, OSMesg msg);

#endif
