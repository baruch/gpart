/*
 * l64seek.h -- gpart signed 64bit seek header file
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

#ifndef _L64SEEK_H
#define _L64SEEK_H

#include <unistd.h>

/*
 * define a type 'off64_t' which is at least 64bit, and a
 * lseek function capable of seeking with at least 64bit
 * offsets.
 */


#if defined(__linux__)
#	include <linux/unistd.h>
#	include <sys/types.h>
	typedef int64_t off64_t;
#endif


#if defined(__FreeBSD__)
	typedef off_t off64_t;
#endif


typedef off64_t s64_t;

off64_t l64seek(int fd, off64_t offset, int whence);
#define l64tell(fd) l64seek(fd,0,SEEK_CUR)
int l64opush(int);
s64_t l64opop(int);


#endif
