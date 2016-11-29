// Connections.c// BetterTelnet// copyright 1997 Rolf Braun// This is free software under the GNU General Public License (GPL). See the file COPYING// which comes with the source code and documentation distributions for details.// based on NCSA Telnet 2.7b5#include "telneterrors.h"#include "DlogUtils.proto.h"#include "movableModal.h"#include "wind.h"#include "event.proto.h"#include "rsinterf.proto.h"#include "vsdata.h"#include "vskeys.h"#include "vsinterf.proto.h"#include "vgtek.proto.h"#include "tekrgmac.proto.h"#include "vr.h"#include "vrrgmac.proto.h" #include "network.proto.h"#include "mydnr.proto.h"#include "InternalEvents.h"#include "menuseg.proto.h"#include "maclook.proto.h"#include "parse.proto.h"#include "parse.h"#include "configure.proto.h"#include "netevent.proto.h"#include "linemode.proto.h"#include "mainseg.proto.h"#include "prefs.proto.h"#include "popup.h"#include "popup.proto.h"#include "Connections.proto.h"#include "tnae.h"#include "authencrypt.h"#include "authencrypt.proto.h"#include "wdefpatch.proto.h"#include "LinkedList.proto.h"/*	These are all of the variables we need... */extern	Cursor	*theCursors[NUMCURS];		/* all the cursors in a nice bundle */extern	WindRec	*screens;extern	short	scrn;extern	short	nNational;				// Number of user-installed translation tablesextern	MenuHandle	myMenus[];extern	Boolean	authOK;extern	Boolean	encryptOK;extern	unsigned char *gReadspace;extern	short	gBlocksize;static	short	WindByPort(short port);static	void setSessStates(DialogPtr dptr);static	pascal short POCdlogfilter( DialogPtr dptr, EventRecord *evt, short *item);PROTO_UPP(POCdlogfilter, ModalFilter);void OpenPortSpecial(MenuHandle menuh, short item){	ConnInitParams		**theParams;	Boolean				success;	Str255				scratchPstring;		GetItem(menuh, item, scratchPstring);		theParams = NameToConnInitParams(scratchPstring, TRUE, 0);	if (theParams == NULL) {		OutOfMemory(1020);		return;		}			success = CreateConnectionFromParams(theParams);}SIMPLE_UPP(POCdlogfilter, ModalFilter);pascal short POCdlogfilter( DialogPtr dptr, EventRecord *evt, short *item){	short key;	if (evt->what == keyDown) {		key = evt->message & charCodeMask;		if ( ((key == 'A') || (key == 'a')) && (evt->modifiers & cmdKey) ) {			*item = NCauthenticate;			return(-1);		}		if ( ((key == 'E') || (key == 'e')) && (evt->modifiers & cmdKey) ) {			*item = NCencrypt;			return(-1);		}	}	if ((evt->what == keyDown) || (evt->what == autoKey)) {		key = evt->message & charCodeMask;		if (key == 0x1F) {			*item = 1000;			return(-1);		}		if (key == 0x1E) {			*item = 1001;			return(-1);		}	}//	if (evt->what == mouseDown)//		return(PopupMousedown(dptr, evt, item));// RAB BetterTelnet 1.2 - we let StdFilterProc handle this now//	return(DLOGwOK_Cancel(dptr, evt, item));	return CallStdFilterProc(dptr, evt, item);}void	PresentOpenConnectionDialog(void){	ConnInitParams	**InitParams;	DialogPtr		dptr;	short			ditem, scratchshort, rolfito;	Boolean			success;	long			scratchlong;	Str255			scratchPstring, terminalPopupString, scritchPstring;	Handle			ItemHandle;	SessionPrefs	**defaultSessHdl,**tempSessHdl;	short 			numberOfTerms, sessMark, requestPort;	MenuHandle		SessPopupHdl, TermPopupHdl;	Rect			scratchRect;	Point			SessPopupLoc;	short			TerminalIndex, itemNumber = 1;	popup TPopup[] = {{NCtermpopup, (MenuHandle) 0, 1},						{0, (MenuHandle) 0, 0}};	Size 			junk;	LinkedListNode	*theHead;		SetCursor(theCursors[normcurs]);	SetUpMovableModalMenus();	dptr = GetNewMyDialog(NewCnxnDLOG, NULL, kInFront, (void *)ThirdCenterDialog);	if (dptr == NULL) {		OutOfMemory(1000);		return;		}			SetDialogDefaultItem(dptr, 1);	SetDialogCancelItem(dptr, 2);	SetDialogTracksCursor(dptr, 1);	ditem = 3;	sessMark = 1;		GetIndString(scratchPstring,MISC_STRINGS,SESSION_STRING);	SessPopupHdl = NewMenu(668, scratchPstring);	if (SessPopupHdl == NULL) {		DisposeDialog(dptr);		OutOfMemory(1000);		return;		}	UseResFile(TelInfo->SettingsFile);	numberOfTerms = Count1Resources(SESSIONPREFS_RESTYPE);	theHead  = createSortedList(SESSIONPREFS_RESTYPE,numberOfTerms,"\p<Default>");	EnableItem(SessPopupHdl, 0);		// Make sure the entire menu is enabled	addListToMenu(SessPopupHdl, theHead, 1);	deleteList(&theHead);	SetItemMark(SessPopupHdl, 1, 18);	GetDItem(dptr, NCsesspopup, &scratchshort, &ItemHandle, &scratchRect);	SessPopupLoc.h = scratchRect.left;	SessPopupLoc.v = scratchRect.top;	//	TermPopupHdl = NewMenu(666, "\p");//	if (TermPopupHdl == NULL) {//		DisposeHandle((Handle)SessPopupHdl);//		DisposeDialog(dptr);//		OutOfMemory(1000);//		return;//		}	SetPort(dptr);	LocalToGlobal(&SessPopupLoc);//	numberOfTerms = Count1Resources(TERMINALPREFS_RESTYPE);//	theHead  = createSortedList(TERMINALPREFS_RESTYPE,numberOfTerms,"\p<Default>");//	addListToMenu(TermPopupHdl, theHead);//	deleteList(&theHead);//	TPopup[0].h = TermPopupHdl;//	PopupInit(dptr, TPopup);		// Get default auth/encrypt settings from default session	defaultSessHdl = GetDefaultSession();	HLock((Handle)defaultSessHdl);	BlockMove("\p<Default>", scratchPstring, 15);	GetHostNameFromSession(scratchPstring);	if ((**defaultSessHdl).port != 23) {			NumToString((unsigned short)(**defaultSessHdl).port, scritchPstring);			pstrcat(scratchPstring, "\p:");			if ((**defaultSessHdl).portNegative)				pstrcat(scratchPstring, "\p-");			pstrcat(scratchPstring, scritchPstring);	}	SetTEText(dptr, NChostname, scratchPstring);	SelIText(dptr, NChostname, 0, 32767);	SetCntrl(dptr, NCauthenticate, (**defaultSessHdl).authenticate && authOK);	SetCntrl(dptr, NCencrypt, (**defaultSessHdl).encrypt && encryptOK);	if (!authOK)	{		Hilite( dptr, NCauthenticate, 255);		Hilite( dptr, NCencrypt, 255);	}		//	TerminalIndex = findPopupMenuItem(TermPopupHdl,(**defaultSessHdl).TerminalEmulation);//	TPopup[0].choice = TerminalIndex;//	PopupInit(dptr, TPopup);	DisposeHandle((Handle)defaultSessHdl);	setSessStates(dptr);	while (ditem > NCcancel) {		movableModalDialog(POCdlogfilterUPP, &ditem);		switch(ditem) 		{			case	NCauthenticate:			case	NCencrypt:				GetDItem(dptr, ditem, &scratchshort, &ItemHandle, &scratchRect);				if ((**(ControlHandle)ItemHandle).contrlHilite == 0) {	// if control not disabled					FlipCheckBox(dptr, ditem);					setSessStates(dptr);				}				break;			case	NCsesspopup:				GetDItem(dptr, NCsesspopup, &scratchshort, &ItemHandle, &scratchRect);				SessPopupLoc.h = scratchRect.left;				SessPopupLoc.v = scratchRect.top;				SetPort(dptr);				LocalToGlobal(&SessPopupLoc);				InsertMenu(SessPopupHdl, hierMenu);				CalcMenuSize(SessPopupHdl);				scratchlong = PopUpMenuSelect(SessPopupHdl, SessPopupLoc.v,												SessPopupLoc.h, 0);				DeleteMenu(668);				if (scratchlong) 				{					scratchshort = scratchlong & 0xFFFF; //	Apple sez ignore the high word					SetItemMark(SessPopupHdl, sessMark, 0);					sessMark = scratchshort;					SetItemMark(SessPopupHdl, sessMark, 18);					GetItem(SessPopupHdl, scratchshort, scratchPstring);					tempSessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, scratchPstring);					if (tempSessHdl) 					{//						TerminalIndex = findPopupMenuItem(TermPopupHdl,//								(**tempSessHdl).TerminalEmulation);//						TPopup[0].choice = TerminalIndex;//						DrawPopUp(dptr, NCtermpopup); //update popup						strcpy((char *)scratchPstring, (char *)(**tempSessHdl).hostname);						if ((**tempSessHdl).port != 23) {								NumToString((unsigned short)(**tempSessHdl).port, scritchPstring);								pstrcat(scratchPstring, "\p:");								if ((**tempSessHdl).portNegative)									pstrcat(scratchPstring, "\p-");								pstrcat(scratchPstring, scritchPstring);						}						SetTEText(dptr, NChostname, scratchPstring);//update the hostname						SelIText(dptr, NChostname, 0, 32767);						SetCntrl(dptr, NCauthenticate, (**tempSessHdl).authenticate && authOK);//update the auth status						SetCntrl(dptr, NCencrypt, (**tempSessHdl).encrypt && encryptOK);						setSessStates(dptr);//encrypt cant be on w/o authenticate						ReleaseResource((Handle)tempSessHdl);					}				}				break;			case 1001:				SetItemMark(SessPopupHdl, sessMark, 0);				sessMark--;				if (sessMark < 1)					sessMark = CountMItems(SessPopupHdl);				SetItemMark(SessPopupHdl, sessMark, 18);				GetItem(SessPopupHdl, sessMark, scratchPstring);				tempSessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, scratchPstring);				if (tempSessHdl) {					strcpy((char *)scratchPstring, (char *)(**tempSessHdl).hostname);					if ((**tempSessHdl).port != 23) {							NumToString((unsigned short)(**tempSessHdl).port, scritchPstring);							pstrcat(scratchPstring, "\p:");							if ((**tempSessHdl).portNegative)								pstrcat(scratchPstring, "\p-");							pstrcat(scratchPstring, scritchPstring);					}					SetTEText(dptr, NChostname, scratchPstring);//update the hostname					SelIText(dptr, NChostname, 0, 32767);					SetCntrl(dptr, NCauthenticate, (**tempSessHdl).authenticate && authOK);//update the auth status					SetCntrl(dptr, NCencrypt, (**tempSessHdl).encrypt && encryptOK);					setSessStates(dptr);//encrypt cant be on w/o authenticate					ReleaseResource((Handle)tempSessHdl);				}				break;			case 1000:				SetItemMark(SessPopupHdl, sessMark, 0);				sessMark++;				if (sessMark > CountMItems(SessPopupHdl))					sessMark = 1;				SetItemMark(SessPopupHdl, sessMark, 18);				GetItem(SessPopupHdl, sessMark, scratchPstring);				tempSessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, scratchPstring);				if (tempSessHdl) {					strcpy((char *)scratchPstring, (char *)(**tempSessHdl).hostname);					if ((**tempSessHdl).port != 23) {							NumToString((unsigned short)(**tempSessHdl).port, scritchPstring);							pstrcat(scratchPstring, "\p:");							if ((**tempSessHdl).portNegative)								pstrcat(scratchPstring, "\p-");							pstrcat(scratchPstring, scritchPstring);					}					SetTEText(dptr, NChostname, scratchPstring);//update the hostname					SelIText(dptr, NChostname, 0, 32767);					SetCntrl(dptr, NCauthenticate, (**tempSessHdl).authenticate && authOK);//update the auth status					SetCntrl(dptr, NCencrypt, (**tempSessHdl).encrypt && encryptOK);					setSessStates(dptr);//encrypt cant be on w/o authenticate					ReleaseResource((Handle)tempSessHdl);				}				break;			default:				break;		} // switch	} // while			if (ditem == NCcancel) {//		PopupCleanup();		DisposeMenu(SessPopupHdl);	// drh � Bug fix: memory leak		DisposeDialog(dptr);		ResetMenus();		return;		}		GetTEText(dptr, NChostname, scratchPstring);	if (!Length(scratchPstring)) {//		PopupCleanup();		DisposeMenu(SessPopupHdl);	// drh � Bug fix: memory leak		DisposeDialog(dptr);		ResetMenus();		return;		}	//	GetItem(TPopup[0].h, TPopup[0].choice, terminalPopupString);//	PopupCleanup();	for (rolfito = 0; rolfito < Length(scratchPstring); rolfito++)		if (scratchPstring[rolfito + 1] == ':')			scratchPstring[rolfito + 1] = ' ';	MaxMem(&junk);	GetItem(SessPopupHdl, sessMark, scritchPstring);	InitParams = NameToConnInitParams(scratchPstring, FALSE, scritchPstring);	if (InitParams == NULL)	{		DisposeMenu(SessPopupHdl);	// drh � Bug fix: memory leak		DisposeDialog(dptr);		OutOfMemory(1000);		return;		}//	if ((**InitParams).terminal == NULL)  //if this is not null, then the string was an alias,//	{										// so dont use the popup terminal//		(**InitParams).terminal = (TerminalPrefs **)//				Get1NamedResource(TERMINALPREFS_RESTYPE,terminalPopupString);//		DetachResource((Handle)(**InitParams).terminal);//		if (InitParams == NULL) {//			OutOfMemory(1000);//			DisposeDialog(dptr);//			return;//			}	 	if (GetCntlVal(dptr, NCauthenticate))	  		(**(**InitParams).session).authenticate = 1;	 	else	 		(**(**InitParams).session).authenticate = 0;	 	if (GetCntlVal(dptr, NCencrypt))	  		(**(**InitParams).session).encrypt = 1;	 	else	 		(**(**InitParams).session).encrypt = 0;//	}	HLock((Handle)InitParams);	HLock((Handle)(**InitParams).session);	GetTEText(dptr, NCwindowname, scratchPstring);	// Copy over the user specified window name.  If blank, CreateConnectionFromParams 	// will copy the hostname to the windowname and append a number.	if (Length(scratchPstring)) 		BlockMove(scratchPstring, (**InitParams).WindowName,					(Length(scratchPstring) > 63) ? 64 : (Length(scratchPstring) + 1));	HUnlock((Handle)(**InitParams).session);	HUnlock((Handle)InitParams);	DisposeMenu(SessPopupHdl);	// drh � Bug fix: memory leak	DisposeDialog(dptr);	ResetMenus();		success = CreateConnectionFromParams(InitParams);}// Set states of session checkboxesstatic	void setSessStates (DialogPtr dptr){			if (GetCntlVal(dptr, NCauthenticate)) {		Hilite(dptr, NCencrypt, (encryptOK)? 0 : 255);	} else {		Hilite(dptr, NCencrypt, 255);		SetCntrl(dptr, NCencrypt, false);	}}void	OpenConnectionFromURL(char *host, char *portstring, char *user, char *password){	ConnInitParams	**Params;	Str255			windowName, tempString;	short			len;	long			port;		Params = ReturnDefaultConnInitParams();		windowName[0] = 0;		// Set up window name if user (and password) given	if (user != nil) {		GetIndString(windowName, MISC_STRINGS, MISC_USERPRMPT);		len = strlen(user);		BlockMoveData(user,& windowName[Length(windowName)+1], len);		windowName[0] += len;		if (password != nil) {		GetIndString(tempString, MISC_STRINGS, MISC_PSWDPRMPT);			BlockMoveData(&tempString[1], &windowName[Length(windowName)+1], tempString[0]);			windowName[0] += tempString[0];			len = strlen(password);			BlockMoveData(password, &windowName[Length(windowName)+1], len);			windowName[0] += len;			}		if (windowName[0] != 0) {			BlockMoveData(windowName, (**Params).WindowName, Length(windowName)+1);			}		}				CtoPstr(host);	BlockMoveData(host, (**(**Params).session).hostname, host[0]+1);		if (portstring != nil) {		CtoPstr(portstring);		StringToNum((StringPtr)portstring, &port);		(**(**Params).session).port = port;		}			(void)CreateConnectionFromParams(Params);}	Boolean CreateConnectionFromParams( ConnInitParams **Params){	short			scratchshort, fontnumber, otherfnum;	static short	numWind = 1, stagNum = 1;	SessionPrefs	*SessPtr;	TerminalPrefs	*TermPtr;	short			cur;	Str32			numPstring;	Str255			scratchPstring;	Boolean			scratchBoolean;	WindRec			*theScreen;	SetCursor(theCursors[watchcurs]);					/* We may be here a bit */	// Check if we have the max number of sessions open	if (TelInfo->numwindows == MaxSess) return(FALSE);		cur = TelInfo->numwindows;			/* Adjust # of windows and get this window's number */	TelInfo->numwindows++;	theScreen = &screens[cur];		theScreen->active = CNXN_NOTINUSE;	// Make sure it is marked as dead (in case we										// abort screen creation after initiating DNR.										// That way CompleteConnectionOpening will know										// we didn't make it.	HLockHi((Handle)Params);	HLockHi((Handle)(**Params).terminal);	HLockHi((Handle)(**Params).session);	SessPtr = *((**Params).session);	TermPtr = *((**Params).terminal);		if (Length((**Params).WindowName) == 0) {		BlockMove((**(**Params).session).hostname, (**Params).WindowName, 					Length((**(**Params).session).hostname)+1);		if (SessPtr->port != 23) {			NumToString((unsigned short)SessPtr->port, numPstring);			pstrcat((**Params).WindowName, "\p:");			pstrcat((**Params).WindowName, numPstring);		}		NumToString(numWind++, numPstring);		pstrcat((**Params).WindowName, "\p (");		pstrcat((**Params).WindowName, numPstring);	// tack the number onto the end.		pstrcat((**Params).WindowName, "\p)");		}	if (SessPtr->hostname[0] == 0) {		OperationFailedAlert(5, 0, 0);		DisposeHandle((Handle)(**Params).terminal);		DisposeHandle((Handle)(**Params).session);		DisposeHandle((Handle)Params);		TelInfo->numwindows--;		updateCursor(1);		return(FALSE);	}	Mnetinit();	// RAB BetterTelnet 1.0fc4	// Get the IP for the host while we set up the connection	if (DoTheDNR(SessPtr->hostname, cur) != noErr) {		OutOfMemory(1010);		DisposeHandle((Handle)(**Params).terminal);		DisposeHandle((Handle)(**Params).session);		DisposeHandle((Handle)Params);		TelInfo->numwindows--;		updateCursor(1);		return(FALSE);		}			DoTheMenuChecks();	  	theScreen->authenticate = SessPtr->authenticate && authOK;  	theScreen->encrypt = SessPtr->encrypt && encryptOK;     theScreen->aedata = NULL; 	 	for (scratchshort = 0; scratchshort < sizeof(theScreen->myopts); scratchshort++) {		theScreen->myopts[scratchshort] = 0;		theScreen->hisopts[scratchshort] = 0;			}		theScreen->cannon[0] = '\0';	theScreen->vtemulation = TermPtr->vtemulation;	theScreen->forcesave = SessPtr->forcesave;	theScreen->eightbit = TermPtr->eightbit;	// Is this necessary?	theScreen->portNum = SessPtr->port;	theScreen->portNegative = SessPtr->portNegative;	theScreen->allowBold = TermPtr->allowBold;	theScreen->colorBold = TermPtr->colorBold;	theScreen->realbold = TermPtr->realbold;	theScreen->inversebold = TermPtr->boldFontStyle;	theScreen->ignoreBeeps = SessPtr->ignoreBeeps;	theScreen->otpauto = SessPtr->otpauto;	theScreen->otpnoprompt = SessPtr->otpnoprompt;	theScreen->otphex = SessPtr->otphex;	theScreen->otpmulti = SessPtr->otpmulti;	theScreen->otpsavepass = SessPtr->otpsavepass;	theScreen->oldScrollback = TermPtr->oldScrollback;	pstrcpy((unsigned char *)theScreen->otppassword, (unsigned char *)SessPtr->otppassword);	theScreen->otpautostate = 0;	theScreen->otpautobuffer[7] = 0;	theScreen->emacsmeta = TermPtr->emacsmetakey;	theScreen->Xterm = TermPtr->Xtermsequences;	theScreen->remapCtrlD = TermPtr->remapCtrlD;	theScreen->keypadmap = TermPtr->remapKeypad;	theScreen->port = -1;				// netxopen will take care of this	theScreen->lineAllow = SessPtr->linemode;	if (SessPtr->linemode) //we allow linemode		initLinemode(&screens[cur]);					GetFNum(TermPtr->DisplayFont, &fontnumber);	GetFNum(TermPtr->BoldFont, &otherfnum);		theScreen->vs = RSnewwindow(&((**Params).WindowLocation),TermPtr->numbkscroll, TermPtr->vtwidth,									TermPtr->vtheight, (**Params).WindowName, TermPtr->vtwrap,									fontnumber, TermPtr->fontsize, 0,									1, SessPtr->forcesave,cur, TermPtr->allowBold, TermPtr->colorBold,									SessPtr->ignoreBeeps, otherfnum, TermPtr->boldFontSize, TermPtr->boldFontStyle,									TermPtr->realbold, TermPtr->oldScrollback);	if (theScreen->vs <0 ) { 	/* we have a problem opening up the virtual screen */		OutOfMemory(1011);		DisposeHandle((Handle)(**Params).terminal);		DisposeHandle((Handle)(**Params).session);		DisposeHandle((Handle)Params);		TelInfo->numwindows--;		DoTheMenuChecks();		updateCursor(1);		return(FALSE);		}	theScreen->wind = RSgetwindow( theScreen->vs);	((WindowPeek)theScreen->wind)->windowKind = WIN_CNXN;				/*	 * Attach our extra part to display encryption status	 */	PatchWindowWDEF(theScreen->wind, &screens[cur]);	theScreen->arrowmap = TermPtr->emacsarrows;  		/* MAT -- save our arrow setting */	theScreen->maxscroll= TermPtr->numbkscroll;	theScreen->bsdel = SessPtr->bksp;	theScreen->crmap = SessPtr->crmap;	if (theScreen->portNum != 23) // RAB BetterTelnet 1.0b1, 1.0fc4		theScreen->crmap = SessPtr->alwaysBSD; // RAB BetterTelnet 1.0b1, 1.0fc4	theScreen->tekclear = SessPtr->tekclear;	theScreen->ESscroll= TermPtr->clearsave;	theScreen->ANSIgraphics  = TermPtr->ANSIgraphics; //ANSI graphics, 2.7	theScreen->tektype = SessPtr->tektype;	theScreen->wrap = TermPtr->vtwrap;	theScreen->pgupdwn = TermPtr->MATmappings;		/* JMB: map pgup/pgdwn/home/end? */	theScreen->qprint = 0;	theScreen->ignoreff = SessPtr->ignoreff;	theScreen->TELstop = SessPtr->skey;	theScreen->TELgo = SessPtr->qkey;	theScreen->TELip = SessPtr->ckey;	BlockMove((Ptr)SessPtr->hostname, theScreen->machine, Length(SessPtr->hostname)+1);	BlockMove(TermPtr->AnswerBackMessage, theScreen->answerback, 32);	theScreen->termstate = VTEKTYPE;	theScreen->naws = 0;								/* NCSA - set default NAWS to zero */	theScreen->telstate=0;	theScreen->timing=0;	theScreen->curgraph=-1;				/* No graphics screen */	theScreen->clientflags = 0;			/* BYU */	theScreen->kblen = 0;				/* nothing in the buffer */	theScreen->enabled = 1;			/* Gotta be enabled to start with */	theScreen->Ittype = 0;	theScreen->Isga = 0;				/* I suppress go ahead = no */	theScreen->Usga = 0;				/* U suppress go ahead = no */	theScreen->remote_flow = 0;		/* they handle toggling remote_flow */	theScreen->allow_flow = 1;		/* initially, we allow flow control */	theScreen->restart_any_flow = 0;	/* initially, only an XON turns flow control back on  */	theScreen->termstate=VTEKTYPE;	/* BYU */	theScreen->echo = 1;	theScreen->halfdup = SessPtr->halfdup;	/* BYU */		theScreen->national = 0;			// Default to no translation.	// Now see if the desired translation is available, if not use default translation.	for(scratchshort = 1; scratchshort <= nNational+1; scratchshort++) {		GetItem(myMenus[National], scratchshort, scratchPstring);		if (EqualString(SessPtr->TranslationTable, scratchPstring, TRUE, FALSE))			theScreen->national = scratchshort-1;		}						// Set up paste related variables	theScreen->incount = 0;	theScreen->outcount = 0;	theScreen->outptr = NULL;	theScreen->outhand = NULL;	theScreen->outlen = 0;	theScreen->pastemethod = SessPtr->pastemethod;	theScreen->pastesize = SessPtr->pasteblocksize;		scratchBoolean = RSsetcolor( theScreen->vs, 0, TermPtr->nfcolor);	scratchBoolean = RSsetcolor( theScreen->vs, 1, TermPtr->nbcolor);	scratchBoolean = RSsetcolor( theScreen->vs, 2, TermPtr->bfcolor);	scratchBoolean = RSsetcolor( theScreen->vs, 3, TermPtr->bbcolor);	addinmenu(cur, (**Params).WindowName, diamondMark);	theScreen->active = CNXN_DNRWAIT;			// Signal we are waiting for DNR.	theScreen->myInitParams = (Handle)Params;	HUnlock((Handle)(**Params).terminal);	HUnlock((Handle)(**Params).session);	// Params handle must stay locked because interrupt level DNR completion routine needs to deref it	VSscrolcontrol( theScreen->vs, -1, theScreen->ESscroll);	updateCursor(1);							/* Done stalling the user */	return(TRUE);}void	CompleteConnectionOpening(short dat, ip_addr the_IP, OSErr DNRerror, char *cname){	ConnInitParams	**Params;		if (screens[dat].active != CNXN_DNRWAIT) return;			// Something is wrong.		Params = (ConnInitParams **)screens[dat].myInitParams;		if (DNRerror == noErr) {		HLockHi((Handle)(**Params).session);		if ((**(**Params).session).NetBlockSize < 512)				(**(**Params).session).NetBlockSize = 512; //less than this can really get messy					if (setReadBlockSize((**(**Params).session).NetBlockSize,dat) != 0) //couldnt get read buffer			return;		screens[dat].port  = netxopen(the_IP,(**(**Params).session).port,/* BYU 2.4.15 - open to host name */					gApplicationPrefs->OpenTimeout);/* CCP 2.7 allow user set-able timeouts on open */				// We need the cannonical hostname for Kerberos. Make best guess if		// DNR did not return a cname.		if (cname)			strncpy(screens[dat].cannon, cname, sizeof(screens[dat].cannon));		else			strncpy(screens[dat].cannon, (char *)(**(**Params).session).hostname, sizeof(screens[dat].cannon));		screens[dat].cannon[sizeof(screens[dat].cannon)-1] = '\0';				DisposeHandle((Handle)(**Params).session);		DisposeHandle((Handle)(**Params).terminal);		DisposeHandle((Handle)Params);				if (screens[dat].port <0) {					/* Handle netxopen fail */			destroyport(dat);			}		screens[dat].active = CNXN_OPENING;		SetMenuMarkToOpeningForAGivenScreen(dat);	/* Change status mark */		}	else		{	// We should report the real DNR error here!		Str255		errorString, numberString, numberString2, scratchPstring;		DialogPtr	theDialog;		short		message, ditem = 3;				HLockHi((Handle)(**Params).session);		BlockMove((**(**Params).session).hostname, scratchPstring, Length((**(**Params).session).hostname)+1);		if (DNRerror >= -23048 && DNRerror <= -23041) message = DNRerror + 23050;		else message = 1;				GetIndString(errorString,DNR_MESSAGES_ID, message);		NumToString((long)0, numberString);		NumToString((long)DNRerror, numberString2);		ParamText(scratchPstring, errorString, numberString, numberString2);				theDialog = GetNewMyDialog(DNRErrorDLOG, NULL, kInFront, (void *)ThirdCenterDialog);		ShowWindow(theDialog);			while (ditem > 1)	ModalDialog(DLOGwOKUPP, &ditem);		DisposeDialog(theDialog);		DisposeHandle((Handle)(**Params).session);		DisposeHandle((Handle)(**Params).terminal);		DisposeHandle((Handle)Params);		destroyport(dat);		}}void	ConnectionOpenEvent(short port){	short	i;		i=WindByPort(port);	if (i<0) { 		return;		}			 	screens[ i].active= CNXN_ACTIVE;	RSshow( screens[i].vs);			/* BYU */	SelectWindow(screens[i].wind);	/* BYU */	telnet_send_initial_options(&screens[i]);	changeport(scrn,i);		/* BYU */	SetMenuMarkToLiveForAGivenScreen(scrn);			/* BYU */	DoTheMenuChecks();		/* BYU */}void	ConnectionDataEvent(short port){	short	i, cnt;		i=WindByPort(port);									/* BYU */	if (i<0) {return; }					/* BYU */	if (TelInfo->ScrlLock || !screens[i].enabled)	/* BYU LSC */		netputuev( CONCLASS, CONDATA, port,0);	else {		cnt = netread(port,gReadspace,gBlocksize);	/* BYU LSC */		parse( &screens[i], gReadspace, cnt);	/* BYU LSC */		screens[i].incount += cnt;				/* BYU LSC */		}}void	ConnectionFailedEvent(short port){	short	i;	Str255	scratchPstring;		netclose( port);	i= WindByPort(port);	if (i<0) { return; }	BlockMove((Ptr)screens[i].machine, (Ptr)scratchPstring, Length(screens[i].machine)+1);	PtoCstr(scratchPstring);	DoError(807 | NET_ERRORCLASS, LEVEL2, (char *)scratchPstring);		if (screens[i].active != CNXN_ACTIVE) destroyport(i);	// JMB - 2.6	else removeport(&screens[i]);		// JMB - 2.6}void	ConnectionClosedEvent(short port){	short i;	i= WindByPort(port);	if (i<0) { 		netclose( port);			/* We close again.... */		return;		}	FlushNetwork(i);				/* BYU */	netclose( screens[i].port);		/* BYU */	removeport(&screens[i]);					/* BYU */}static	short	WindByPort(short port){	short i=0;	while (i<TelInfo->numwindows &&			(screens[i].port != port || 				((screens[i].active != CNXN_ACTIVE) && (screens[i].active != CNXN_OPENING)))			) i++;	if (i>=TelInfo->numwindows) {					/* BYU */		putln("Can't find a window for the port # in WindByPort");	/* BYU */		if (i==0) i=999;		/* BYU */		return(-i);				/* BYU */		}						/* BYU */	return(i);}void destroyport(short wind){	Handle	h;	short	i,			callNoWindow=0;	Size 	junk;	WindRecPtr	tw;		tw = &screens[wind];		SetCursor(theCursors[watchcurs]);		/* We may be here a while */	if (tw->active == CNXN_ISCORPSE) {		if (tw->curgraph>-1)			detachGraphics( tw->curgraph);	/* Detach the Tek screen */		if (tw->outlen>0) {			tw->outlen=0;						/* Kill the remaining send*/			HUnlock( tw->outhand);			/*  buffer */			HPurge ( tw->outhand);			}		}	if (FrontWindow() == tw->wind)		callNoWindow=1;	if (tw->aedata != NULL)		DisposePtr((Ptr)tw->aedata); 	/*	 * Get handle to the WDEF patch block, kill the window, and then	 * release the handle.	 */	h = GetPatchStuffHandle(tw->wind, tw);	RSkillwindow( tw->vs);	SetDefaultKCHR();	if (h)		DisposeHandle(h);	tw->active = CNXN_NOTINUSE;	for (i=wind;i<TelInfo->numwindows-1;i++) {		screens[i]=screens[i+1];		/* Bump all of the pointers */		RePatchWindowWDEF(screens[i].wind, &screens[i]);	/* hack hack hack */		}	if (scrn>wind) scrn--;				/* Adjust for deleting a lower #ered screen */	TelInfo->numwindows--;						/* There are now fewer windows */	extractmenu( wind);					/* remove from the menu bar */	DoTheMenuChecks();	MaxMem(&junk);/* BYU 2.4.11 - the call to "NoWindow()" changes "myfrontwindow",                 which is used by "updateCursor()", so we reversed                 the order of the following two lines. */	if (callNoWindow) NoWindow();		/* BYU 2.4.11 - Update cursor stuff if front window */	updateCursor(1);					/* BYU 2.4.11 - Done stalling the user */	} /* destroyport */void removeport(WindRecPtr tw){	Str255		scratchPstring;		SetCursor(theCursors[watchcurs]);				/* We may be here a while */	if (tw->curgraph>-1)		detachGraphics( tw->curgraph);		/* Detach the Tek screen */			if (tw->outlen>0) {				tw->outlen=0;				/* Kill the remaining send*/				HUnlock( tw->outhand);		/*  buffer */				HPurge ( tw->outhand);				}	if (VSiscapturing(tw->vs))				/* NCSA: close up the capture */		CloseCaptureFile(tw->vs);			/* NCSA */	if (VSisprinting(tw->vs))		ClosePrintingFile(tw->vs);	if ((gApplicationPrefs->destroyKTickets)&&(numberLiveConnections() == 1))//if this is last window		DestroyTickets();			if (!gApplicationPrefs->WindowsDontGoAway)		destroyport(findbyVS(tw->vs));	else {		Str255	temp;				GetWTitle(tw->wind, scratchPstring);		sprintf((char *)temp, "(%#s)", scratchPstring);		CtoPstr((char *)temp);		SetWTitle(tw->wind, temp);		tw->port = 32700;		tw->active = CNXN_ISCORPSE;		}	updateCursor(1);							/* Done stalling the user */} /* removeport *///	We recognize the following input string: "xxxx yyyy"//	If "xxxx" matches a session name, that session record is used.  Otherwise, the default//	session record is used with "xxxx" as the DNS hostname.   "yyyy", if extant, is//	converted to a number.  If it is a valid number, it is used as the port to connect to.//	WARNING: Do not pass this routing a blank string.  (We check this in PresentOpenConnectionDialog.)////	CCP 2.7:  If second argument is true, get terminal out of session pref; otherwise set it to NULLConnInitParams	**NameToConnInitParams(StringPtr InputString, Boolean useDefaultTerminal, StringPtr otherPstring){	ConnInitParams	**theHdl;	SessionPrefs	**sessHdl;	TerminalPrefs	**termHdl;	short			portRequested, portHack, portNegative;	Boolean			foundPort, wasAlias = FALSE;	long 			junk;	theHdl = (ConnInitParams **)myNewHandleCritical(sizeof(ConnInitParams));	if (theHdl == NULL)		return NULL;	if (useDefaultTerminal) {		foundPort = 0;		portNegative = 0;	} else		foundPort = ProcessHostnameString(InputString, &portRequested, &portNegative);	UseResFile(TelInfo->SettingsFile);	if (useDefaultTerminal) {		sessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, InputString);		if (sessHdl == NULL)		{				// Connect to host xxxx w/default session.			portHack = 1;			sessHdl = GetDefaultSession();			DetachResource((Handle) sessHdl);			HLock((Handle)sessHdl);			BlockMove(InputString, (**sessHdl).hostname, 64);		}		else 		{				portHack = 0;			DetachResource((Handle) sessHdl);			HLock((Handle)sessHdl);			wasAlias = TRUE;		}	} else {		sessHdl = NULL;		if (gApplicationPrefs->parseAliases)			sessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, InputString);		if (sessHdl == NULL) {			portHack = 1;			sessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, otherPstring);			DetachResource((Handle) sessHdl);			HLock((Handle)sessHdl);			BlockMove(InputString, (**sessHdl).hostname, 64);		} else {			portHack = 0;			DetachResource((Handle) sessHdl);			HLock((Handle)sessHdl);		}	}	(**theHdl).session = sessHdl;		UseResFile(TelInfo->SettingsFile);//	if ((useDefaultTerminal)||(wasAlias))//	if (1)//	{		termHdl = (TerminalPrefs **)Get1NamedResource			(TERMINALPREFS_RESTYPE, (**sessHdl).TerminalEmulation);	if (termHdl == NULL) termHdl = GetDefaultTerminal();	DetachResource((Handle) termHdl);	(**theHdl).terminal = termHdl;//	}//	else//		(**theHdl).terminal = NULL;	UnloadSeg(&PREFSUnload);	MaxMem(&junk);	  //swap out space so we can make the new window CCP	HUnlock((Handle)sessHdl);		((**theHdl).WindowName)[0] = 0;	(**sessHdl).ip_address = 0;		if (foundPort) { (**sessHdl).port = portRequested; (**sessHdl).portNegative = portNegative; }	else if (portHack) (**sessHdl).port = 23;		return(theHdl);}ConnInitParams	**ReturnDefaultConnInitParams(void){	ConnInitParams	**theHdl;	theHdl = (ConnInitParams **)myNewHandle(sizeof(ConnInitParams));	(**theHdl).session = GetDefaultSession();	(**(**theHdl).session).ip_address = 0;	(**theHdl).terminal = GetDefaultTerminal();		return(theHdl);}short numberLiveConnections(void){	short i;	short liveConnections = 0;	for(i = 0; i < MaxSess; i++)		if ((screens[i].active == CNXN_ACTIVE)||(screens[i].active == CNXN_OPENING))			liveConnections++;	return liveConnections;}