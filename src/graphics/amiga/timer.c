/*
**
** $VER: timer.c 0.1 (27.02.99)
** Description
**
** (C) Copyright 1999 Paul Hill
**
*/

#include <proto/exec.h>
#include <proto/timer.h>
#include <proto/alib.h>
#include <devices/timer.h>
#include <sys/time.h>
#include <stdio.h>

static struct timerequest *TimerIO=NULL;
static struct MsgPort *TimerMP=NULL;
static struct Device *TimerBase=NULL;


int opentimer()
{
	int rc=0;

	if ((TimerMP = CreatePort(0,0)))
	{
		if ((TimerIO = (struct timerequest *) CreateExtIO(TimerMP,sizeof(struct timerequest)) ))
		{
			/* Open the device once */
			if (!(OpenDevice( TIMERNAME, UNIT_VBLANK,(struct IORequest *) TimerIO, 0L)))
			{
				TimerBase = (struct Device *)TimerIO->tr_node.io_Device;
				rc = 1;
			}
		}
	}
	return rc;
}


void closetimer()
{
	if (TimerIO)
	{
		/* Delete any pending timer requests */
		if (!(CheckIO((struct IORequest *)TimerIO))) AbortIO((struct IORequest *)TimerIO);

		CloseDevice((struct IORequest *) TimerIO);
		DeleteExtIO((struct IORequest *) TimerIO);
	}

	if (TimerMP)
	{
		DeletePort(TimerMP);
	}
}



int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	if (tp)
	{
		GetSysTime(tp);
//	void GetSysTime( struct timeval * );
//		TimerIO->tr_node.io_Command = TR_GETSYSTIME;
//		DoIO((struct IORequest *) TimerIO);
//		*tp = TimerIO->tr_time;

		/* add the offset from unix to amigados time system (NEEDED???) */
		tp->tv_sec += (8*365+2) * 24 * 3600;
	}

  return 0;
}

