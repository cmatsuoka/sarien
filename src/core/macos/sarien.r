/*
 * $Id$
 */

#include "Types.r"

/* These define's are used in the MENU resources to disable specific
 * menu items.
 */
#define AllItems	0b1111111111111111111111111111111	/* 31 flags */
#define MenuItem1	0b00001
#define MenuItem2	0b00010
#define MenuItem3	0b00100
#define MenuItem4	0b01000
#define MenuItem5	0b10000


resource 'MENU' (128, "Apple", preload) {
	128,
	textMenuProc,
	AllItems & ~MenuItem2,		/* Disable item #2 */
	enabled,
	apple,
	{ "About Sarien…",	noicon, nokey, nomark, plain;
	  "-",			noicon, nokey, nomark, plain }
};

#if 0
resource 'MENU' (129, "File", preload) {
	129,
	textMenuProc,
	AllItems,			/* Disable item #2 */
	enabled,
	"File",
	{ "Quit",		noicon, "Q", nomark, plain }
};
#endif

resource 'DLOG' (1000, "About Sarien…") {
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
		{ 60, 167, 81, 244 }, button { enabled, "OK" };
		{ 5, 10, 60, 400 }, staticText {
			disabled,
			"This is an experimental port of Sarien to MacOS.  "
			"It's not functional now (as you may have noticed).  "
		}
	}
};


data 'pltt' (1000, preload) {
	$"0022 0000 0000 0000 0000 0000 0000 0000"	/* Entries */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"	/* white */
	$"0000 0000 0000 0002 0000 0000 0000 0000"	/* black */
	$"FFFF 0000 0000 0004 0000 0000 0000 0000"
};


/*
 * Here is the quintessential MultiFinder friendliness device,
 * the SIZE resource
 */

resource 'SIZE' (-1) {
	saveScreen,
	acceptSuspendResumeEvents,
	disableOptionSwitch,
	canBackground,
	needsActivateOnFGSwitch,	/* we do our own activate/deactivate */
	backgroundAndForeground,	/* not a background-only application! */
	dontGetFrontClicks,		/* change this is if you want "do first click" behavior like the Finder */
	ignoreAppDiedEvents,		/* essentially, I'm not a debugger */
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

