/*
 * $Id$
 */
 
#include "Types.r"
#include "resource.h"

/* These define's are used in the MENU resources to disable specific
 * menu items.
 */
#define AllItems	0b1111111111111111111111111111111	/* 31 flags */
#define MenuItem1	0b00001
#define MenuItem2	0b00010
#define MenuItem3	0b00100
#define MenuItem4	0b01000
#define MenuItem5	0b10000


resource 'MBAR' (rMenuBar, preload) {
	{ mApple, mFile, mView };
};

resource 'MENU' (mApple, "Apple", preload) {
	mApple,
	textMenuProc,
	AllItems & ~MenuItem2,	/* Disable item #2 */
	enabled,
	apple,
	{ "About Sarien�", noicon, nokey, nomark, plain;
	  "-", noicon, nokey, nomark, plain
	}
};

resource 'MENU' (mFile, "File", preload) {
	mFile,
	textMenuProc,
	AllItems,
	enabled,
	"File",
	{ "Quit", noicon, "Q", nomark, plain }
};

resource 'MENU' (mView, "View", preload) {
	mView,
	textMenuProc,
	AllItems & ~MenuItem2 & ~MenuItem4,
	enabled,
	"View",
	{ "Sarien colors", noicon, "P", nomark, plain;
	  "-", noicon, nokey, nomark, plain;
	  "Hi-res mode", noicon, "H", nomark, plain;
	  "-", noicon, nokey, nomark, plain;
	  "Double size", noicon, "S", nomark, plain;
	  "Aspect ratio", noicon, "R", nomark, plain
	}
};

resource 'DLOG' (1000, "About Sarien�") {
	{ 90, 50, 180, 460 },
	rDocProc,
	visible,
	noGoAway,
	0x0,
	1000,
	"SarienAbout",
	centerMainScreen
};


resource 'DITL' (1000) {
	 {
/* 1 */ {60, 167, 81, 244},
		button {
			enabled,
			"OK"
		};
/* 2 */ {5, 10, 60, 400},				/* SourceLanguage Item */
		staticText {
			disabled,
			"This is an experimental port of Sarien to MacOS.  "
			"It's not fully functional now (as you may have noticed).  "
		}
	}
};


data 'pltt' (1000, preload) {
	$"0022 0000 0000 0000 0000 0000 0000 0000"  /* 32 entries, plus B&W */

	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"  /* white as first guy. */
	$"0000 0000 0000 0002 0000 0000 0000 0000"  /* black as next guy. */

	$"FFFF 0000 0000 0004 0000 0000 0000 0000"
};


/* here is the quintessential MultiFinder friendliness device,
 * the SIZE resource
 */
resource 'SIZE' (-1) {
	saveScreen,
	acceptSuspendResumeEvents,
	disableOptionSwitch,
	canBackground,
	needsActivateOnFGSwitch,	/* we do our own activate/deactivate */
	backgroundAndForeground,	/* not a background-only application! */
	dontGetFrontClicks,		/* not "do first click" behavior */
	ignoreAppDiedEvents,		/* I'm not a debugger (sub-launching) */
	is32BitCompatible,		/* run in 32-bit address space */
	notHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	8192 * 1024,			/* preferred size */
	4096 * 1024			/* minimum size */
};

