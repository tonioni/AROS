#ifndef AFS_HANDLER_H
#define AFS_HANDLER_H

/*
    Copyright � 1995-2003, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <exec/devices.h>
#include <dos/filesystem.h>
#include <devices/timer.h>

struct afsbase
{
	struct Device device;
	struct IntuitionBase *intuitionbase;
	struct DosLibrary *dosbase;
	struct ExecBase *sysbase;
	BPTR seglist;
	struct MsgPort port;			/* MsgPort of the handler */
	struct MsgPort rport;		/* replyport of the handler */
	struct IOFileSys *iofs;		/* to be aborted iofs or NULL */
	struct List device_list;	/* list of mounted devices (struct Volume) */
};

#define expunge() AROS_LC0(BPTR, expunge, struct afsbase *, afsbase, 3, afs)

#endif
