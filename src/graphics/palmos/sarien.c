/*
 * $Id$
 */

#include <PalmOS.h>

#include "callback.h"
#include "sarienRsc.h"

#define FRAME_RATE 2
#define BPP 8

void mainloop (void);
static UInt32 timeout, ticks;


char *strdup (char *s)
{
	char *r = MemPtrNew (strlen (s) + 1);
	strcpy (r, s);
	return r;
}


static Boolean MainFormHandleEvent (EventPtr e)
{
    Boolean handled = false;
    FormPtr frm;
    
    CALLBACK_PROLOGUE

    switch (e->eType) {
    case frmOpenEvent:
	frm = FrmGetActiveForm();
	FrmDrawForm(frm);
	handled = true;
	break;

    case menuEvent:
	MenuEraseStatus(NULL);

	switch(e->data.menu.itemID) {
	}

    	handled = true;
	break;

    case ctlSelectEvent:
	switch(e->data.ctlSelect.controlID) {
	}
	break;

    case nilEvent: {
	UInt32 dt, t0 = TimGetTicks();
	mainloop ();
	dt = TimGetTicks() - t0;
	timeout = (TimGetTicks() - t0 > ticks) ? 0 : ticks - dt;
	}
	break;
    default:
        break;
    }

    CALLBACK_EPILOGUE

    return handled;
}

static Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    UInt16  formId;
    Boolean handled = false;

    if (e->eType == frmLoadEvent) {
	formId = e->data.frmLoad.formID;
	frm = FrmInitForm(formId);
	FrmSetActiveForm(frm);

	switch(formId) {
	case MainForm:
	    FrmSetEventHandler(frm, MainFormHandleEvent);
	    break;
	}
	handled = true;
    }

    return handled;
}

/* Get preferences, open (or create) app database */
static UInt16 StartApplication(void)
{
    FrmGotoForm(MainForm);
    return 0;
}

/* Save preferences, close forms, close app database */
static void StopApplication(void)
{
    FrmSaveAllForms();
    FrmCloseAllForms();
}

/* The main event loop */
static void EventLoop(void)
{
    UInt16 err;
    EventType e;

    do {
	EvtGetEvent(&e, timeout);
	if (! SysHandleEvent (&e))
	    if (! MenuHandleEvent (NULL, &e, &err))
		if (! ApplicationHandleEvent (&e))
		    FrmDispatchEvent (&e);
    } while (e.eType != appStopEvent);
}




WinHandle buffer;


static void clear (UInt8 color)
{
	MemSet (BmpGetBits(WinGetBitmap(buffer)), 160 * (160 * BPP / 8), color);
}



static void blit ()
{
	const RectangleType rect = { {0,0}, {160,160} };
	WinCopyRectangle (buffer, WinGetDisplayWindow(), &rect, 0, 0, winPaint);
}


void mainloop ()
{
	static int i = 0;

	clear (i);
	blit ();

	i++;
}



/* Main entry point; it is unlikely you will need to change this except to
   handle other launch command codes */
UInt32 PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt16 err;

	if (cmd != sysAppLaunchCmdNormalLaunch)
		return sysErrParamErr;

	if (!(buffer = WinCreateOffscreenWindow (160, 160, screenFormat, &err)))
		return err;

	if ((err = StartApplication()) != 0)
		return err;

	ticks = SysTicksPerSecond() / FRAME_RATE;

	run_game ();
	/* EventLoop(); */
	StopApplication();

	WinDeleteWindow (buffer, false);

	return 0;
}

