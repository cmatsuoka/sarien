/* * $Id$ * * Warning: This file must be in MacOS format (CR only) or Rez won't be * able to read it. */#include "Types.r"#include "resource.h"/* These define's are used in the MENU resources to disable specific * menu items. */#define AllItems	0b1111111111111111111111111111111	/* 31 flags */#define MenuItem1	0b00001#define MenuItem2	0b00010#define MenuItem3	0b00100#define MenuItem4	0b01000#define MenuItem5	0b10000resource 'MBAR' (rMenuBar, preload) {	{ mApple, mFile, mView };};resource 'MENU' (mApple, "Apple", preload) {	mApple,	textMenuProc,	AllItems & ~MenuItem2,	/* Disable item #2 */	enabled,	apple,	{ "About Sarien�", noicon, nokey, nomark, plain;	  "-", noicon, nokey, nomark, plain	}};resource 'MENU' (mFile, "File", preload) {	mFile,	textMenuProc,	AllItems,	enabled,	"File",	{ "Quit", noicon, "Q", nomark, plain }};resource 'MENU' (mView, "View", preload) {	mView,	textMenuProc,	AllItems & ~MenuItem2 & ~MenuItem4,	enabled,	"View",	{ "Sarien colors", noicon, "P", nomark, plain;	  "-", noicon, nokey, nomark, plain;	  "Hi-res mode", noicon, "H", nomark, plain;	  "-", noicon, nokey, nomark, plain;	  "Double size", noicon, "S", nomark, plain;	  "Aspect ratio", noicon, "R", nomark, plain	}};resource 'DLOG' (rAbout, "About Sarien�") {	{ 100, 100, 412, 580 },	dBoxProc,	visible,	noGoAway,	0x0,	rAbout,	"About Sarien",	centerMainScreen};resource 'dctb' (rAbout) {	{		wContentColor,	0xdddd, 0xdddd, 0xdddd,		wFrameColor,	0x0000, 0x0000, 0x0000,		wHiliteColor,	0x0000,	0x0000,	0x0000	}};resource 'DITL' (rAbout) {	 {/* 1 */ { 280, 187, 301, 304 },		button {			enabled,			"OK"		};/* 2 */ { 21, 94, 42, 420 },				/* SourceLanguage Item */		staticText {			disabled,			"Sarien - A Sierra AGI Resource Interpreter Engine"		};/* 3 */	{ 56, 94, 141, 444 },		staticText {			disabled,			"Copyright (C) 1999-2002 Stuart George. "			"Portions Copyright (C) 1998 Lance Ewing, "			"(C) 1999 Felipe Rosinha, "			"(C) 1999-2002 Claudio Matsuoka, "			"(C) 1999-2001 Igor Nesterov, "			"(C) 2001,2002 Vasyl Tsvirkunov, "			"(C) 2001,2002 Thomas Akesson."		};/* 4 */	{ 153, 94, 177, 360 },		staticText {			disabled,			"MacOS port written by Claudio Matsuoka."		};/* 5 */	{ 189, 94, 256, 440 },		staticText {			disabled,			"This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License, version 2 or later, as published by the Free Software Foundation."		};/* 6 */ { 60, 20, 181, 68 },		picture {			disabled,			rAboutPicture		};	}};data 'pltt' (1000, preload) {	$"0022 0000 0000 0000 0000 0000 0000 0000"  /* 32 entries, plus B&W */	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"  /* white as first guy. */	$"0000 0000 0000 0002 0000 0000 0000 0000"  /* black as next guy. */	$"FFFF 0000 0000 0004 0000 0000 0000 0000"};/* here is the quintessential MultiFinder friendliness device, * the SIZE resource */resource 'SIZE' (-1) {	saveScreen,	acceptSuspendResumeEvents,	disableOptionSwitch,	canBackground,	needsActivateOnFGSwitch,	/* we do our own activate/deactivate */	backgroundAndForeground,	/* not a background-only application! */	dontGetFrontClicks,			/* not "do first click" behavior */	ignoreAppDiedEvents,		/* I'm not a debugger (sub-launching) */	is32BitCompatible,			/* run in 32-bit address space */	notHighLevelEventAware,	onlyLocalHLEvents,	notStationeryAware,	dontUseTextEditServices,	reserved,	reserved,	reserved,	8192 * 1024,				/* preferred size */	4096 * 1024					/* minimum size */};/* * Icons */resource 'FREF' (rApplFref) {	'APPL', rApplFrefLocalID, ""};type 'FAGI' as 'STR ';resource 'FAGI' (0) {			/* Creator resource ID must be 0 */	"Sarien 0.8.0 Copyright (C) 1999-2002 Stuart George and Claudio Matsuoka"};resource 'BNDL' (0) {			/* Bundle resource ID should be 0 */	'FAGI',						/* Signature */	0,							/* Creator resource ID (must be 0) */	{		'ICN#', { rApplFrefLocalID, rApplIcon },		'FREF', { rApplIconLocalID, rApplFref }	}};data 'ICN#' (128) {	$"0A80 0000 1540 0000 2AA0 0000 17E0 0000"            /* .�...@..*�...�.. */	$"2BE0 0000 154C 0000 2AAC 0000 0D4C 0000"            /* +�...L..*�..�L.. */	$"08AC 0000 040C 0000 0A28 0000 1544 3FFE"            /* .�.......(...D?� */	$"2A8A 4001 1555 4FF9 2BA8 5001 17C4 5001"            /* *�@..UO�+�P..�P. */	$"2A8C 5001 154C 5001 0AAC 5001 0D54 5001"            /* *�P..LP..�P.�TP. */	$"0BC8 5001 0547 4001 028F 4001 0157 4079"            /* .�P..G@..�@..W@y */	$"0A20 4001 0414 5001 0808 4001 0404 7FFF"            /* . @...P...@....� */	$"2808 3FFE 1404 0000 300C 0000 300C 0000"            /* (.?�....0...0... */	$"0F80 0000 3FE0 0000 3FE0 0000 3FE0 0000"            /* .�..?�..?�..?�.. */	$"3FE0 0000 3FEC 0000 3FEC 0000 0FEC 0000"            /* ?�..?�..?�...�.. */	$"0C6C 0000 0C0C 0000 0F3C 0000 3FCC 3FFE"            /* .l.......<..?�?� */	$"3FCF 7FFF 3FFF 7FFF 3FFC 7FFF 3FCC 7FFF"            /* ?�.�?�.�?�.�?�.� */	$"3FCC 7FFF 3FCC 7FFF 0FFC 7FFF 0FFC 7FFF"            /* ?�.�?�.�.�.�.�.� */	$"0FCC 7FFF 0FCF 7FFF 03CF 7FFF 03FF 7FFF"            /* .�.�.�.�.�.�.�.� */	$"0F30 7FFF 0C3C 7FFF 0C0C 7FFF 0C0C 7FFF"            /* .0.�.<.�...�...� */	$"3C0C 3FFE 3C0C 0000 300C 0000 300C 0000"            /* <.?�<...0...0... */};data 'icl8' (128) {	$"0000 0000 D8D8 D8D8 D800 0000 0000 0000"            /* ....�����....... */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 D8D8 D8D8 D800 0000 0000"            /* ..���������..... */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 D8D8 D8D8 D800 0000 0000"            /* ..���������..... */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 FFFF FFFF FF00 0000 0000"            /* ..���������..... */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 D8D8 D8D8 D800 0000 0000"            /* ..���������..... */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 D8D8 D8D8 D800 FAFA 0000"            /* ..���������.��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 D8D8 D8D8 D800 FFFF 0000"            /* ..���������.��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 0000 FFFF D8D8 D8D8 D800 FAFA 0000"            /* ....�������.��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 0000 D8D8 0000 00D8 D800 FFFF 0000"            /* ....��...��.��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 0000 D8D8 0000 0000 0000 FFFF 0000"            /* ....��......��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 0000 D8D8 D8D8 0000 1616 1616 0000"            /* ....����........ */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 D8D8 D8D8 D8D8 D8D8 0000 1616 0000"            /* ..��������...... */	$"0001 FFFF FFFF FFFF FFFF FFFF FFFF FF01"            /* ..�������������. */	$"0000 D8D8 D8D8 D8D8 D8D8 0000 FFFF FFFF"            /* ..��������..���� */	$"00FF 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2AFF"            /* .�*************� */	$"0000 D8D8 D8D8 D8D8 D8D8 D8D8 FFFF FFFF"            /* ..�������������� */	$"00FF 2A2A FFFF FFFF FFFF FFFF FF2A 2AFF"            /* .�**���������**� */	$"0000 D8D8 D8D8 FFFF D8D8 D8D8 FFFF 0000"            /* ..������������.. */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 D8D8 D8D8 FFFF FFFF 0000 FFFF 0000"            /* ..��������..��.. */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 D8D8 D8D8 D8D8 D8D8 0000 FFFF 0000"            /* ..��������..��.. */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 D8D8 D8D8 D8D8 D8D8 0000 1616 0000"            /* ..��������...... */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 0000 D8D8 D8D8 D8D8 D8D8 1616 0000"            /* ....��������.... */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 0000 FFFF D8D8 D8D8 D8D8 1616 0000"            /* ....��������.... */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 0000 D8D8 FFFF FFFF 0000 FFFF 0000"            /* ....������..��.. */	$"00FF 2AFF 2A2A 2A2A 2A2A 2A2A 2A00 2AFF"            /* .�*�*********.*� */	$"0000 0000 D8D8 D8D8 D8D8 0000 FFFF FFFF"            /* ....������..���� */	$"00FF 2A2A 0000 0000 0000 0000 002A 2AFF"            /* .�**.........**� */	$"0000 0000 0000 D8D8 D8D8 0000 FFFF FFFF"            /* ......����..���� */	$"00FF 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2AFF"            /* .�*************� */	$"0000 0000 0000 D8D8 D8D8 D8D8 FFFF FFFF"            /* ......���������� */	$"00FF 2A2A 2A2A 2A2A 2AFF FFFF FF2A 2AFF"            /* .�*******����**� */	$"0000 0000 D8D8 D8D8 F5F5 D8D8 0000 0000"            /* ....��������.... */	$"00FF 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2AFF"            /* .�*************� */	$"0000 0000 D8D8 0000 0000 D8D8 D8D8 0000"            /* ....��....����.. */	$"00FF 2AD8 2A2A 2A2A 2A2A 2A2A 2A2A 2AFF"            /* .�*�***********� */	$"0000 0000 D8D8 0000 0000 0000 D8D8 0000"            /* ....��......��.. */	$"00FF 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2AFF"            /* .�*************� */	$"0000 0000 D8D8 0000 0000 0000 D8D8 0000"            /* ....��......��.. */	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .��������������� */	$"0000 D8D8 D8D8 0000 0000 0000 D8D8 0000"            /* ..����......��.. */	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ..�������������. */	$"0000 D8D8 D8D8 0000 0000 0000 D8D8 0000"            /* ..����......��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 FFFF 0000 0000 0000 0000 FFFF 0000"            /* ..��........��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */	$"0000 FFFF 0000 0000 0000 0000 FFFF 0000"            /* ..��........��.. */	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */};data 'icl4' (128) {	$"0000 3333 3000 0000 0000 0000 0000 0000"            /* ..330........... */	$"0033 3333 3330 0000 0000 0000 0000 0000"            /* .33330.......... */	$"0033 3333 3330 0000 0000 0000 0000 0000"            /* .33330.......... */	$"0033 33FF FFF0 0000 0000 0000 0000 0000"            /* .33���.......... */	$"0033 3333 3330 0000 0000 0000 0000 0000"            /* .33330.......... */	$"0033 3333 3330 DD00 0000 0000 0000 0000"            /* .33330�......... */	$"0033 3333 3330 FF00 0000 0000 0000 0000"            /* .33330�......... */	$"0000 FF33 3330 DD00 0000 0000 0000 0000"            /* ..�330�......... */	$"0000 3300 0330 FF00 0000 0000 0000 0000"            /* ..3..0�......... */	$"0000 3300 0000 FF00 0000 0000 0000 0000"            /* ..3...�......... */	$"0000 3333 0022 2200 0000 0000 0000 0000"            /* ..33.""......... */	$"0033 3333 3300 2200 00FF FFFF FFFF FFF0"            /* .3333."..������� */	$"0033 3333 3300 FFFF 0FCC CCCC CCCC CCCF"            /* .3333.��.������� */	$"0033 3333 3333 FFFF 0FCC FFFF FFFF FCCF"            /* .33333��.������� */	$"0033 33FF 3333 FF00 0FCF CCCC CCCC C0CF"            /* .33�33�..������� */	$"0033 33FF FF00 FF00 0FCF CCCC CCCC C0CF"            /* .33��.�..������� */	$"0033 3333 3300 FF00 0FCF CCCC CCCC C0CF"            /* .3333.�..������� */	$"0033 3333 3300 2200 0FCF CCCC CCCC C0CF"            /* .3333."..������� */	$"0000 3333 3333 2200 0FCF CCCC CCCC C0CF"            /* ..3333"..������� */	$"0000 FF33 3333 2200 0FCF CCCC CCCC C0CF"            /* ..�333"..������� */	$"0000 33FF FF00 FF00 0FCF CCCC CCCC C0CF"            /* ..3��.�..������� */	$"0000 3333 3300 FFFF 0FCC 0000 0000 0CCF"            /* ..333.��.�.....� */	$"0000 0033 3300 FFFF 0FCC CCCC CCCC CCCF"            /* ...33.��.������� */	$"0000 0033 3333 FFFF 0FCC CCCC CFFF FCCF"            /* ...333��.������� */	$"0000 3333 0033 0000 0FCC CCCC CCCC CCCF"            /* ..33.3...������� */	$"0000 3300 0033 3300 0FC3 CCCC CCCC CCCF"            /* ..3..33..������� */	$"0000 3300 0000 3300 0FCC CCCC CCCC CCCF"            /* ..3...3..������� */	$"0000 3300 0000 3300 0FFF FFFF FFFF FFFF"            /* ..3...3..������� */	$"0033 3300 0000 3300 00FF FFFF FFFF FFF0"            /* .33...3..������� */	$"0033 3300 0000 3300 0000 0000 0000 0000"            /* .33...3......... */	$"00FF 0000 0000 FF00 0000 0000 0000 0000"            /* .�....�......... */	$"00FF 0000 0000 FF00 0000 0000 0000 0000"            /* .�....�......... */};data 'ics8' (128) {	$"0000 0000 D8D8 D8D8 D8D8 D8D8 0000 0000"            /* ....��������.... */	$"0000 0000 D8D8 D8D8 D8D8 D8D8 0000 0000"            /* ....��������.... */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 FFFF FFFF FFFF FFFF"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 FFFF FFFF FFFF FFFF"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8 D8D8"            /* ���������������� */	$"0000 0000 FFFF FFFF D8D8 D8D8 D8D8 D8D8"            /* ....������������ */	$"0000 0000 FFFF FFFF D8D8 D8D8 D8D8 D8D8"            /* ....������������ */	$"0000 0000 0000 0000 0000 0000 D8D8 D8D8"            /* ............���� */	$"0000 0000 0000 0000 0000 0000 D8D8 D8D8"            /* ............���� */};data 'ics4' (128) {	$"0000 3333 3333 0000 0000 3333 3333 0000"            /* ..3333....3333.. */	$"3333 3333 3333 3333 3333 3333 3333 3333"            /* 3333333333333333 */	$"3333 3333 3333 3333 3333 3333 3333 3333"            /* 3333333333333333 */	$"3333 3333 FFFF FFFF 3333 3333 FFFF FFFF"            /* 3333����3333���� */	$"3333 3333 3333 3333 3333 3333 3333 3333"            /* 3333333333333333 */	$"3333 3333 3333 3333 3333 3333 3333 3333"            /* 3333333333333333 */	$"0000 FFFF 3333 3333 0000 FFFF 3333 3333"            /* ..��3333..��3333 */	$"0000 0000 0000 3333 0000 0000 0000 3333"            /* ......33......33 */};data 'ics#' (128) {	$"0AA0 0550 AAAA 5555 AAAA 5555 ABFF 57FF"            /* .�.P��UU��UU��W� */	$"AAAA 5555 AAAA 5555 0FAA 0F55 000A 0005"            /* ��UU��UU.�.U.... */	$"0FF0 0FF0 FFFF FFFF FFFF FFFF FFFF FFFF"            /* .�.������������� */	$"FFFF FFFF FFFF FFFF 0FFF 0FFF 000F 000F"            /* ��������.�.�.... */};/* * About picture */data 'PICT' (rAboutPicture) {	$"0DD4 0000 0000 0070 0031 0011 02FF 0C00"            /* ��.....p.1...�.. */	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ��...H...H...... */	$"0070 0031 0000 0000 0001 000A 8001 8001"            /* .p.1........�.�. */	$"7FFF 7FFF 0098 8032 0000 0000 0070 0031"            /* .�.�.��2.....p.1 */	$"0000 0000 0000 0000 0048 0000 0048 0000"            /* .........H...H.. */	$"0000 0008 0001 0008 0000 0000 0185 4C70"            /* .............�Lp */	$"0000 0000 0000 1EE6 0000 00FF 0000 FFFF"            /* .......�...�..�� */	$"FFFF FFFF 0001 FFFF FFFF CCCC 0002 FFFF"            /* ����..������..�� */	$"FFFF 9999 0003 FFFF FFFF 6666 0004 FFFF"            /* ����..����ff..�� */	$"FFFF 3333 0005 FFFF FFFF 0000 0006 FFFF"            /* ��33..����....�� */	$"CCCC FFFF 0007 FFFF CCCC CCCC 0008 FFFF"            /* ����..������..�� */	$"CCCC 9999 0009 FFFF CCCC 6666 000A FFFF"            /* �̙�.�����ff..�� */	$"CCCC 3333 000B FFFF CCCC 0000 000C FFFF"            /* ��33..����....�� */	$"9999 FFFF 000D FFFF 9999 CCCC 000E FFFF"            /* ����.�������..�� */	$"9999 9999 000F FFFF 9999 6666 0010 FFFF"            /* ����..����ff..�� */	$"9999 3333 0011 FFFF 9999 0000 0012 FFFF"            /* ��33..����....�� */	$"6666 FFFF 0013 FFFF 6666 CCCC 0014 FFFF"            /* ff��..��ff��..�� */	$"6666 9999 0015 FFFF 6666 6666 0016 FFFF"            /* ff��..��ffff..�� */	$"6666 3333 0017 FFFF 6666 0000 0018 FFFF"            /* ff33..��ff....�� */	$"3333 FFFF 0019 FFFF 3333 CCCC 001A FFFF"            /* 33��..��33��..�� */	$"3333 9999 001B FFFF 3333 6666 001C FFFF"            /* 33��..��33ff..�� */	$"3333 3333 001D FFFF 3333 0000 001E FFFF"            /* 3333..��33....�� */	$"0000 FFFF 001F FFFF 0000 CCCC 0020 FFFF"            /* ..��..��..��. �� */	$"0000 9999 0021 FFFF 0000 6666 0022 FFFF"            /* ..��.!��..ff."�� */	$"0000 3333 0023 FFFF 0000 0000 0024 CCCC"            /* ..33.#��.....$�� */	$"FFFF FFFF 0025 CCCC FFFF CCCC 0026 CCCC"            /* ����.%������.&�� */	$"FFFF 9999 0027 CCCC FFFF 6666 0028 CCCC"            /* ����.'����ff.(�� */	$"FFFF 3333 0029 CCCC FFFF 0000 002A CCCC"            /* ��33.)����...*�� */	$"CCCC FFFF 002B CCCC CCCC CCCC 002C CCCC"            /* ����.+������.,�� */	$"CCCC 9999 002D CCCC CCCC 6666 002E CCCC"            /* �̙�.-����ff..�� */	$"CCCC 3333 002F CCCC CCCC 0000 0030 CCCC"            /* ��33./����...0�� */	$"9999 FFFF 0031 CCCC 9999 CCCC 0032 CCCC"            /* ����.1�̙���.2�� */	$"9999 9999 0033 CCCC 9999 6666 0034 CCCC"            /* ����.3�̙�ff.4�� */	$"9999 3333 0035 CCCC 9999 0000 0036 CCCC"            /* ��33.5�̙�...6�� */	$"6666 FFFF 0037 CCCC 6666 CCCC 0038 CCCC"            /* ff��.7��ff��.8�� */	$"6666 9999 0039 CCCC 6666 6666 003A CCCC"            /* ff��.9��ffff.:�� */	$"6666 3333 003B CCCC 6666 0000 003C CCCC"            /* ff33.;��ff...<�� */	$"3333 FFFF 003D CCCC 3333 CCCC 003E CCCC"            /* 33��.=��33��.>�� */	$"3333 9999 003F CCCC 3333 6666 0040 CCCC"            /* 33��.?��33ff.@�� */	$"3333 3333 0041 CCCC 3333 0000 0042 CCCC"            /* 3333.A��33...B�� */	$"0000 FFFF 0043 CCCC 0000 CCCC 0044 CCCC"            /* ..��.C��..��.D�� */	$"0000 9999 0045 CCCC 0000 6666 0046 CCCC"            /* ..��.E��..ff.F�� */	$"0000 3333 0047 CCCC 0000 0000 0048 9999"            /* ..33.G��.....H�� */	$"FFFF FFFF 0049 9999 FFFF CCCC 004A 9999"            /* ����.I������.J�� */	$"FFFF 9999 004B 9999 FFFF 6666 004C 9999"            /* ����.K����ff.L�� */	$"FFFF 3333 004D 9999 FFFF 0000 004E 9999"            /* ��33.M����...N�� */	$"CCCC FFFF 004F 9999 CCCC CCCC 0050 9999"            /* ����.O������.P�� */	$"CCCC 9999 0051 9999 CCCC 6666 0052 9999"            /* �̙�.Q����ff.R�� */	$"CCCC 3333 0053 9999 CCCC 0000 0054 9999"            /* ��33.S����...T�� */	$"9999 FFFF 0055 9999 9999 CCCC 0056 9999"            /* ����.U������.V�� */	$"9999 9999 0057 9999 9999 6666 0058 9999"            /* ����.W����ff.X�� */	$"9999 3333 0059 9999 9999 0000 005A 9999"            /* ��33.Y����...Z�� */	$"6666 FFFF 005B 9999 6666 CCCC 005C 9999"            /* ff��.[��ff��.\�� */	$"6666 9999 005D 9999 6666 6666 005E 9999"            /* ff��.]��ffff.^�� */	$"6666 3333 005F 9999 6666 0000 0060 9999"            /* ff33._��ff...`�� */	$"3333 FFFF 0061 9999 3333 CCCC 0062 9999"            /* 33��.a��33��.b�� */	$"3333 9999 0063 9999 3333 6666 0064 9999"            /* 33��.c��33ff.d�� */	$"3333 3333 0065 9999 3333 0000 0066 9999"            /* 3333.e��33...f�� */	$"0000 FFFF 0067 9999 0000 CCCC 0068 9999"            /* ..��.g��..��.h�� */	$"0000 9999 0069 9999 0000 6666 006A 9999"            /* ..��.i��..ff.j�� */	$"0000 3333 006B 9999 0000 0000 006C 6666"            /* ..33.k��.....lff */	$"FFFF FFFF 006D 6666 FFFF CCCC 006E 6666"            /* ����.mff����.nff */	$"FFFF 9999 006F 6666 FFFF 6666 0070 6666"            /* ����.off��ff.pff */	$"FFFF 3333 0071 6666 FFFF 0000 0072 6666"            /* ��33.qff��...rff */	$"CCCC FFFF 0073 6666 CCCC CCCC 0074 6666"            /* ����.sff����.tff */	$"CCCC 9999 0075 6666 CCCC 6666 0076 6666"            /* �̙�.uff��ff.vff */	$"CCCC 3333 0077 6666 CCCC 0000 0078 6666"            /* ��33.wff��...xff */	$"9999 FFFF 0079 6666 9999 CCCC 007A 6666"            /* ����.yff����.zff */	$"9999 9999 007B 6666 9999 6666 007C 6666"            /* ����.{ff��ff.|ff */	$"9999 3333 007D 6666 9999 0000 007E 6666"            /* ��33.}ff��...~ff */	$"6666 FFFF 007F 6666 6666 CCCC 0080 6666"            /* ff��..ffff��.�ff */	$"6666 9999 0081 6666 6666 6666 0082 6666"            /* ff��.�ffffff.�ff */	$"6666 3333 0083 6666 6666 0000 0084 6666"            /* ff33.�ffff...�ff */	$"3333 FFFF 0085 6666 3333 CCCC 0086 6666"            /* 33��.�ff33��.�ff */	$"3333 9999 0087 6666 3333 6666 0088 6666"            /* 33��.�ff33ff.�ff */	$"3333 3333 0089 6666 3333 0000 008A 6666"            /* 3333.�ff33...�ff */	$"0000 FFFF 008B 6666 0000 CCCC 008C 6666"            /* ..��.�ff..��.�ff */	$"0000 9999 008D 6666 0000 6666 008E 6666"            /* ..��.�ff..ff.�ff */	$"0000 3333 008F 6666 0000 0000 0090 3333"            /* ..33.�ff.....�33 */	$"FFFF FFFF 0091 3333 FFFF CCCC 0092 3333"            /* ����.�33����.�33 */	$"FFFF 9999 0093 3333 FFFF 6666 0094 3333"            /* ����.�33��ff.�33 */	$"FFFF 3333 0095 3333 FFFF 0000 0096 3333"            /* ��33.�33��...�33 */	$"CCCC FFFF 0097 3333 CCCC CCCC 0098 3333"            /* ����.�33����.�33 */	$"CCCC 9999 0099 3333 CCCC 6666 009A 3333"            /* �̙�.�33��ff.�33 */	$"CCCC 3333 009B 3333 CCCC 0000 009C 3333"            /* ��33.�33��...�33 */	$"9999 FFFF 009D 3333 9999 CCCC 009E 3333"            /* ����.�33����.�33 */	$"9999 9999 009F 3333 9999 6666 00A0 3333"            /* ����.�33��ff.�33 */	$"9999 3333 00A1 3333 9999 0000 00A2 3333"            /* ��33.�33��...�33 */	$"6666 FFFF 00A3 3333 6666 CCCC 00A4 3333"            /* ff��.�33ff��.�33 */	$"6666 9999 00A5 3333 6666 6666 00A6 3333"            /* ff��.�33ffff.�33 */	$"6666 3333 00A7 3333 6666 0000 00A8 3333"            /* ff33.�33ff...�33 */	$"3333 FFFF 00A9 3333 3333 CCCC 00AA 3333"            /* 33��.�3333��.�33 */	$"3333 9999 00AB 3333 3333 6666 00AC 3333"            /* 33��.�3333ff.�33 */	$"3333 3333 00AD 3333 3333 0000 00AE 3333"            /* 3333.�3333...�33 */	$"0000 FFFF 00AF 3333 0000 CCCC 00B0 3333"            /* ..��.�33..��.�33 */	$"0000 9999 00B1 3333 0000 6666 00B2 3333"            /* ..��.�33..ff.�33 */	$"0000 3333 00B3 3333 0000 0000 00B4 0000"            /* ..33.�33.....�.. */	$"FFFF FFFF 00B5 0000 FFFF CCCC 00B6 0000"            /* ����.�..����.�.. */	$"FFFF 9999 00B7 0000 FFFF 6666 00B8 0000"            /* ����.�..��ff.�.. */	$"FFFF 3333 00B9 0000 FFFF 0000 00BA 0000"            /* ��33.�..��...�.. */	$"CCCC FFFF 00BB 0000 CCCC CCCC 00BC 0000"            /* ����.�..����.�.. */	$"CCCC 9999 00BD 0000 CCCC 6666 00BE 0000"            /* �̙�.�..��ff.�.. */	$"CCCC 3333 00BF 0000 CCCC 0000 00C0 0000"            /* ��33.�..��...�.. */	$"9999 FFFF 00C1 0000 9999 CCCC 00C2 0000"            /* ����.�..����.�.. */	$"9999 9999 00C3 0000 9999 6666 00C4 0000"            /* ����.�..��ff.�.. */	$"9999 3333 00C5 0000 9999 0000 00C6 0000"            /* ��33.�..��...�.. */	$"6666 FFFF 00C7 0000 6666 CCCC 00C8 0000"            /* ff��.�..ff��.�.. */	$"6666 9999 00C9 0000 6666 6666 00CA 0000"            /* ff��.�..ffff.�.. */	$"6666 3333 00CB 0000 6666 0000 00CC 0000"            /* ff33.�..ff...�.. */	$"3333 FFFF 00CD 0000 3333 CCCC 00CE 0000"            /* 33��.�..33��.�.. */	$"3333 9999 00CF 0000 3333 6666 00D0 0000"            /* 33��.�..33ff.�.. */	$"3333 3333 00D1 0000 3333 0000 00D2 0000"            /* 3333.�..33...�.. */	$"0000 FFFF 00D3 0000 0000 CCCC 00D4 0000"            /* ..��.�....��.�.. */	$"0000 9999 00D5 0000 0000 6666 00D6 0000"            /* ..��.�....ff.�.. */	$"0000 3333 00D7 EEEE 0000 0000 00D8 DDDD"            /* ..33.���.....��� */	$"0000 0000 00D9 BBBB 0000 0000 00DA AAAA"            /* .....ٻ�.....ڪ� */	$"0000 0000 00DB 8888 0000 0000 00DC 7777"            /* .....ۈ�.....�ww */	$"0000 0000 00DD 5555 0000 0000 00DE 4444"            /* .....�UU.....�DD */	$"0000 0000 00DF 2222 0000 0000 00E0 1111"            /* .....�"".....�.. */	$"0000 0000 00E1 0000 EEEE 0000 00E2 0000"            /* .....�..��...�.. */	$"DDDD 0000 00E3 0000 BBBB 0000 00E4 0000"            /* ��...�..��...�.. */	$"AAAA 0000 00E5 0000 8888 0000 00E6 0000"            /* ��...�..��...�.. */	$"7777 0000 00E7 0000 5555 0000 00E8 0000"            /* ww...�..UU...�.. */	$"4444 0000 00E9 0000 2222 0000 00EA 0000"            /* DD...�..""...�.. */	$"1111 0000 00EB 0000 0000 EEEE 00EC 0000"            /* .....�....��.�.. */	$"0000 DDDD 00ED 0000 0000 BBBB 00EE 0000"            /* ..��.�....��.�.. */	$"0000 AAAA 00EF 0000 0000 8888 00F0 0000"            /* ..��.�....��.�.. */	$"0000 7777 00F1 0000 0000 5555 00F2 0000"            /* ..ww.�....UU.�.. */	$"0000 4444 00F3 0000 0000 2222 00F4 0000"            /* ..DD.�...."".�.. */	$"0000 1111 00F5 EEEE EEEE EEEE 00F6 DDDD"            /* .....�������.��� */	$"DDDD DDDD 00F7 BBBB BBBB BBBB 00F8 AAAA"            /* ����.�������.��� */	$"AAAA AAAA 00F9 8888 8888 8888 00FA 7777"            /* ����.�������.�ww */	$"7777 7777 00FB 5555 5555 5555 00FC 4444"            /* wwww.�UUUUUU.�DD */	$"4444 4444 00FD 2222 2222 2222 00FE 1111"            /* DDDD.�"""""".�.. */	$"1111 1111 00FF 0000 0000 0000 0000 0000"            /* .....�.......... */	$"0070 0031 0000 0000 0070 0031 0040 04D0"            /* .p.1.....p.1.@.� */	$"F600 DD04 D0F6 00DD 04D0 F600 DD04 D0F6"            /* �.�.��.�.��.�.�� */	$"00DD 08F7 F6F2 D8E9 F600 DD08 F7F6 F2D8"            /* .�.������.�.���� */	$"E9F6 00DD 08F7 F6F2 D8E9 F600 DD08 FDF6"            /* ��.�.������.�.�� */	$"E6D8 EFF6 00DD 08FD F6E6 D8EF F600 DD08"            /* ����.�.������.�. */	$"FDF6 E6D8 EFF6 00DD 08FD F6E6 D8EF F600"            /* ������.�.������. */	$"DD08 FDF6 E6D8 EFF6 0099 08FD F6E6 D8EF"            /* �.������.�.����� */	$"F600 990A FDF6 F5D8 F2FF EFF6 0099 0AFD"            /* �.�.��������.�.� */	$"F6F5 D8F2 FFEF F600 990A FDF6 F5D8 F2FF"            /* �������.�.������ */	$"EFF6 0099 08FD F6E6 D8EF F600 9908 FDF6"            /* ��.�.������.�.�� */	$"E6D8 EFF6 0099 08FD F6E6 D8EF F600 990C"            /* ����.�.������.�. */	$"FDF6 E6D8 FEF6 FBFA F8F6 0099 0CFD F6E6"            /* ����������.�.��� */	$"D8FE F6FB FAF8 F600 990C FDF6 E6D8 FEF6"            /* �������.�.������ */	$"FBFA F8F6 0099 0CFD F6E6 D8FE F6FB FFF8"            /* ����.�.��������� */	$"F600 990C FDF6 E6D8 FEF6 FBFF F8F6 0099"            /* �.�.����������.� */	$"0CFD F6E6 D8FE F6FB FFF8 F600 990E F7F6"            /* .����������.�.�� */	$"FBFF F2D8 FEF6 FBFA F8F6 0099 0EF7 F6FB"            /* ����������.�.��� */	$"FFF2 D8FE F6FB FAF8 F600 990E F7F6 FBFF"            /* ���������.�.���� */	$"F2D8 FEF6 FBFA F8F6 0099 10F7 F6FB D8F8"            /* ��������.�.����� */	$"F6FB D8FE F6FB FFF8 F600 9910 F7F6 FBD8"            /* ���������.�.���� */	$"F8F6 FBD8 FEF6 FBFF F8F6 0099 10F7 F6FB"            /* ����������.�.��� */	$"D8F8 F6FB D8FE F6FB FFF8 F600 990C F7F6"            /* �����������.�.�� */	$"FBD8 EFF6 FBFF F8F6 0099 0CF7 F6FB D8EF"            /* ��������.�.����� */	$"F6FB FFF8 F600 990C F7F6 FBD8 EFF6 FBFF"            /* �����.�.�������� */	$"F8F6 0099 0CF7 F6F5 D8FB F6F5 16F8 F600"            /* ��.�.�������.��. */	$"990C F7F6 F5D8 FBF6 F516 F8F6 0099 0CF7"            /* �.�������.��.�.� */	$"F6F5 D8FB F6F5 16F8 F600 990C FDF6 E9D8"            /* ������.��.�.���� */	$"FBF6 FB16 F8F6 0099 0CFD F6E9 D8FB F6FB"            /* ���.��.�.������� */	$"16F8 F600 990C FDF6 E9D8 FBF6 FB16 F8F6"            /* .��.�.�������.�� */	$"0099 0CFD F6E9 D8FB F6F5 FFFE F600 990C"            /* .�.����������.�. */	$"FDF6 E9D8 FBF6 F5FF FEF6 0099 0CFD F6E9"            /* ����������.�.��� */	$"D8FB F6F5 FFFE F600 990A FDF6 E3D8 F5FF"            /* �������.�.������ */	$"FEF6 0099 0AFD F6E3 D8F5 FFFE F600 990A"            /* ��.�.��������.�. */	$"FDF6 E3D8 F5FF FEF6 0099 0EFD F6F5 D8FB"            /* ��������.�.����� */	$"FFF5 D8FB FFF8 F600 990E FDF6 F5D8 FBFF"            /* �������.�.������ */	$"F5D8 FBFF F8F6 0099 0EFD F6F5 D8FB FFF5"            /* ������.�.������� */	$"D8FB FFF8 F600 990E FDF6 F5D8 F5FF FBF6"            /* �����.�.�������� */	$"FBFF F8F6 0099 0EFD F6F5 D8F5 FFFB F6FB"            /* ����.�.��������� */	$"FFF8 F600 990E FDF6 F5D8 F5FF FBF6 FBFF"            /* ���.�.���������� */	$"F8F6 0099 0CFD F6E9 D8FB F6FB FFF8 F600"            /* ��.�.����������. */	$"990C FDF6 E9D8 FBF6 FBFF F8F6 0099 0CFD"            /* �.����������.�.� */	$"F6E9 D8FB F6FB FFF8 F600 990C FDF6 E9D8"            /* ���������.�.���� */	$"FBF6 FB16 F8F6 0099 0CFD F6E9 D8FB F6FB"            /* ���.��.�.������� */	$"16F8 F600 990C FDF6 E9D8 FBF6 FB16 F8F6"            /* .��.�.�������.�� */	$"0099 0AF7 F6E9 D8FB 16F8 F600 990A F7F6"            /* .�.�����.��.�.�� */	$"E9D8 FB16 F8F6 0099 0AF7 F6E9 D8FB 16F8"            /* ���.��.�.�����.� */	$"F600 990C F7F6 FBFF EFD8 FB16 F8F6 0099"            /* �.�.�������.��.� */	$"0CF7 F6FB FFEF D8FB 16F8 F600 000C F7F6"            /* .�������.��...�� */	$"FBFF EFD8 FB16 F8F6 0000 0EF7 F6FB D8F5"            /* �����.��...����� */	$"FFFB F6FB FFF8 F600 000E F7F6 FBD8 F5FF"            /* �������...������ */	$"FBF6 FBFF F8F6 0000 0EF7 F6FB D8F5 FFFB"            /* ������...������� */	$"F6FB FFF8 F600 000C F7F6 EFD8 FBF6 F5FF"            /* �����...�������� */	$"FEF6 0000 0CF7 F6EF D8FB F6F5 FFFE F600"            /* ��...����������. */	$"000C F7F6 EFD8 FBF6 F5FF FEF6 0000 0CF1"            /* ..����������...� */	$"F6F5 D8FB F6F5 FFFE F600 000C F1F6 F5D8"            /* ���������...���� */	$"FBF6 F5FF FEF6 0000 0CF1 F6F5 D8FB F6F5"            /* ������...������� */	$"FFFE F600 000A F1F6 EFD8 F5FF FEF6 0000"            /* ���...��������.. */	$"0AF1 F6EF D8F5 FFFE F600 000A F1F6 EFD8"            /* .��������...���� */	$"F5FF FEF6 0060 0CF7 F6F5 D8FB F5FB D8F2"            /* ����.`.��������� */	$"F600 4D0C F7F6 F5D8 FBF5 FBD8 F2F6 0010"            /* �.M.����������.. */	$"0CF7 F6F5 D8FB F5FB D8F2 F600 4D0C F7F6"            /* .����������.M.�� */	$"FBD8 F5F6 F5D8 F8F6 0000 0CF7 F6FB D8F5"            /* ��������...����� */	$"F6F5 D8F8 F600 6B0C F7F6 FBD8 F5F6 F5D8"            /* �����.k.�������� */	$"F8F6 000F 0CF7 F6FB D8EF F6FB D8F8 F600"            /* ��...����������. */	$"000C F7F6 FBD8 EFF6 FBD8 F8F6 0044 0CF7"            /* ..����������.D.� */	$"F6FB D8EF F6FB D8F8 F600 4D0C F7F6 FBD8"            /* ���������.M.���� */	$"EFF6 FBD8 F8F6 004D 0CF7 F6FB D8EF F6FB"            /* ������.M.������� */	$"D8F8 F600 090C F7F6 FBD8 EFF6 FBD8 F8F6"            /* ���.�.���������� */	$"0000 0CFD F6F5 D8EF F6FB D8F8 F600 4D0C"            /* ...����������.M. */	$"FDF6 F5D8 EFF6 FBD8 F8F6 0000 0CFD F6F5"            /* ����������...��� */	$"D8EF F6FB D8F8 F600 570C FDF6 F5D8 EFF6"            /* �������.W.������ */	$"FBD8 F8F6 004D 0CFD F6F5 D8EF F6FB D8F8"            /* ����.M.��������� */	$"F600 4D0C FDF6 F5D8 EFF6 FBD8 F8F6 0073"            /* �.M.����������.s */	$"0CFD F6FB FFE9 F6FB FFF8 F600 4D0C FDF6"            /* .����������.M.�� */	$"FBFF E9F6 FBFF F8F6 0000 0CFD F6FB FFE9"            /* ��������...����� */	$"F6FB FFF8 F600 8A0C FDF6 FBFF E9F6 FBFF"            /* �����.�.�������� */	$"F8F6 0000 0CFD F6FB FFE9 F6FB FFF8 F600"            /* ��...����������. */	$"000C FDF6 FBFF E9F6 FBFF F8F6 0080 0CFD"            /* ..����������.�.� */	$"F6FB FFE9 F6FB FFF8 F600 4D0C FDF6 FBFF"            /* ���������.M.���� */	$"E9F6 FBFF F8F6 004D 0CFD F6FB FFE9 F6FB"            /* ������.M.������� */	$"FFF8 F600 6E0C FDF6 FBFF E9F6 FBFF F8F6"            /* ���.n.���������� */	$"0074 0CFD F6FB FFE9 F6FB FFF8 F600 4F0C"            /* .t.����������.O. */	$"FDF6 FBFF E9F6 FBFF F8F6 004E 0CFD F6F5"            /* ����������.N.��� */	$"FFEF F6F5 FFFE F600 640C FDF6 F5FF EFF6"            /* �������.d.������ */	$"F5FF FEF6 000C 0CFD F6F5 FFEF F6F5 FFFE"            /* ����...��������� */	$"F600 0004 D0F6 0000 04D0 F600 7404 D0F6"            /* �...��...��.t.�� */	$"0049 00FF"                                          /* .I.� */};