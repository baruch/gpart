/*
 * errmsgs.h -- gpart error/warning messages header file
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   04.01.1999 <mb@ichabod.han.de>
 * Modified:
 *
 */

#ifndef _ERRMSGS_H
#define _ERRMSGS_H


/* dialog messages */
#define DM_YESNO		"(y,n)"
#define DM_YES			"yY"
#define DM_NUMORQUIT		" (%d..%d, q to quit) : "
#define DM_QUIT			"qQ"
#define DM_STARTSCAN		"\nBegin scan...\n"
#define DM_ENDSCAN		"End scan.\n"
#define DM_EDITPTBL		"Edit this table"
#define DM_ACCEPTGUESS		"\nAccept this guess"
#define DM_ACTWHICHPART		"Activate which partition"
#define DM_EDITWHICHPART	"Edit which partition"
#define DM_WRITEIT		"Write this partition table"
#define DM_ASKTOREBOOT		"partition table written, you should reboot now"
#define DM_NOTWRITTEN		"Partition table not written\n"
#define DM_STARTCHECK		"\nChecking partitions...\n"
#define DM_NOOFINCONS		"Number of inconsistencies found: %d.\n"
#define DM_GUESSEDPTBL		"\nGuessed primary partition table:\n"
#define DM_NOCHECKWARNING	"\nWarning: entered values will not be checked; enter at your own risk!\n"
#define DM_EDITWHICHITEM	"\nEdit which value"

/* partition list messages */
#define PM_DEVDESC1		"\ndev(%s) mss(%d)"
#define PM_DEVDESC2		" chs(%d/%d/%d)%s#s(%qd) size(%qdmb)"
#define PM_MBRPRINT		"\ndev(%s) master boot record (w/o partition table):\n"
#define PM_PRIMPART		"Primary partition(%d)\n"
#define PM_EXTPART		"   Logical partition\n"
#define PM_POSSIBLEPART		"Possible partition(%s), size(%qdmb), offset(%qdmb)\n"
#define PM_POSSIBLEEXTPART	"Possible extended partition at offset(%qdmb)\n"
#define PM_PT_TYPE		"   type: %03d(0x%02X)(%s)"
#define PM_PT_SIZE		"   size: %qdmb #s(%qd)"
#define PM_PT_CHS		"   chs:  (%d/%d/%d)-(%d/%d/%d)d"
#define PM_PT_HEX		"   hex: "
#define PM_G_PRIMARY		"primary "
#define PM_G_LOGICAL		"logical "
#define PM_G_INVALID		"invalid "
#define PM_G_ORPHANED		"orphaned "
#define PM_EDITITEM1		"1 - Absolute start sector (%12lu)\n"
#define PM_EDITITEM2		"2 - Absolute sector count (%12lu)\n"
#define PM_EDITITEM3		"3 - Partition type        (%12d)(%s)\n"

/* error/warning messages */
#define EM_FATALERROR		"\n*** Fatal error: %s.\n"
#define EM_SIMPLEERROR		"\n** Error: %s.\n"
#define EM_WARNING		"\n* Warning: %s.\n"
#define EM_PINVALID		"\n* Partition invalid(%s):\n"
#define EM_MALLOCFAILED		"malloc(%d) failed"
#define EM_IOCTLFAILED		"ioctl(%s) failed: %s"
#define EM_OPENFAIL		"open(%s): %s"
#define EM_STRANGEPTBLMAGIC	"strange partition table magic 0x%04X"
#define EM_WRONGSECTSIZE	"sector size must be > 0 and <= %d"
#define EM_FAILSSIZEATTEMPT	"failed trying to use sector size %d"	
#define EM_SEEKFAILURE		"dev(%s): seek failure"
#define EM_STATFAILURE		"stat(%s): %s"
#define EM_READERROR		"dev(%s): read error near sector(%qd): %s"
#define EM_CANTGETSSIZE		"cannot get sector size on dev(%s)"
#define EM_CANTGETGEOM		"cannot get disk geometry"
#define EM_MINITFAILURE		"module(%s) failed to init"
#define EM_INVVALUE		"invalid number value"
#define EM_PSTART2BIG		"partition(%s) starts beyond disk end"
#define EM_PSIZE2BIG		"partition(%s) is bigger than the disk"
#define EM_PEND2BIG		"partition(%s) ends beyond disk end"
#define EM_STRANGEPTYPE		"partition(%s) contains strange flag"
#define EM_PTBLREAD		"failed to read partition table"
#define EM_PTBLWRITE		"could not write partition table"
#define EM_MBRWRITE		"could not write master boot record"
#define EM_TOOMANYEXTP		"found more than one extended partition, skipping"
#define EM_TOOMANYLOGP		"more than %d logical partitions encountered"
#define EM_EPILLEGALOFS		"extended ptbl illegal sector offset"
#define EM_INVXPTBL		"invalid extended ptbl found at sector(%qd)"
#define EM_DISCARDOVLP		"Discarded %d overlapping partition guesses"
#define EM_TOOMANYXPTS		"more than one extended partition: %d"
#define EM_TOOMANYPPTS		"more than %d primary partitions: %d"
#define EM_OPENLOG		"cannot open logfile %s"
#define EM_NOSUCHMOD		"no such module: %s"
#define EM_SHORTBREAD		"short read near sector(%qd), %d bytes instead of %d. Skipping.."
#define EM_BADREADIO		"read error (EIO) near sector(%qd), skipping.."
#define EM_PINCONS		"partition still overlaps with previous one or seems invalid:"
#define EM_P_EATEND		"extended ptbl without any following partitions"
#define EM_P_EWLP		"extended ptbl without logical partition"
#define EM_P_MTOE		"encountered a second extended ptbl"
#define EM_P_LISAE		"logical partition is an extended partition"
#define EM_P_UTS		"wrong size, no valid type or (ptbl,link) type mismatch"
#define EM_P_2MANYPP		"too many primary partitions"
#define EM_P_NOTSANE		"invalid partition entry (see comments above)"
#define EM_P_ENDNOTF		"primary partition within extended ptbl link"


#endif /* _ERRMSGS_H */
