/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  s_conf.h: A header for the debug functions.
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
 *   $Id: s_debug.h,v 1.2 2002/01/06 07:18:13 a1kmm Exp $
 */

#ifndef INCLUDED_s_debug_h
#define INCLUDED_s_debug_h

#ifndef INCLUDED_sys_types_h
#include <sys/types.h>       /* size_t */
#define INCLUDED_sys_types_h
#endif

struct Client;

/*
 * defined debugging levels
 */
#define DEBUG_FATAL  0
#define DEBUG_ERROR  1  /* report_error() and other errors that are found */
#define DEBUG_NOTICE 3
#define DEBUG_DNS    4  /* used by all DNS related routines - a *lot* */
#define DEBUG_INFO   5  /* general usful info */
#define DEBUG_NUM    6  /* numerics */
#define DEBUG_SEND   7  /* everything that is sent out */
#define DEBUG_DEBUG  8  /* anything to do with debugging, ie unimportant :) */
#define DEBUG_MALLOC 9  /* malloc/free calls */
#define DEBUG_LIST  10  /* debug list use */


extern void send_usage(struct Client*);
extern void count_memory (struct Client *);

extern void debug(int, char *, ...);

#endif /* INCLUDED_s_debug_h */

