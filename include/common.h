/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  common.h: An ircd header common to most code.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * $Id: common.h,v 1.3 2002/01/06 09:20:33 a1kmm Exp $
 */

#ifndef INCLUDED_common_h
#define INCLUDED_common_h


#ifndef NULL
#define NULL 0
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define FALSE  0
#define TRUE   1
#define HIDEME 2

/* Blah. I use these a lot. -Dianora */
#ifdef YES
#undef YES
#endif

#define YES 1

#ifdef NO
#undef NO
#endif

#define NO  0

/* Just blindly define our own MIN/MAX macro */

#define IRCD_MAX(a, b)  ((a) > (b) ? (a) : (b))
#define IRCD_MIN(a, b)  ((a) < (b) ? (a) : (b))

/* Right out of the RFC */
#define IRCD_BUFSIZE 512

#ifdef __GNUC__
#define INLINE inline
#define CONSTFUNC __attribute__((const))
#define REGPARAM(x) __attribute__((regparm(x)))
#else
#define INLINE
#define CONSTFUNC
#define REGPARAM(x)
#endif

#endif /* INCLUDED_common_h */
