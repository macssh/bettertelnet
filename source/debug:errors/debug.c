// debug.c// BetterTelnet// copyright 1997, 1998, 1999 Rolf Braun// This is free software under the GNU General Public License (GPL). See the file COPYING// which comes with the source code and documentation distributions for details.// based on NCSA Telnet 2.7b5#include "vsdata.h"#include "vsinterf.proto.h"		// For VSwrite proto #include "wind.h"			    // For WindRec structure#include "parse.proto.h"		// For DemangleLinemodeShort#include "rsinterf.proto.h"		// For RSshow proto#include "linemode.proto.h"#include "prefs.proto.h"WindRec	*console;		//	Window Record (VS) for	console Window   extern WindRec	*screens;#define DEBUG_FACILITIESvoid InitDebug(void){#ifdef DEBUG_FACILITIES	Rect pRect;	TerminalPrefs **termHdl;	Boolean scratchBoolean;	console = (WindRec *) myNewPtr(sizeof(WindRec));	//	SetRect(&pRect, 50, 150, 700, 350);		// Need to make this a resource!	SetRect(&pRect, 50, 150, 0, 0);		console->vs=RSnewwindow( &pRect, 350, 80, 24,					"\p<console>", 1, DefFONT, DefSIZE, TelInfo->debug,0,0,0,0,0,1, DefFONT, DefSIZE, 0, 0, 1, 0, 0);	/* NCSA 2.5 */	console->wind = RSgetwindow( console->vs);	((WindowPeek)console->wind)->windowKind = WIN_CONSOLE;//	VSwrite(console->vs,"\033[24;0H",7);	console->active=0;	console->port=0;	console->termstate=VTEKTYPE;	console->national = 0;			/* LU: no translation */	UseResFile(TelInfo->SettingsFile);	termHdl = (TerminalPrefs **)Get1NamedSizedResource				(TERMINALPREFS_RESTYPE, "\p<Default>", sizeof(TerminalPrefs));	DetachResource((Handle) termHdl);	HLock((Handle)termHdl);		scratchBoolean = RSsetcolor( console->vs, 0, (*termHdl)->nfcolor);	scratchBoolean = RSsetcolor( console->vs, 1, (*termHdl)->nbcolor);	scratchBoolean = RSsetcolor( console->vs, 2, (*termHdl)->bfcolor);	scratchBoolean = RSsetcolor( console->vs, 3, (*termHdl)->bbcolor);	DisposeHandle((Handle)termHdl);#else	console = NULL;#endif}void putln( char *cp)								{#ifdef DEBUG_FACILITIES	short temp;	if (!TelInfo->debug)		return;	temp=strlen(cp);	if (temp>80) return;	VSwrite(console->vs,cp,temp);	VSwrite(console->vs,"\015\012",2);#endif}// Called by HandleKeyDown.  Allows me to insert debug info keys all in one place//	that can be easily #defined out for release versions.  Returns TRUE if//	HandleKeyDown should do an immediate return after calling us.Boolean	DebugKeys(Boolean cmddwn, unsigned char ascii, short s){#ifdef DEBUG_FACILITIES	if (cmddwn && (ascii == ';')) {	// 2.6b16.1		char hackhackhack[80];				strcpy(hackhackhack, "Linemode: ");		DemangleLineModeShort(hackhackhack, screens[s].lmodeBits);		putln(hackhackhack);		return(FALSE);		}	if (cmddwn && (ascii == 39)) { //single quote		if (TelInfo->debug) HideDebugWindow();		else ShowDebugWindow();		return(FALSE);	}#endif	return (FALSE);}void	ShowDebugWindow(void){#ifdef DEBUG_FACILITIES		if (console != NULL)	{		TelInfo->debug = TRUE;		RSshow(console->vs);	}#endif}void	HideDebugWindow(void){#ifdef DEBUG_FACILITIES		if (console != NULL)	{		TelInfo->debug = FALSE;		RShide(console->vs);	}#endif}